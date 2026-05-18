#include "PatientSensorApp.h"
#include "NumGenerator.h"
#include "inet/common/packet/Packet.h"
namespace flora {
Define_Module(PatientSensorApp);
PatientSensorApp::PatientSensorApp() { }
PatientSensorApp::~PatientSensorApp() { }
void PatientSensorApp::initialize(int stage) {
    SimpleLoRaApp::initialize(stage);
    if (stage == inet::INITSTAGE_LOCAL) {
        urgentSentSignal       = registerSignal("urgentPacketSent");
        normalSentSignal       = registerSignal("normalPacketSent");
        emergencyVariateSignal = registerSignal("emergencyVariate");
    }
}
void PatientSensorApp::finish() {
    recordScalar("urgentPacketsSentTotal", urgentPacketsSent);
    recordScalar("normalPacketsSentTotal", normalPacketsSent);
    SimpleLoRaApp::finish();
}
void PatientSensorApp::handleMessage(omnetpp::cMessage *msg) {
    if (msg->isSelfMessage()) {
        if (msg == sendMeasurements) {
            this->sendJoinRequest();
            if (simTime() >= getSimulation()->getWarmupPeriod())
                sentPackets++;
            delete msg;
            sendMeasurements = nullptr;
            if (numberOfPacketsToSend == 0 || sentPackets < numberOfPacketsToSend) {
                double time = 0;
                int loRaSF = getSF();
                if (loRaSF == 7)  time = 7.808;
                else if (loRaSF == 8)  time = 13.9776;
                else if (loRaSF == 9)  time = 24.6784;
                else if (loRaSF == 10) time = 49.3568;
                else if (loRaSF == 11) time = 85.6064;
                else if (loRaSF == 12) time = 171.2128;
                do {
                    timeToNextPacket = par("timeToNextPacket");
                } while (timeToNextPacket <= time);
                sendMeasurements = new omnetpp::cMessage("sendMeasurements");
                scheduleAt(simTime() + timeToNextPacket, sendMeasurements);
            }
        }
    } else {
        handleMessageFromLowerLayer(msg);
        delete msg;
    }
}
void PatientSensorApp::handleMessageFromLowerLayer(omnetpp::cMessage *msg) {
    SimpleLoRaApp::handleMessageFromLowerLayer(msg);
}
void PatientSensorApp::sendJoinRequest() {
    auto pktRequest = new inet::Packet("PatientDataFrame");
    pktRequest->setKind(DATA);
    auto payload = inet::makeShared<LoRaAppPacket>();
    payload->setChunkLength(inet::B(par("dataSize").intValue()));
    payload->setMsgType(DATA);
    double lambda    = par("expo_rate_lambda").doubleValue();
    double threshold = par("emergencyThreshold").doubleValue();
    double variate   = NumGenerator::exponential(lambda);
    bool   isEmergency = (variate < threshold);
    payload->setSampleMeasurement(isEmergency ? 1 : 0);
    emit(emergencyVariateSignal, variate);
    if (simTime() >= getSimulation()->getWarmupPeriod()) {
        if (isEmergency) {
            urgentPacketsSent++;
            emit(urgentSentSignal, 1L);
        } else {
            normalPacketsSent++;
            emit(normalSentSignal, 1L);
        }
    }
    EV_INFO << "Variate: " << variate << " Threshold: " << threshold
            << " => " << (isEmergency ? "URGENT" : "normal") << std::endl;
    auto loraTag = pktRequest->addTagIfAbsent<LoRaTag>();
    loraTag->setBandwidth(getBW());
    loraTag->setCenterFrequency(getCF());
    loraTag->setSpreadFactor(getSF());
    loraTag->setCodeRendundance(getCR());
    loraTag->setPower(inet::mW(inet::math::dBmW2mW(getTP())));
    sfVector.record(getSF());
    tpVector.record(getTP());
    EV_INFO << "Sending patient packet, TP: " << getTP() << ", SF: " << getSF() << std::endl;
    pktRequest->insertAtBack(payload);
    send(pktRequest, "socketOut");
    if (evaluateADRinNode) {
        ADR_ACK_CNT++;
        if (ADR_ACK_CNT == ADR_ACK_LIMIT) sendNextPacketWithADRACKReq = true;
        if (ADR_ACK_CNT >= ADR_ACK_LIMIT + ADR_ACK_DELAY) {
            ADR_ACK_CNT = 0;
            increaseSFIfPossible();
        }
    }
    emit(LoRa_AppPacketSent, getSF());
}
}
