#include <DnsLayer.h>
#include <EthLayer.h>
#include <IPv4Layer.h>
#include <Packet.h>
#include <SystemUtils.h>
#include <UdpLayer.h>

#include <iostream>
#include <string>

#include "entities/Router.h"
#include "yaml-cpp/yaml.h"

using namespace std;

int main(int argc, char *argv[]) {

    cout << "Hello, World!" << endl;

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

    Router r("R1", "AS_12345", nullptr);
    cout << r.ID << endl;

    YAML::Node config = YAML::LoadFile(argv[1]);

    return 0;
}
