#include "HospitalServerApp.h"
#include "NumGenerator.h"
#include "LoRaApp/LoRaAppPacket_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/Chunk.h"
namespace flora {
Define_Module(HospitalServerApp);
HospitalServerApp::HospitalServerApp() { }
HospitalServerApp::~HospitalServerApp() { }
void HospitalServerApp::initialize(int stage) {
    NetworkServerApp::initialize(stage);
    if (stage == inet::INITSTAGE_LOCAL) {
        inet::Chunk::enableImplicitChunkSerialization = true;
        serviceRate = par("expo_rate_lambda").doubleValue();
        queueSize   = par("queueSize").intValue();
        queueLengthSignal = registerSignal("queueLength");
        urgentDroppedSignal = registerSignal("urgentDropped");
        normalDroppedSignal = registerSignal("normalDropped");
        urgentWaitTimeSignal = registerSignal("urgentWaitTime");
        normalWaitTimeSignal = registerSignal("normalWaitTime");
        urgentEndToEndDelaySignal = registerSignal("urgentEndToEndDelay");
        normalEndToEndDelaySignal = registerSignal("normalEndToEndDelay");
    }
}
void HospitalServerApp::finish() {
    recordScalar("urgentReceivedTotal", urgentReceivedCounter);
    recordScalar("normalReceivedTotal", normalReceivedCounter);
    recordScalar("urgentDroppedTotal",  urgentDroppedCounter);
    recordScalar("normalDroppedTotal",  normalDroppedCounter);
    recordScalar("urgentServedTotal",   urgentServedCounter);
    recordScalar("normalServedTotal",   normalServedCounter);
    if (urgentReceivedCounter > 0)
        recordScalar("urgentLossRate", (double)urgentDroppedCounter / urgentReceivedCounter);
    if (normalReceivedCounter > 0)
        recordScalar("normalLossRate", (double)normalDroppedCounter / normalReceivedCounter);
    NetworkServerApp::finish();
}
int HospitalServerApp::totalQueueLength() const {
    return (int)(urgentQueue.size() + normalQueue.size());
}
void HospitalServerApp::enqueuePacket(omnetpp::simtime_t creationTime, bool urgent) {
    auto &q             = urgent ? urgentQueue : normalQueue;
    auto &dropCounter   = urgent ? urgentDroppedCounter : normalDroppedCounter;
    auto  dropSignal    = urgent ? urgentDroppedSignal  : normalDroppedSignal;
    if ((int)q.size() >= queueSize) {
        dropCounter++;
        emit(dropSignal, 1L);
    } else {
        q.push(creationTime);
        emit(queueLengthSignal, (long)totalQueueLength());
        if (!serverBusy) {
            startNextService();
        }
    }
}
void HospitalServerApp::startNextService() {
    bool serveUrgent;
    if (!urgentQueue.empty())      serveUrgent = true; 
    else if (!normalQueue.empty()) serveUrgent = false; 
    else { serverBusy = false; return; }
    serverBusy = true; 
    currentlyServingUrgent = serveUrgent; 
    auto &q = serveUrgent ? urgentQueue : normalQueue;
    omnetpp::simtime_t creationTime = q.front();
    q.pop(); 
    emit(queueLengthSignal, (long)totalQueueLength());
    omnetpp::simtime_t waitTime = simTime() - creationTime;
    emit(serveUrgent ? urgentWaitTimeSignal : normalWaitTimeSignal, waitTime);
    emit(serveUrgent ? urgentEndToEndDelaySignal : normalEndToEndDelaySignal, waitTime);
    if (serveUrgent) urgentServedCounter++;
    else normalServedCounter++;
    omnetpp::simtime_t serviceDuration = NumGenerator::exponential(serviceRate);
    scheduleAt(simTime() + serviceDuration, new omnetpp::cMessage("serviceComplete"));
}
void HospitalServerApp::handleMessage(omnetpp::cMessage *msg) {
    if (msg->arrivedOn("socketIn")) {
        auto pkt = inet::check_and_cast<inet::Packet *>(msg);
        if (simTime() >= getSimulation()->getWarmupPeriod()) {
                    try {
                        auto macFrame = pkt->popAtFront<LoRaMacFrame>();
                        auto appPkt = pkt->peekAtFront<LoRaAppPacket>();
                        bool urgent = (appPkt->getSampleMeasurement() == 1);
                        if (urgent) urgentReceivedCounter++;
                        else normalReceivedCounter++;
                        omnetpp::simtime_t creationTime = pkt->getCreationTime();
                        enqueuePacket(creationTime, urgent);
                        pkt->insertAtFront(macFrame);
                    }
                    catch (std::exception &e) {
                        EV_WARN << "Could not parse LoRaAppPacket: " << e.what() << std::endl;
                    }
                }
        updateKnownNodes(pkt);
        processLoraMACPacket(pkt);
    }
    else if (msg->isSelfMessage() && strcmp(msg->getName(), "serviceComplete") == 0) {
        delete msg;
        serverBusy = false;
        startNextService();
    }
    else if (msg->isSelfMessage()) {
        processScheduledPacket(msg);
    }
}
}
