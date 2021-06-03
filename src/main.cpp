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


int main(int argc, char *argv[]) {
    // Seed the random generator
    srand(time(nullptr) + getpid());

    // Get the first instance of the logger and set its options
    Logger::getInstance()->setTargetLogLevel(LogLevel::DEBUG);

    L_VERBOSE("main", "START");

    // TODO REMOVE ME, just examples
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
    std::cout << bgpOpen.toString() << std::endl;

    // BGP Update
    std::vector<LengthAndIpPrefix> withdrawnRoutes;
    withdrawnRoutes.emplace_back(20, "10.3.2.1");
    withdrawnRoutes.emplace_back(24, "10.6.5.4");

    std::vector<PathAttribute> pathAttributes;
    PathAttribute              pathAttribute;
    const int                  data_len       = 10;
    uint8_t                    data[data_len] = {'a', 'b'};
    pathAttribute.setAttributeLengthAndValue(data, data_len);
    pathAttribute.attributeTypeCode =
        PathAttribute::AttributeTypeCode_uint8_t::NEXT_HOP;
    pathAttributes.push_back(pathAttribute);
    std::vector<LengthAndIpPrefix> nlri;
    nlri.emplace_back(4, "10.9.8.7");
    BGPUpdateLayer bgpUpdate =
        BGPUpdateLayer(withdrawnRoutes, pathAttributes, nlri);
    bgpUpdate.computeCalculateFields();
    std::cout << bgpUpdate.toString() << std::endl;

    // BGP Keepalive
    BGPKeepaliveLayer bgpKeepalive = BGPKeepaliveLayer();
    bgpKeepalive.computeCalculateFields();
    std::cout << bgpKeepalive.toString() << std::endl;

    // BGP Notification
    uint8_t              notificationData[] = {'a', '0'};
    BGPNotificationLayer bgpNotification    = BGPNotificationLayer(
        BGPNotificationLayer::ErrorCode_uint8_t::MSG_HEADER_ERR,
        BGPNotificationLayer::ErrorSubcode_uint8_t::ERR_1_BAD_MSG_TYPE,
        notificationData,
        sizeof(notificationData));
    bgpNotification.computeCalculateFields();
    std::cout << bgpNotification.toString() << std::endl;


    // Add to packet

    pcpp::Packet newPacket(100);

    newPacket.addLayer(&newEthernetLayer);
    newPacket.addLayer(&newIPLayer);
    // newPacket.addLayer(&bgpOpen);
    // newPacket.addLayer(&bgpUpdate);
    // newPacket.addLayer(&bgpKeepalive);
    newPacket.addLayer(&bgpNotification);

    newPacket.computeCalculateFields();

    std::cout << newPacket.toString() << std::endl;
    // END: REMOVE ME just examples

    if (argc > 1) {
        std::vector<Device *> *devices = Parser::parseAndBuild(argv[1]);

        for (auto device : *devices) {
            /* TODO REMOVE ME, just an example
            if (auto *x = dynamic_cast<Router *>(device)) {
                cout << x->ID << endl;
            } else if (auto *x = dynamic_cast<EndPoint *>(device)) {
                cout << x->ID << endl;
            }
            */
            device->bootUp();
            std::this_thread::sleep_for(500ms);  // TODO remove me
        }

        // devices->at(0)->listen();
        // std::string testAddress("90.36.25.1");
        // devices->at(2)->connect(testAddress, 179);

        // this_thread::sleep_for(5s);
        // devices->at(2)->resetConnection(testAddress, 179);
        // this_thread::sleep_for(5s);
        // devices->at(2)->closeConnection(testAddress, 179);
        // this_thread::sleep_for(2s);
        // testAddress = "90.36.25.123";
        // devices->at(0)->closeConnection(testAddress, 12345);
        // this_thread::sleep_for(5s);

        //         BGPConnection connection(dynamic_cast<Router
        //         *>(devices->at(0))); connection.enqueueEvent(AutomaticStart);
        std::this_thread::sleep_for(20s);
        //         connection.enqueueEvent(ManualStop);

        L_INFO("main", "SHUTTING DOWN SIMULATION");
        for (auto device : *devices) {
            delete device;
        }
        delete devices;
    } else {
        L_FATAL("main",
                "Too few arguments. Did you specify the configuration file?");
    }

    L_VERBOSE("main", "END");

    return 0;
}
