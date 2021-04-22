#include "Router.h"

#include <iomanip>
#include <iostream>

#include "../bgp/ASPath.h"
#include "../bgp/BGPApplication.h"
#include "../bgp/BGPTableRow.h"
#include "Layer.h"
#include "Link.h"
#include "NetworkCard.h"


Router::~Router() {
    // Note: all the subclasses need to call Device::shutdown() in their
    // deconstructor. Keep them in sync.
    shutdown();

    delete bgpApplication;

    for (BGPTableRow *row : *bgpTable) {
        delete row;
    }
    delete bgpTable;
}

void Router::forwardMessage(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers,
    NetworkCard *                                             networkCard) {
    networkCard->sendPacket(std::move(layers));
}

void Router::bootUpInternal() {
    bgpApplication = new BGPApplication(this, this->peer_addresses.front());
    bgpApplication->passiveOpenAll();
}

void Router::setUpRIP(vector<NetworkCard *> *networkCards) {
    bgpTable = new std::vector<BGPTableRow *>();

    char origin =
        'i';  // this value can be 'i', 'e' or '?'
              // need to understand if the router is interior or exterior

    ASPath loopback_asPath;

    loopback_asPath.add(
        "i");  // this value can be 'i', 'e' or '?'
               // need to understand if the router is interior or exterior

    BGPTableRow *loopback_row =
        new BGPTableRow(loopbackIP,
                        pcpp::IPv4Address("255.255.255.255"),
                        pcpp::IPv4Address::Zero,
                        origin,
                        loopback_asPath,
                        0,
                        0,
                        32768);

    bgpTable->push_back(loopback_row);

    for (NetworkCard *networkCard : *networkCards) {
        if (networkCard->owner->ID[0] == 'R') {
            NetworkCard *networkCardPeer =
                networkCard->link->getPeerNetworkCardOrNull(networkCard);

            if (networkCardPeer->owner->ID[0] == 'R') {
                ASPath asPath;

                asPath.add("i");

                pcpp::IPv4Address networkIP(networkCard->IP.toInt() &
                                            networkCard->netmask.toInt());

                BGPTableRow *row =
                    new BGPTableRow(networkIP,
                                    pcpp::IPv4Address(networkCard->netmask),
                                    pcpp::IPv4Address::Zero,
                                    origin,
                                    asPath,
                                    0,
                                    0,
                                    32768);

                pcpp::IPv4Address networkIPPeer(
                    networkCardPeer->IP.toInt() &
                    networkCardPeer->netmask.toInt());

                std::string numIDPeer =
                    std::string(1, networkCardPeer->owner->ID[1]);

                asPath.add(" " + numIDPeer);

                BGPTableRow *rowPeer =
                    new BGPTableRow(networkIPPeer,
                                    pcpp::IPv4Address(networkCardPeer->netmask),
                                    networkCardPeer->IP,
                                    origin,
                                    asPath,
                                    0,
                                    0,
                                    32768);


                bgpTable->push_back(row);
                bgpTable->push_back(rowPeer);
            }
        }
    }
    printBGPTable();
}

void Router::printElement(std::string t) {
    // TODO (zanna): make it return a string a print with logger
    const char separator = ' ';
    const int  width     = 16;
    std::cout << left << setw(width) << setfill(separator) << t;
}

void Router::printBGPTable() {
    // TODO (zanna): make it return a string a print with logger
    printElement("Network");
    printElement("NextHop");
    printElement("Metric");
    printElement("LocPref");
    printElement("Weight");
    printElement("Path");
    std::cout << std::endl;
    for (BGPTableRow *row : *bgpTable) {
        printElement(row->networkIP.toString());
        printElement(row->nextHop.toString());
        printElement(std::to_string(row->metric));
        printElement(std::to_string(row->localPreferences));
        printElement(std::to_string(row->weight));
        for (int i = 0; i < row->asPath.autonomousSystems.size(); i++) {
            std::cout << row->asPath.autonomousSystems[i];
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}
