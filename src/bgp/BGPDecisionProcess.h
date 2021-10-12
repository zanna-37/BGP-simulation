#ifndef BGPSIMULATION_BGP_BGPDECISIONPROCESS_H
#define BGPSIMULATION_BGP_BGPDECISIONPROCESS_H

#include <vector>

#include "../entities/Router.h"
#include "../ip/TableRow.h"
#include "BGPConnection.h"
#include "BGPTableRow.h"
#include "packets/BGPUpdateLayer.h"

void runDecisionProcess(Router *                         router,
                        std::unique_ptr<BGPUpdateLayer> &BGPUpdateMessage,
                        pcpp::IPv4Address &              routerIP,
                        BGPConnection *                  bgpConnectionToAvoid);

void calculatePreferredRoute(BGPTableRow *newRoute,
                             BGPTableRow *currentPreferredRoute);

void updateIPTable(std::vector<TableRow> &IPTable,
                   BGPTableRow &          newRoute,
                   bool                   isWithDrawnRoute);

#endif  // BGPSIMULATION_BGP_BGPDECISIONPROCESS_H
