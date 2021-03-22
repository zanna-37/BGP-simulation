#ifndef BGPSIMULATION_ENTITIES_LINK_H
#define BGPSIMULATION_ENTITIES_LINK_H

#include "NetworkCard.h"


using namespace std;

enum Connection_status { active, failed };

class NetworkCard;  // forward declaration
class Link {
   public:
    Connection_status connection_status;

    Link(Connection_status connection_status)
        : connection_status(connection_status) {}

    NetworkCard *getPeerNetworkCardOrNull(NetworkCard *networkCard);

   private:
    pair<NetworkCard *, NetworkCard *> device_source_networkCards;

    void disconnect(NetworkCard *networkCard);

    void connect(NetworkCard *networkCard);

    friend class NetworkCard;
};


#endif  // BGPSIMULATION_ENTITIES_LINK_H
