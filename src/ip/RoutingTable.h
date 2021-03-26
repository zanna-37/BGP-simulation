#ifndef ROUTINGTABLE_H
#define ROUTINGTABLE_H

#include <iomanip>
#include <iostream>

#include "TableRow.h"

class RoutingTable {
   public:
    RoutingTable(){};

    ~RoutingTable(){};

    void insertRow(TableRow row);

    void deleteRow(TableRow row);

    void printTable();

   private:
    vector<TableRow> table;

    template <typename T>
    void printElement(T t);
};
#endif