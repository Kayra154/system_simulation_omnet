#ifndef PATIENTSENSORAPP_H_
#define PATIENTSENSORAPP_H_

#include "LoRaApp/SimpleLoRaApp.h"
#include "LoRa/LoRaTagInfo_m.h"
#include "LoRaApp/LoRaAppPacket_m.h"

namespace flora {//önemli

class PatientSensorApp final : public flora::SimpleLoRaApp {
private:
    // kpi urgent normal counter
    long urgentPacketsSent = 0;
    long normalPacketsSent = 0;

    // omnetpp simülasyonunda resultta gözükmesi için omnetpp::simsignalt kullandık
    omnetpp::simsignal_t urgentSentSignal;
    omnetpp::simsignal_t normalSentSignal;
    omnetpp::simsignal_t emergencyVariateSignal; // QQ-plot analizi için

public:
    PatientSensorApp();
    virtual ~PatientSensorApp();
    //kodumun copy constructoru
    //PatientSensorApp(const PatientSensorApp &other);

    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;

protected:
    void handleMessageFromLowerLayer(omnetpp::cMessage *msg);
    void sendJoinRequest();
};

} // namespace flora bitti
#endif
