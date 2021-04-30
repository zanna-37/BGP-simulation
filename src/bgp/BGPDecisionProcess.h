#ifndef BGPDECISIONPROCESS_H
#define BGPDECISIONPROCESS_H

#include <BGPTableRow.h>
#include <entities/Router.h>
#include <ip/TableRow.h>
#include <packets/BGPUpdateLayer.h>

#include <vector>

// forward declarations
#include <packets/BGPUpdateLayer.h>

void runDecisionProcess(Router *router, BGPUpdateLayer *BGPUpdateMessage);

void calculatePreferredRoute(BGPTableRow *newRoute,
                             BGPTableRow *currentPreferredRoute);

void updateIPTable(std::vector<TableRow *> *IPTable, BGPTableRow *newRoute);

#endif