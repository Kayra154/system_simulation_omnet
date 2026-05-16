package test2;
import inet.applications.udpapp.UdpBasicApp;
import inet.node.inet.Router;
import inet.node.internetcloud.InternetCloud;
import flora.LoRaPhy.LoRaMedium;
import flora.LoraNode.LoRaNode;
import flora.LoraNode.LoRaGW;
import inet.node.inet.StandardHost;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth1G;
import inet.queueing.sink.ActivePacketSink;
import inet.queueing.queue.PacketQueue;
import inet.queueing.classifier.PriorityClassifier;
import inet.queueing.source.ActivePacketSource;

@license(LGPL);

network LoRaNetworkTest {
    parameters:
        int numPatients = default(20);
        
    submodules:
        loRaNodes[numPatients]: LoRaNode {
            @display("p=287,373");
        }
        loRaGateway: LoRaGW {
            @display("p=262,320;is=s");
        }
        LoRaMedium: LoRaMedium {
            @display("p=833,64");
        }
        HospitalServer: StandardHost {
            @display("p=157,182");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                assignDisjunctSubnetAddresses = false;
                @display("p=830,167");
        }
        gwRouter: Router {
            @display("p=135,287");
        }
    connections:
        gwRouter.ethg++ <--> Eth1G <--> loRaGateway.ethg++;
        gwRouter.ethg++ <--> Eth1G <--> HospitalServer.ethg++;
}
