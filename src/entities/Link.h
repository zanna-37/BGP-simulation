#ifndef BGPSIMULATION_ENTITIES_LINK_H
#define BGPSIMULATION_ENTITIES_LINK_H

#include <Packet.h>
#include <assert.h>
#include <sys/time.h>

#include "NetworkCard.h"


using namespace std;

enum Connection_status {
    /**
     * The link is up and healthy. All the packets pass through it successfully.
     */
    ACTIVE,
    /**
     * The link is broken. No packet can pass though it.
     */
    FAILED
};

class NetworkCard;  // forward declaration

/**
 * This class abstracts the concept of a medium (e.g. a cable) that connects two
 * devices.
 */
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

    void sendPacket(pcpp::Packet *packet, NetworkCard *origin);

    void receivePacket(pair<const uint8_t *, int> data,
                       NetworkCard *              destination);

    pair<const uint8_t *, int> serialize(pcpp::Packet *packet);

    pcpp::Packet *deserialize(const uint8_t *rawData, int rawDataLen);

   private:
    /**
     * Convenient pair that stores the references to the connected devices. No
     * order is assumed.
     *
     * @warning This must always reflect the state of the \a NetworkCards. It is
     * kept in sync by the methods \a Link::connect(...) and \a
     * Link::disconnect(...) which are in turn called from the \a NetworkCard
     * class. For this reason this pair should be used as a convenient reference
     * but the ground truth is always the state inside the \a NetworkCards.
     */
    pair<NetworkCard *, NetworkCard *> device_source_networkCards;

    /**
     * Disassociate the specified networkCard.
     * This reflects the changes performed in \a NeworkCard::disconnect(...). In
     * particular, when a network card disassociate from a link, the link will
     * in turn remove its reference of the (now removed) network card.
     *
     * @warning This is an internal method and should only be called from the \a
     * NetworkCard class. Use \a NeworkCard::disconnect(...) instead.
     * @param networkCard The networkcard that has been disassociated.
     */
    void disconnect(NetworkCard *networkCard);

    /**
     * Associate the specified networkCard.
     * This reflects the changes performed in \a NeworkCard::connect(...). In
     * particular, when a network card associate with a link, the link stores an
     * internal reference of the newly attached network card.
     *
     * @warning This is an internal method and should only be called from the \a
     * NetworkCard class.  Use \a NeworkCard::connect(...) instead.
     * @param networkCard The networkcard that has been associated.
     */
    void connect(NetworkCard *networkCard);

    friend class NetworkCard;
};


#endif  // BGPSIMULATION_ENTITIES_LINK_H
