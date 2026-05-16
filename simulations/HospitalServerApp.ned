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
import flora.LoRa.NetworkServerApp;

simple HospitalServerApp extends NetworkServerApp {
    parameters:
        @class(flora::HospitalServerApp); // override için lazım
        
        int    queueSize        = default(50);
        double expo_rate_lambda = default(5);//service rate

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