#ifndef HOSPITALSERVERAPP_H_
#define HOSPITALSERVERAPP_H_
#include "LoRa/NetworkServerApp.h"
#include <queue>
namespace flora {
class HospitalServerApp final : public flora::NetworkServerApp {
private:
    double serviceRate = 0;
    int    queueSize   = 0;
    std::queue<omnetpp::simtime_t> urgentQueue;
    std::queue<omnetpp::simtime_t> normalQueue;
    bool serverBusy = false;
    bool currentlyServingUrgent = false;
    long urgentReceivedCounter = 0;
    long normalReceivedCounter = 0;
    long urgentDroppedCounter  = 0;
    long normalDroppedCounter  = 0;
    long urgentServedCounter   = 0;
    long normalServedCounter   = 0; 
    omnetpp::simsignal_t queueLengthSignal; 
    omnetpp::simsignal_t urgentDroppedSignal; 
    omnetpp::simsignal_t normalDroppedSignal; 
    omnetpp::simsignal_t urgentWaitTimeSignal; 
    omnetpp::simsignal_t normalWaitTimeSignal; 
    omnetpp::simsignal_t urgentEndToEndDelaySignal; 
    omnetpp::simsignal_t normalEndToEndDelaySignal;
    void enqueuePacket(omnetpp::simtime_t creationTime, bool urgent);
    void startNextService();
    int  totalQueueLength() const;
public:
    HospitalServerApp();
    virtual ~HospitalServerApp();
    HospitalServerApp(const HospitalServerApp &other);
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
};
}
#endif
