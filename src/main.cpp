#include <DnsLayer.h>
#include <EthLayer.h>
#include <IPv4Layer.h>
#include <Packet.h>
#include <SystemUtils.h>
#include <UdpLayer.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include "bgp/BGPConnection.h"
#include "configuration/parser/Parser.h"
#include "entities/EndPoint.h"
#include "entities/Router.h"
#include "logger/Logger.h"


using namespace std;

int main(int argc, char *argv[]) {
    srand(time(NULL) + getpid());
    Logger::getInstance()->setTargetLogLevel(LogLevel::DEBUG);
    L_VERBOSE("START");

    pcpp::EthLayer newEthernetLayer(pcpp::MacAddress("11:11:11:11:11:11"),
                                    pcpp::MacAddress("aa:bb:cc:dd:ee:ff"));

    pcpp::IPv4Layer newIPLayer(pcpp::IPv4Address(std::string("192.168.1.1")),
                               pcpp::IPv4Address(std::string("10.0.0.1")));
    newIPLayer.getIPv4Header()->ipId       = pcpp::hostToNet16(2000);
    newIPLayer.getIPv4Header()->timeToLive = 64;

    pcpp::UdpLayer newUdpLayer(12345, 53);

    pcpp::DnsLayer newDnsLayer;
    newDnsLayer.addQuery("www.ebay.com", pcpp::DNS_TYPE_A, pcpp::DNS_CLASS_IN);

    pcpp::Packet newPacket(100);

    newPacket.addLayer(&newEthernetLayer);
    newPacket.addLayer(&newIPLayer);
    newPacket.addLayer(&newUdpLayer);
    newPacket.addLayer(&newDnsLayer);

    newPacket.computeCalculateFields();

    cout << newPacket.toString() << endl;

    vector<Device *> *devices = Parser::parseAndBuild(argv[1]);


    // TODO logic here
    for (auto device : *devices) {
        if (auto *x = dynamic_cast<Router *>(device)) {
            cout << x->ID << endl;
        } else if (auto *x = dynamic_cast<EndPoint *>(device)) {
            cout << x->ID << endl;
        }
        device->start();
    }

    this_thread::sleep_for(5s);

    L_DEBUG("DELETING OBJECTS");
    for (auto device : *devices) {
        delete device;
    }
    delete devices;

    // BGPConnection connection;
    // connection.enqueueEvent(AutomaticStart);
    // this_thread::sleep_for(10s);
    // connection.enqueueEvent(ManualStop);

    L_VERBOSE("END");

    return 0;
}
