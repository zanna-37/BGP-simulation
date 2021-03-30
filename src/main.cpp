#include <BgpLayer.h>
#include <DnsLayer.h>
#include <EthLayer.h>
#include <IPv4Layer.h>
#include <Packet.h>
#include <SystemUtils.h>
#include <UdpLayer.h>
#include <unistd.h>

#include <iostream>
#include <stack>
#include <string>

#include "bgp/BGPConnection.h"
#include "bgp/packets/BGPKeepaliveLayer.h"
#include "bgp/packets/BGPNotificationLayer.h"
#include "bgp/packets/BGPOpenLayer.h"
#include "bgp/packets/BGPUpdateLayer.h"
#include "configuration/parser/Parser.h"
#include "entities/EndPoint.h"
#include "entities/Router.h"
#include "logger/Logger.h"
#include "server/Server.h"


using namespace std;

int main(int argc, char *argv[]) {
    srand(time(NULL) + getpid());
    Logger::getInstance()->setTargetLogLevel(LogLevel::DEBUG);
    L_VERBOSE("main", "START");

    // Tests

    pcpp::EthLayer  newEthernetLayer(pcpp::MacAddress("11:11:11:11:11:11"),
                                    pcpp::MacAddress("aa:bb:cc:dd:ee:ff"));
    pcpp::IPv4Layer newIPLayer(pcpp::IPv4Address(std::string("192.168.1.1")),
                               pcpp::IPv4Address(std::string("10.0.0.1")));
    newIPLayer.getIPv4Header()->ipId       = pcpp::hostToNet16(2000);
    newIPLayer.getIPv4Header()->timeToLive = 64;

    // BGP Open
    BGPOpenLayer bgpOpen =
        BGPOpenLayer(1, 2, pcpp::IPv4Address(std::string("10.0.0.1")));
    bgpOpen.computeCalculateFields();
    cout << bgpOpen.toString() << endl;

    // BGP Update
    std::vector<LengthAndIpPrefix> withdrawnRoutes;
    withdrawnRoutes.push_back(LengthAndIpPrefix(20, "10.3.2.1"));
    withdrawnRoutes.push_back(LengthAndIpPrefix(24, "10.6.5.4"));

    std::vector<PathAttribute> pathAttributes;
    PathAttribute              pathAttribute;
    const int                  data_len       = 10;
    uint8_t                    data[data_len] = {'a', 'b'};
    pathAttribute.setAttributeLengthAndValue(data, data_len);
    pathAttribute.attributeTypeCode =
        PathAttribute::AttributeTypeCode_uint8_t::NEXT_HOP;
    pathAttributes.push_back(pathAttribute);
    std::vector<LengthAndIpPrefix> nlri;
    nlri.push_back(LengthAndIpPrefix(4, "10.9.8.7"));
    BGPUpdateLayer bgpUpdate =
        BGPUpdateLayer(withdrawnRoutes, pathAttributes, nlri);
    bgpUpdate.computeCalculateFields();
    cout << bgpUpdate.toString() << endl;

    // BGP Keepalive
    BGPKeepaliveLayer bgpKeepalive = BGPKeepaliveLayer();
    bgpKeepalive.computeCalculateFields();
    cout << bgpKeepalive.toString() << endl;

    // BGP Notification
    uint8_t              notificationData[] = {'a', '0'};
    BGPNotificationLayer bgpNotification    = BGPNotificationLayer(
        BGPNotificationLayer::ErrorCode_uint8_t::MSG_HEADER_ERR,
        BGPNotificationLayer::ErrorSubcode_uint8_t::ERR_1_BAD_MSG_TYPE,
        notificationData,
        sizeof(notificationData));
    bgpNotification.computeCalculateFields();
    cout << bgpNotification.toString() << endl;


    // Add to packet

    pcpp::Packet newPacket(100);

    newPacket.addLayer(&newEthernetLayer);
    newPacket.addLayer(&newIPLayer);
    // newPacket.addLayer(&bgpOpen);
    // newPacket.addLayer(&bgpUpdate);
    // newPacket.addLayer(&bgpKeepalive);
    newPacket.addLayer(&bgpNotification);

    newPacket.computeCalculateFields();

    cout << newPacket.toString() << endl;

    // End tests

    vector<Device *> *devices = Parser::parseAndBuild(argv[1]);


    // TODO logic here

        // Define address port and and threads for the rest server
    Pistache::Port port(9080);
    int thr = 2;
    Pistache::Address addr(Ipv4::any(), port);

    //Start rest server
    cout << "Cores = " << hardware_concurrency() << endl;
    cout << "Using " << thr << " threads" << endl;

    ApiEndpoint stats(addr);

    stats.init(thr);
    stats.start();

    // Debug trial
    for (auto device : *devices) {
        if (auto *x = dynamic_cast<Router *>(device)) {
            cout << x->ID << endl;
        } else if (auto *x = dynamic_cast<EndPoint *>(device)) {
            cout << x->ID << endl;
        }
        device->start();
    }

    devices->at(0)->listen();
    std::string testAddress("90.36.25.1");
    devices->at(2)->connect(testAddress, 179);

    this_thread::sleep_for(5s);
    devices->at(2)->resetConnection(testAddress, 179);
    this_thread::sleep_for(5s);
    devices->at(2)->closeConnection(testAddress, 179);
    this_thread::sleep_for(2s);
    testAddress = "90.36.25.123";
    devices->at(0)->closeConnection(testAddress, 12345);
    this_thread::sleep_for(5s);

    // BGPConnection connection(devices->at(0));
    // connection.enqueueEvent(AutomaticStart);
    // this_thread::sleep_for(10s);
    // connection.enqueueEvent(ManualStop);

    L_DEBUG("main", "DELETING OBJECTS");
    for (auto device : *devices) {
        delete device;
    }
    delete devices;


    L_VERBOSE("main", "END");

    return 0;
}
