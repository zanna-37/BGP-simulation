#ifndef BGPSIMULATION_ENTITIES_LINK_H
#define BGPSIMULATION_ENTITIES_LINK_H

#include "NetworkCard.h"


using namespace std;

enum Connection_status { active, failed };

class NetworkCard;  // forward declaration
class Link {
   public:
    pair<NetworkCard *, NetworkCard *> device_source_networkCards;
    Connection_status                  connection_status;

    Link(Connection_status connection_status)
        : connection_status(connection_status) {}

    void disconnect(NetworkCard *networkCard);

    void connect(NetworkCard *networkCard);

    NetworkCard *getPeerNetworkCardOrNull(NetworkCard *networkCard);
};


#endif  // BGPSIMULATION_ENTITIES_LINK_H
