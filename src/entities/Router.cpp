#include "Router.h"

#include <iomanip>
#include <ostream>
#include <cstdint>
#include "../bgp/ASPath.h"
#include "../bgp/BGPApplication.h"
#include "../bgp/BGPTableRow.h"
#include "../logger/Logger.h"
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
    buildBgpTable();
    std::string bgpTableAsString = getBgpTableAsString();
    L_VERBOSE(ID, "BGP table:\n" + bgpTableAsString);

    bgpApplication = new BGPApplication(this, this->peer_addresses.front());
    bgpApplication->passiveOpenAll();
}

void Router::buildBgpTable() {
    bgpTable = new std::vector<BGPTableRow *>();

    char origin = 'i';  // this value can be 'i', 'e' or '?'
    // need to understand if the router is interior or exterior

    std::vector<uint16_t> loopback_asPath;  // empty vector

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
                std::vector<uint16_t> asPath;

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

                asPath.push_back(
                    std::stoi(numIDPeer));  // in asPath vector we don't
                // consider the AS itself

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
}

std::string Router::getBgpTableCellAsString(const std::string &s) {
    const char         separator = ' ';
    const int          width     = 16;
    std::ostringstream oss;

    oss << std::left << std::setw(width) << std::setfill(separator) << s;
    return oss.str();
}

std::string Router::getBgpTableAsString() {
    std::string output;

    output += getBgpTableCellAsString("Network");
    output += getBgpTableCellAsString("NextHop");
    output += getBgpTableCellAsString("Metric");
    output += getBgpTableCellAsString("LocPref");
    output += getBgpTableCellAsString("Weight");
    output += getBgpTableCellAsString("Path");

    for (BGPTableRow *row : *bgpTable) {
        output += "\n";
        output += getBgpTableCellAsString(row->networkIP.toString());
        output += getBgpTableCellAsString(row->nextHop.toString());
        output += getBgpTableCellAsString(std::to_string(row->metric));
        output +=
            getBgpTableCellAsString(std::to_string(row->localPreferences));
        output += getBgpTableCellAsString(std::to_string(row->weight));
        string asPAth;
        for (uint16_t i : row->asPath) {
            asPAth += std::to_string(i) + " ";
        }
        output += getBgpTableCellAsString(asPAth);
    }

    return output;
}
