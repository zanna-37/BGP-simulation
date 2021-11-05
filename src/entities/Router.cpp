#include "Router.h"

#include <cstdint>
#include <iomanip>
#include <ostream>

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
    char origin = '?';  // this value can be 'i', 'e' or '?'
                        // need to understand if the router is interior or
                        // exterior in our case we can let '?'

    for (NetworkCard *networkCard : *networkCards) {
        std::vector<uint16_t> asPath;

        pcpp::IPv4Address networkIP(networkCard->IP.toInt() &
                                    networkCard->netmask.toInt());

        BGPTableRow row(networkIP,
                        pcpp::IPv4Address(networkCard->netmask),
                        pcpp::IPv4Address::Zero,
                        origin,
                        asPath,
                        0,
                        0,
                        32768);

        row.preferred = true;
        bgpTable.push_back(row);
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
    output += getBgpTableCellAsString("NetworkMask");
    output += getBgpTableCellAsString("NextHop");
    output += getBgpTableCellAsString("Metric");
    output += getBgpTableCellAsString("LocPref");
    output += getBgpTableCellAsString("Weight");
    output += getBgpTableCellAsString("Path");
    output += getBgpTableCellAsString("Preferred");

    for (const BGPTableRow &row : bgpTable) {
        output += "\n";
        output += getBgpTableCellAsString(row.networkIP.toString());
        output += getBgpTableCellAsString(row.networkMask.toString());
        output += getBgpTableCellAsString(row.nextHop.toString());
        output += getBgpTableCellAsString(std::to_string(row.metric));
        output += getBgpTableCellAsString(std::to_string(row.localPreferences));
        output += getBgpTableCellAsString(std::to_string(row.weight));
        string asPAth;
        for (uint16_t i : row.asPath) {
            asPAth += std::to_string(i) + " ";
        }
        output += getBgpTableCellAsString(asPAth);
        output += getBgpTableCellAsString(row.preferred ? "yes" : "no");
    }

    return output;
}
