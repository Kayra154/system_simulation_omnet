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

package test2;
import flora.LoRaApp.SimpleLoRaApp;

simple PatientSensorApp extends SimpleLoRaApp {
    parameters:
        @class(flora::PatientSensorApp); // omnet bizim yaptığımız patientsensorapp logicini kullansın diye signature
        
        double expo_rate_lambda   = default(5);//arrival time 
        double emergencyThreshold = default(40);

        // statistik değerlerini tutmak için
        @signal[urgentPacketSent](type=long);
        @signal[normalPacketSent](type=long);
        @signal[emergencyVariate](type=double);

        // kpi
        @statistic[urgentPacketSent](title="Urgent packets sent"; record=count,sum,vector);
        @statistic[normalPacketSent](title="Normal packets sent"; record=count,sum,vector);
        @statistic[emergencyVariate](title="Exponential variate samples"; record=vector,histogram);
}
