#include "HospitalServerApp.h"
#include "NumGenerator.h"
#include "LoRaApp/LoRaAppPacket_m.h"

// inet paket açsın kapatsın diye bunları ekledik buraya
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/Chunk.h"

namespace flora { // flora boku tam çalışsın simülasyonda patlamayalım bizim cpyı görsün diye namespace attık

Define_Module(HospitalServerApp); // omnetee bu sınıf simülasyon modülü dedik

HospitalServerApp::HospitalServerApp() { }
HospitalServerApp::~HospitalServerApp() { }
// kopyalama hatasını burayı kapatıp gömdük patlamasın diye

void HospitalServerApp::initialize(int stage) {
    NetworkServerApp::initialize(stage); // atradan gelen standart ayarları çalıştırdık

    if (stage == inet::INITSTAGE_LOCAL) { // simülasyon ilk başlarken ayarları yüklüyoruz burda ınıtstage local 0 demek aslında stage 0
        inet::Chunk::enableImplicitChunkSerialization = true; // inet arkada paket açamam diye zırlamasın diye burayı true çektik
        serviceRate = par("expo_rate_lambda").doubleValue(); // omnetppini inideki sunucu servis hızını içeri aldık lambdayı yani
        queueSize   = par("queueSize").intValue(); // düzü 50 değişir dediğimiz kuyruk sınırı parametresi bu

        // grafik motoruna veri kaydetsindiye  program çalışırken kaydettik tek tek
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
    // simülasyon durunca sayısal verileri alacağımız ve skalar olarak kaydettiğimiz
    //bu yeri line plotter çizmezse pythonda parse edeceğimiz sayaçları burda sca şeklinde alcaz
    recordScalar("urgentReceivedTotal", urgentReceivedCounter);
    recordScalar("normalReceivedTotal", normalReceivedCounter);
    recordScalar("urgentDroppedTotal",  urgentDroppedCounter);
    recordScalar("normalDroppedTotal",  normalDroppedCounter);
    recordScalar("urgentServedTotal",   urgentServedCounter);
    recordScalar("normalServedTotal",   normalServedCounter);
    //bunlar olmayınca simülasyon patlıyordu

    // sıfıra bölme hatası verip patlamasın diye  kayıp oranlarını hesaplattık
    if (urgentReceivedCounter > 0)
        recordScalar("urgentLossRate", (double)urgentDroppedCounter / urgentReceivedCounter);
    if (normalReceivedCounter > 0)
        recordScalar("normalLossRate", (double)normalDroppedCounter / normalReceivedCounter);

    NetworkServerApp::finish();
}

// iki kuyrukta bekleyen toplam amele sayısını veren fonksiyon helper gibi hesaplamada kolaylık oluyor
int HospitalServerApp::totalQueueLength() const {
    return (int)(urgentQueue.size() + normalQueue.size());
}

// paket geldiğinde acil durum bayrağına göre ayırıp doğru kuyruğa yollayan operasyon
void HospitalServerApp::enqueuePacket(omnetpp::simtime_t creationTime, bool urgent) {
    // referans pointer muhabbetiyle tek satırda doğru kuyruğu ve drop sayacını seçtik
    // bunu claude abi verdi valla koy çalışır dedi çalıştı
    auto &q             = urgent ? urgentQueue : normalQueue;
    auto &dropCounter   = urgent ? urgentDroppedCounter : normalDroppedCounter;
    auto  dropSignal    = urgent ? urgentDroppedSignal  : normalDroppedSignal;

    if ((int)q.size() >= queueSize) { // seçtiğimiz kuyruk dolduysa sınırı yani normal hesaba göre 50yi aştıysa
        dropCounter++; // paketi içeri almayıp çöpe attık drop sayacını +1
        emit(dropSignal, 1L); // süre hesabında lazım olacak drop sinyalini flag gibi fırlattık böylece drop var olduğunu anladık
    } else {
        q.push(creationTime); // kuyrukta yer varsa paketin sensörde doğduğu zamanı kaydettik
        emit(queueLengthSignal, (long)totalQueueLength()); // güncel kuyruk sizeını grafiğe yolladık

        if (!serverBusy) { // sunucu o an boşta yatıyorsa paketi işlemek için servisi başlattık
            startNextService();
        }
    }
}

// sunucu boşa çıkınca kuyrukları kontrol edip önce urgent olanları alan ana kuyruk ayıklama logic
void HospitalServerApp::startNextService() {
    bool serveUrgent;
    if (!urgentQueue.empty())      serveUrgent = true; // acil kuyruğunda tek paket bile varsa önce hep onu alıyoruz
    else if (!normalQueue.empty()) serveUrgent = false; // acil tamamen boşsa normalleri işleme alıp normalleri alıyoruz
    else { serverBusy = false; return; } // iki kuyruk da bomboşsa meşguliyeti kapatıp uykuya geçiyor

    serverBusy = true; // sunucuyu meşgul moda aldık hiçbir ife takılmadıysa meşguldür
    currentlyServingUrgent = serveUrgent; // işlenen paket urgent mi normal mi state olarak tuttuk burda
    auto &q = serveUrgent ? urgentQueue : normalQueue;

    omnetpp::simtime_t creationTime = q.front(); // kuyruğun en önündeki ilk gelen elemanı çektik fcfs yani
    q.pop(); // çektiğimiz elemanı kuyruktan sildik
    emit(queueLengthSignal, (long)totalQueueLength()); // eleman eksildiği için güncel boyuti fırlattık tekrar

    // bekleme süresi hesabı şu anki saatten paketin üretim saatini çıkarıp kaç saniye işlem döndü bulduk
    omnetpp::simtime_t waitTime = simTime() - creationTime;
    emit(serveUrgent ? urgentWaitTimeSignal : normalWaitTimeSignal, waitTime); // hesaplanan süreyi istatistik olarak yazmak için sinyale pasladık flag gibi
    emit(serveUrgent ? urgentEndToEndDelaySignal : normalEndToEndDelaySignal, waitTime);

    if (serveUrgent) urgentServedCounter++; //  işlenen paketlerin sayaçlarını bir artırdık
    else normalServedCounter++; //normalse de normali arttırdık

    // hata düzelltiğimiz numgeneratordan üstel dağılımlı service süresi çektik
    omnetpp::simtime_t serviceDuration = NumGenerator::exponential(serviceRate);
    // simülatöre alarm kurduk şu saate servis süresini ekle bitince servicecomplete mesajı yolla
    scheduleAt(simTime() + serviceDuration, new omnetpp::cMessage("serviceComplete"));
}

// ağdan paket gelince ya da sunucunun kendi alarmı tetiklenince çalışan ana merkez kapısı
void HospitalServerApp::handleMessage(omnetpp::cMessage *msg) {
    if (msg->arrivedOn("socketIn")) { // gelen mesaj dışardan yani ağdan gelen hasta paketi mi ona baktık
        auto pkt = inet::check_and_cast<inet::Packet *>(msg); // mesajı güvenli şekilde inet paket tipine çevirdik

        if (simTime() >= getSimulation()->getWarmupPeriod()) { // ısınma periyodu yani ilk 60 saniye bittiyse işlemeye başladık toaha dedi warmup olsun diye
                    try {
                        // şimdi burada önemli bir olay var
                        //normal şartlarda bizim yolladığımız dataframe tek parçadan oluşmuyor
                        //bu amınakodumun paketi macframe denen bi bokla örtülü geliyor
                        //bu da simülasyonda packet could couldnt parsed gibi saçma sapan gariban omnet abinin paketin içeriğini okumasını engelliyor
                        //claude abi de diyor ki madem öyle
                        //sen macframe diye bir temp aç
                        //macframe denen şeyi oraya koy
                        //içeriği oku
                        //geri kapat paket direkt gitsin
                        //oldu öyle olunca

                        // 1 zırhı çıkar inet serializer hatası verip patlamasın diye dıştaki loramacframei sökmek için pop attık
                        auto macFrame = pkt->popAtFront<LoRaMacFrame>();

                        // 2 veriyi oku kabuk soyulunca içteki loraapppacket öne çıktı peek atıp acil durum bayrağı 1 mi 0 mı baktık
                        auto appPkt = pkt->peekAtFront<LoRaAppPacket>();
                        bool urgent = (appPkt->getSampleMeasurement() == 1); // sensörün yazdığı 1 veya 0 bilgisini çektik

                        if (urgent) urgentReceivedCounter++; // paketin türüne göre sunucuya ulaşan paket sayacını güncelledik
                        else normalReceivedCounter++;

                        omnetpp::simtime_t creationTime = pkt->getCreationTime(); // paketin ilk üretim saatini alıp kuyruğa yolladık
                        enqueuePacket(creationTime, urgent);

                        // 3. adım macframei geri tak kütüphanenin alt katmanları kırılmasın diye söktüğümüz mac bilmemnesini önüne aynen geri taktık insertle
                        pkt->insertAtFront(macFrame);
                    }
                    catch (std::exception &e) { // burası bir şekil patlarsa simülasyonu çökertme konsola uyarı bas devam et dedik
                        EV_WARN << "Could not parse LoRaAppPacket: " << e.what() << std::endl;
                    }
                }

        updateKnownNodes(pkt);
        processLoraMACPacket(pkt);
    }
    else if (msg->isSelfMessage() && strcmp(msg->getName(), "serviceComplete") == 0) {
        // sunucunun kendine kurduğu hizmet bitti alarmı tetiklenince burası çalışıyor
        delete msg; // alarm mesajıyla iş bitti çöp olmasın diye bellekten sildik
        serverBusy = false; // sunucunya boşta dedik meşguliyeti kapattık
        startNextService(); // kuyrukta bekleyen sonraki ğaketleri eritmek için başa sardık
    }
    else if (msg->isSelfMessage()) { //sonra devam
        processScheduledPacket(msg);
    }
}

} // namespace flora bitti
