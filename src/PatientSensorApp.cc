#include "PatientSensorApp.h"
#include "NumGenerator.h"
#include "inet/common/packet/Packet.h"

namespace flora {//önemli

Define_Module(PatientSensorApp);

PatientSensorApp::PatientSensorApp() { }
PatientSensorApp::~PatientSensorApp() { }

void PatientSensorApp::initialize(int stage) {
    SimpleLoRaApp::initialize(stage);

    if (stage == inet::INITSTAGE_LOCAL) {
        // sinyalleri NED dosyasıyla eşleşecek şekilde kaydetmek lazım
        //yoksa simülasyonda donup donup duruyor durduk yere
        urgentSentSignal       = registerSignal("urgentPacketSent");
        normalSentSignal       = registerSignal("normalPacketSent");
        emergencyVariateSignal = registerSignal("emergencyVariate");
    }
}

void PatientSensorApp::finish() {
    // kaç tane paket olduğunu sayı cinsinden kaydetmek lazım
    //o yğzden recordscalar kullanıp ne kadar paket gitti ne kadar normal ne kadar urgent burada kaydedicez en sonda
    recordScalar("urgentPacketsSentTotal", urgentPacketsSent);
    recordScalar("normalPacketsSentTotal", normalPacketsSent);
    SimpleLoRaApp::finish();
}
//bu bokun da override edilememesinin sebebi bu flora namespace içinde olmamasından kaynaklıymış
void PatientSensorApp::handleMessage(omnetpp::cMessage *msg) {
    if (msg->isSelfMessage()) {
        if (msg == sendMeasurements) {
            this->sendJoinRequest(); // handle messagede ne yaptıysak aynısını burda da yapıcaz

            if (simTime() >= getSimulation()->getWarmupPeriod())
                sentPackets++;

            delete msg;
            sendMeasurements = nullptr;
            //kalsın
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
    //paket ismini değiştirdim bizimle uyumlu olsun diye wireless signal gibi bir şeydi kötü duruyodu
    auto pktRequest = new inet::Packet("PatientDataFrame");

    //inet::DATA arıza yarattı flora namespacesine alınca o yüzden yerine flora'nın kendi data verisi kullanmak lazımdı yoksa paso çöküyor
    pktRequest->setKind(DATA);

    // bizim göndereceğimiz mesaj/payload dediğimiz şey normalde omnetpp::makeShared di ama sürekli simülasyonda warning oluyordu
    // yerine inet::makeShared kullandık
    auto payload = inet::makeShared<LoRaAppPacket>();
    //data kaç byte falan
    payload->setChunkLength(inet::B(par("dataSize").intValue()));
    payload->setMsgType(DATA);

    // numgenerator sonucuna göre paketin urgent mi değil mi kararı burada veriliyor
    double lambda    = par("expo_rate_lambda").doubleValue(); //bunu ned dosyasından çekiyoruz
    double threshold = par("emergencyThreshold").doubleValue();//bunu da
    double variate   = NumGenerator::exponential(lambda); //bu generatorden geliytor
    bool   isEmergency = (variate < threshold);//burada da labellanıyor

    payload->setSampleMeasurement(isEmergency ? 1 : 0);

    // QQ-plot için ham variate değerini lazım
    //bunu kullanmak lazım analysis dosyasında direkt çıktı alabiliyoruz simülasyon resulttan
    emit(emergencyVariateSignal, variate);

    // kpi sayaçları ve sinyalleri,  yine qqplot için emit kullanıyoruyz
    if (simTime() >= getSimulation()->getWarmupPeriod()) {
        if (isEmergency) {
            urgentPacketsSent++;
            emit(urgentSentSignal, 1L);
        } else {
            normalPacketsSent++;
            emit(normalSentSignal, 1L);
        }
    }
    //debug
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

} // namespace flora
