#ifndef ROUTINGTABLE_H
#define ROUTINGTABLE_H

#include <vector>

#include "../entities/NetworkCard.h"
#include "TableRow.h"

// forward declarations
#include "../entities/NetworkCard.fwd.h"
#include "TableRow.fwd.h"

class RoutingTable {
   public:
    RoutingTable();

    ~RoutingTable();

    void insertRow(TableRow* row);

    void deleteRow(TableRow* row);

    void printTable();

    NetworkCard* findNextHop(pcpp::IPv4Address dstAddress);

   private:
    std::vector<TableRow*>* table = nullptr;

    template <typename T>
    void printElement(T t);
};
#endif