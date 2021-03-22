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

    /**
     * Get the networkCard of the opposite endpoint in the link.
     *
     * @param networkCard The networkCard whose peer address will be returned.
     * @return The networkCard of the opposite endpoint in the link or \a
     * nullptr if there is not such endpoint connected or if the specified
     * networkCard is not connected to the link.
     */
    NetworkCard *getPeerNetworkCardOrNull(NetworkCard *networkCard);

   private:
    pair<NetworkCard *, NetworkCard *> device_source_networkCards;

    /**
     * Disassociate the specified networkCard.
     *
     * @warning This should only be called from the \a NetworkCard class.
     *
     * @param networkCard The networkcard to disassociate.
     */
    void disconnect(NetworkCard *networkCard);

    /**
     * Associate the specified networkCard.
     *
     * @warning This should only be called from the \a NetworkCard class.
     *
     * @param networkCard The networkcard to associate.
     */
    void connect(NetworkCard *networkCard);

    friend class NetworkCard;
};


#endif  // BGPSIMULATION_ENTITIES_LINK_H
