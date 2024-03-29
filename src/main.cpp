#include <unistd.h>

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "configuration/parser/Parser.h"
#include "entities/Device.h"
#include "entities/EndPoint.h"
#include "entities/Router.h"
#include "logger/Logger.h"
#include "pistache/net.h"
#include "pistache/os.h"
#include "server/Server.h"


// Variable for sig interrupt (Ctrl+C)
volatile sig_atomic_t stop;

/**
 * @brief Startup of the server
 *
 * @param devices Vector of the Devices parsed for the configuration of the
 * program
 */
int start_server(unsigned short int     serverPort,
                 std::vector<Device *> *devices) {
    Pistache::Port    port(serverPort);
    int               thr = 2;
    Pistache::Address addr(Pistache::Ipv4::any(), port);

    // Start rest server
    L_DEBUG(
        "Server",
        "Hardware cores: " + std::to_string(Pistache::hardware_concurrency()));
    L_DEBUG("Server", "Using " + std::to_string(thr) + " threads");

    ApiEndpoint stats(addr);

    stats.init(thr, devices);
    stats.start(&stop);

    return 0;
}

/**
 * @brief
 * https://stackoverflow.com/questions/26965508/infinite-while-loop-and-control-c
 *
 * @param signum
 */
void inthand(int signum) {
    if (stop == false) {
        stop = 1;
    } else {
        L_ERROR("main", "Exit forcefully");
        std::abort();
    }
}

int main(int argc, char *argv[]) {
    // Seed the random generator
    srand(time(nullptr) + getpid());

    // Get the first instance of the logger and set its options
    Logger::getInstance()->setTargetLogLevel(LogLevel::DEBUG);

    L_VERBOSE("main", "START");
    int rc = 1;

    // TODO REMOVE ME, just examples
    // pcpp::EthLayer  newEthernetLayer(pcpp::MacAddress("11:11:11:11:11:11"),
    //                                 pcpp::MacAddress("aa:bb:cc:dd:ee:ff"));
    // pcpp::IPv4Layer newIPLayer(pcpp::IPv4Address(std::string("192.168.1.1")),
    //                            pcpp::IPv4Address(std::string("10.0.0.1")));
    // newIPLayer.getIPv4Header()->ipId       = pcpp::hostToNet16(2000);
    // newIPLayer.getIPv4Header()->timeToLive = 64;
    //
    //// BGP Open
    // BGPOpenLayer bgpOpen =
    //     BGPOpenLayer(1, 2, pcpp::IPv4Address(std::string("10.0.0.1")));
    // bgpOpen.computeCalculateFields();
    // std::cout << bgpOpen.toString() << std::endl;
    //
    //// BGP Update
    // std::vector<LengthAndIpPrefix> withdrawnRoutes;
    // withdrawnRoutes.emplace_back(20, "10.3.2.1");
    // withdrawnRoutes.emplace_back(24, "10.6.5.4");
    //
    // std::vector<PathAttribute> pathAttributes;
    // PathAttribute              pathAttribute;
    // const int                  data_len       = 10;
    // uint8_t                    data[data_len] = {'a', 'b'};
    // pathAttribute.setAttributeLengthAndValue(data, data_len);
    // pathAttribute.attributeTypeCode =
    //     PathAttribute::AttributeTypeCode_uint8_t::NEXT_HOP;
    // pathAttributes.push_back(pathAttribute);
    // std::vector<LengthAndIpPrefix> nlri;
    // nlri.emplace_back(4, "10.9.8.7");
    // BGPUpdateLayer bgpUpdate =
    //     BGPUpdateLayer(withdrawnRoutes, pathAttributes, nlri);
    // bgpUpdate.computeCalculateFields();
    // std::cout << bgpUpdate.toString() << std::endl;
    //
    //// BGP Keepalive
    // BGPKeepaliveLayer bgpKeepalive = BGPKeepaliveLayer();
    // bgpKeepalive.computeCalculateFields();
    // std::cout << bgpKeepalive.toString() << std::endl;
    //
    //// BGP Notification
    // uint8_t              notificationData[] = {'a', '0'};
    // BGPNotificationLayer bgpNotification    = BGPNotificationLayer(
    //     BGPNotificationLayer::ErrorCode_uint8_t::MSG_HEADER_ERR,
    //     BGPNotificationLayer::ErrorSubcode_uint8_t::ERR_1_BAD_MSG_TYPE,
    //     notificationData,
    //     sizeof(notificationData));
    // bgpNotification.computeCalculateFields();
    // std::cout << bgpNotification.toString() << std::endl;
    //
    //
    //// Add to packet
    // pcpp::Packet newPacket(100);
    //
    // newPacket.addLayer(&newEthernetLayer);
    // newPacket.addLayer(&newIPLayer);
    //// newPacket.addLayer(&bgpOpen);
    //// newPacket.addLayer(&bgpUpdate);
    //// newPacket.addLayer(&bgpKeepalive);
    // newPacket.addLayer(&bgpNotification);
    //
    // newPacket.computeCalculateFields();
    //
    // std::cout << newPacket.toString() << std::endl;
    // END: REMOVE ME just examples

    // TODO set it with a command line argument bu provide a default
    unsigned short int port = 8080;  // If you change the port, keep all the
                                     // docker-compose files in sync

    if (argc > 1) {
        std::vector<Device *> *devices = Parser::parseAndBuild(argv[1]);

        signal(SIGINT, inthand);
        auto *serverThread = new std::thread([&]() {
            rc = start_server(port, devices);
            if (rc == 0) {
                L_INFO("Server",
                       "SIGINT received, Pistache shutdown correctly!");
            } else {
                L_ERROR("Server", "Shutdown Failed!");
            }
        });

        L_SUCCESS("main",
                  "SERVER STARTING...\nOpen the web interface at "
                  "http://localhost:" +
                      std::to_string(port) +
                      "/showGUI\nPress Ctrl+C to end the simulation");

        for (auto device : *devices) {
            // if (auto *x = dynamic_cast<Router *>(device)) {
            // cout << x->ID << endl;
            // } else if (auto *x = dynamic_cast<EndPoint *>(device)) {
            // cout << x->ID << endl;
            // }
            device->bootUp();
            std::this_thread::sleep_for(
                std::chrono::milliseconds(500));  // TODO remove me
        }

        // TODO REMOVE ME, just examples
        /*std::this_thread::sleep_for(std::chrono::milliseconds(20000));
        auto *sender = dynamic_cast<Device *>(devices->at(1));
        sender->ping(pcpp::IPv4Address("199.4.0.104"));*/

        while (!stop) {
            sleep(1);  // TODO change from polling to wait
        }

        L_VERBOSE("main", "SIGINT (Ctrl+C) received. Exiting...");

        // Once the Ctrl+C is pressed all the threads stop
        serverThread->join();
        delete serverThread;
        serverThread = nullptr;

        L_INFO("main", "SHUTTING DOWN DEVICES");
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
