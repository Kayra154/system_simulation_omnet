package test2;
import flora.LoRaApp.SimpleLoRaApp;
simple PatientSensorApp extends SimpleLoRaApp {
    parameters:
        @class(flora::PatientSensorApp); 
        double expo_rate_lambda   = default(5);
        double emergencyThreshold = default(40);
        @signal[urgentPacketSent](type=long);
        @signal[normalPacketSent](type=long);
        @signal[emergencyVariate](type=double);
        @statistic[urgentPacketSent](title="Urgent packets sent"; record=count,sum,vector);
        @statistic[normalPacketSent](title="Normal packets sent"; record=count,sum,vector);
        @statistic[emergencyVariate](title="Exponential variate samples"; record=vector,histogram);
}
