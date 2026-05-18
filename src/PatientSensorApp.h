#ifndef PATIENTSENSORAPP_H_
#define PATIENTSENSORAPP_H_
#include "LoRaApp/SimpleLoRaApp.h"
#include "LoRa/LoRaTagInfo_m.h"
#include "LoRaApp/LoRaAppPacket_m.h"
namespace flora {
class PatientSensorApp final : public flora::SimpleLoRaApp {
private:
    long urgentPacketsSent = 0;
    long normalPacketsSent = 0;
    omnetpp::simsignal_t urgentSentSignal;
    omnetpp::simsignal_t normalSentSignal;
    omnetpp::simsignal_t emergencyVariateSignal;
public:
    PatientSensorApp();
    virtual ~PatientSensorApp();
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
protected:
    void handleMessageFromLowerLayer(omnetpp::cMessage *msg);
    void sendJoinRequest();
};
}
#endif
