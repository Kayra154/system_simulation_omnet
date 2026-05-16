//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef HOSPITALSERVERAPP_H_
#define HOSPITALSERVERAPP_H_

#include "LoRa/NetworkServerApp.h"
#include <queue>//queue veri yapısını kullanabilmek için laızm

namespace flora { // kodun flora bokunu tam çalıştırmsaı simülasyonda patlamaması çalışması için namespace flora attık


class HospitalServerApp final : public flora::NetworkServerApp {
private:
    // omnetini dosyasından parse edilecek parametreler
    double serviceRate = 0; // sunucunun paket işleme oranı
    int    queueSize   = 0; // her bir kuyruğun alabileceği paket kapasitesi (düzü 50 değişir)

    // queueler 2 tane var bir urgent 1 normal
    // paketlerin kendilerini değil sadece kuyruğa giriş zamanı  tutuyoruz ı da simtime diye geçiyo
    std::queue<omnetpp::simtime_t> urgentQueue; // urgent paketlerinin toplandığı queue
    std::queue<omnetpp::simtime_t> normalQueue; // normal paketlerinin toplandığı queue

    // state değişken
    bool serverBusy = false;             // sunucu şu an paket işliyor mu onu tuytuyor
    bool currentlyServingUrgent = false; // gelen ve serve edilen kapet urgentmi

    // kpiler
    // omnet line ploıtter arayüzü grafik çizmezse (ki arada çöküyor),
    //simülasyon çıktısını pythonda parse etmek için kalsın
    long urgentReceivedCounter = 0; // sunucuya ulaşmayı başaran toplam acil paket sayısı
    long normalReceivedCounter = 0; // sunucuya ulaşmayı başaran toplam normal paket sayısı
    long urgentDroppedCounter  = 0; // kuyruk tamamen dolu olduğu için çöpe giden drop yiyen acil paket sayısı
    long normalDroppedCounter  = 0; // kuyruk tamamen dolu olduğu için çöpe giden drop yiyen normal paket sayısı
    long urgentServedCounter   = 0; // başarıyla işlenen toplam acil paket sayısı
    long normalServedCounter   = 0; // başarıyla işlenien toplam normal paket sayısı

    // sim
    omnetpp::simsignal_t queueLengthSignal; // anlık toplam kuyruk uzunluğunu ne kadar
    omnetpp::simsignal_t urgentDroppedSignal; // acil paket kaybı yaşandığında bir flag gibi davranacak süre hesabında lazım
    omnetpp::simsignal_t normalDroppedSignal; // normal paket kaybı yaşandığında haberimiz olması için sinyal tipinde flag
    omnetpp::simsignal_t urgentWaitTimeSignal; // acil paketlerin kuyrukta bekleme süresini kaydedecek, hesaplamada lazım
    omnetpp::simsignal_t normalWaitTimeSignal; // aynı şekil normal paketlerin kuyrukta bekleme süresini kaydecek
    omnetpp::simsignal_t urgentEndToEndDelaySignal; // acil paketlerin ne kadar gecikecek
    omnetpp::simsignal_t normalEndToEndDelaySignal; // normal paketlerin toplam gecikme sinyali


    // paket geldi,
    // gelen paketi acil durum bayrağına göre ayırt edip doğru kuyruğa yerleştircek
    void enqueuePacket(omnetpp::simtime_t creationTime, bool urgent);

    // sunucu boşa çıktığında kuyrukları kontrol edip
    //önce urgent olanları alacak
    //bir sonraki hizmeti başlatacak
    void startNextService();

    // iki kuyrukta bekleyen toplam paket sayısını ne kadar
    //direkt olarak verecek
    int  totalQueueLength() const;

public:
    HospitalServerApp();
    virtual ~HospitalServerApp();
    HospitalServerApp(const HospitalServerApp &other);
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
};

} // namespace flora bitti
#endif
