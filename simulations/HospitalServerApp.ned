package test2;
import flora.LoRa.NetworkServerApp;
simple HospitalServerApp extends NetworkServerApp {
    parameters:
        @class(flora::HospitalServerApp); 
        int    queueSize        = default(50);
        double expo_rate_lambda = default(5);
        @signal[queueLength](type=long);
        @signal[urgentDropped](type=long);
        @signal[normalDropped](type=long);
        @signal[urgentWaitTime](type=simtime_t);
        @signal[normalWaitTime](type=simtime_t);
        @signal[urgentEndToEndDelay](type=simtime_t);
        @signal[normalEndToEndDelay](type=simtime_t);
        @statistic[queueLength](title="Total queue length"; record=vector,timeavg,max,mean);
        @statistic[urgentDropped](title="Urgent packets dropped"; record=count,sum);
        @statistic[normalDropped](title="Normal packets dropped"; record=count,sum);
        @statistic[urgentWaitTime](title="Urgent wait time (s)"; record=vector,mean,max,histogram);
        @statistic[normalWaitTime](title="Normal wait time (s)"; record=vector,mean,max,histogram);
        @statistic[urgentEndToEndDelay](title="Urgent end-to-end delay (s)"; record=vector,mean,max,histogram);
        @statistic[normalEndToEndDelay](title="Normal end-to-end delay (s)"; record=vector,mean,max,histogram);
}
