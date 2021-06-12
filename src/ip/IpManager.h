#ifndef BGPSIMULATION_ENTITIES_IP_IPMANAGER_H
#define BGPSIMULATION_ENTITIES_IP_IPMANAGER_H

#include <string>
#include <vector>

#include "../entities/NetworkCard.h"
#include "IpAddress.h"
#include "TableRow.h"


class IpManager {
   public:
    static void buildRoutingTable(std::vector<TableRow> &     routingTable,
                                  std::vector<NetworkCard *> *networkCards,
                                  const pcpp::IPv4Address &   defaultGateway);

    /**
     * Get the formatted routing table as string.
     */
    static std::string getRoutingTableAsString(
        const std::vector<TableRow> &routingTable);

    /**
     * Search for the network card that have to send the packet, based on the IP
     * destination address. If no network card can handle the message, a \a
     * nullptr is returned
     * @param dstAddress the destnation address of the packet
     */
    static NetworkCard *findExitingNetworkCard(
        const pcpp::IPv4Address &    dstAddress,
        const std::vector<TableRow> &routingTable);

   private:
    /**
     * Format a string to fit a table cell.
     *
     * @param s the string to format.
     */
    static std::string getTableCellAsString(const std::string &s);
};

#endif  // BGPSIMULATION_ENTITIES_IP_IPMANAGER_H
