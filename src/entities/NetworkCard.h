#ifndef BGPSIMULATION_ENTITIES_NET_DETAILS_H
#define BGPSIMULATION_ENTITIES_NET_DETAILS_H

#include <EthLayer.h>
#include <IPv4Layer.h>
#include <Packet.h>
#include <ProtocolType.h>
#include <assert.h>
#include <time.h>

#include <memory>
#include <queue>
#include <stack>
#include <string>

#include "../utils/MacGenerator.h"
#include "Device.h"
#include "Link.h"

// forward declarations
#include "Device.fwd.h"
#include "Link.fwd.h"
using namespace std;


/**
 * This abstracts the concept of a network card.
 * It can connect and disconnect to and from a \a Link.
 */
class NetworkCard {
   public:
    /**
     * Network interface.
     */
    std::string netInterface;

    /**
     * The IP associated with this network interface.
     */
    pcpp::IPv4Address IP;

    /**
     * The network mask associated with this network interface.
     */
    pcpp::IPv4Address netmask;

    /**
     * The link this network card is attached to, or \a nullptr when
     * disconnected.
     */
    std::shared_ptr<Link> link = nullptr;

    /**
     * The owner of the network card, where the card is installed.
     */
    Device* owner;

    /**
     * The mac address of the network card.
     */
    pcpp::MacAddress mac;

    /**
     * A queue of packets that arrives at the network card.
     */
    queue<unique_ptr<pcpp::Packet>> receivedPacketsQueue;


    /**
     * Create a NetworkCard.
     * @param netInterface The network interface name. For example: \a eth0, \a
     * wlan1, ...
     * @param IP The IP address associated to the networkCard.
     * @param netmask The netmask address associated to the networkCard.
     * @param mac The MAC address associated to the networkCard. If it is equal
     * to \a pcpp::MacAddress::Zero if will be randomly generated.
     * @param owner The device where the card is installed.
     */
    NetworkCard(std::string       netInterface,
                pcpp::IPv4Address IP,
                pcpp::IPv4Address netmask,
                pcpp::MacAddress  mac,
                Device*           owner);

    ~NetworkCard();

    /**
     * Connect the networkCard to the specified link.
     *
     * It does nothing if the networkCard is already connected to a link.
     *
     * @warning If the link is already fully associated with other network cards
     * the behavior is undefined.
     * @param linkToConnect The link to connect to.
     */
    void connect(const std::shared_ptr<Link>& linkToConnect);

    /**
     * Disconnect the networkCard from the specified link.
     *
     * It does nothing if the networkCard is not connected to the specified link
     * or no link at all.
     * @param linkToConnect The link to disconnect from.
     */
    void disconnect(const std::shared_ptr<Link>& linkToDisconnect);

    /**
     * Send the packet packet to the lower layer after instantiating the MAC
     * layer. Here the layers are used to craft the packet
     * @warning This should be called by the \a Device.
     * @param layers The \a std::stack simulating the packet.
     */
    void sendPacket(stack<pcpp::Layer*>* layers);

    /**
     * Called when the device event queue is ready to handle a packet from this
     * network card. It takes the first packet in the queue and start creating
     * the layers for upper protocols.
     * @warning It should be called by the device when the message is handled
     */
    void handleNextPacket();

   private:
    /**
     * The packet is arrived from the link and it is enqueued in the
     * packetQueue.
     *
     * It copies the content of the received packet and notify the upper layers
     * that it is ready for processing.
     *
     * @warning it should be called by the link
     * @param receivedDataStream The pair indicating the length and the array of
     * bytes.
     */
    void receivePacketFromWire(
        std::pair<const uint8_t*, const int> receivedDataStream);

    /**
     * Get the read-only array-of-bytes representation of the parsed packet.
     * @param packet The packet to be serialized.
     * @return A pair with the data and the data length.
     */
    static std::pair<const uint8_t*, const int> serialize(
        const pcpp::Packet* packet);

    /**
     * Put the stream of bytes in a parsed packet.
     *
     * The newly created packet will be in charge of deallocating the rawData on
     * destruction.
     *
     * @param rawData The raw data to wrap.
     * @param rawDataLen The length of the stream.
     * @return A parsed packet.
     */
    static std::unique_ptr<pcpp::Packet> deserialize(uint8_t*  rawData,
                                                     const int rawDataLen);

    friend class Link;
};

#endif  // BGPSIMULATION_ENTITIES_NET_DETAILS_H
