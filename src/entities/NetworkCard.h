#ifndef BGPSIMULATION_ENTITIES_NET_DETAILS_H
#define BGPSIMULATION_ENTITIES_NET_DETAILS_H

#include <assert.h>
#include <time.h>

#include <atomic>
#include <memory>
#include <queue>
#include <stack>
#include <string>

#include "../utils/MacGenerator.h"
#include "Device.h"
#include "EthLayer.h"
#include "IPv4Layer.h"
#include "Link.h"
#include "Packet.h"
#include "ProtocolType.h"

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
     * Mutex to lock the queue.
     */
    std::mutex receivedPacketsQueue_mutex;
    /**
     * condition variable set when the queue is not empty.
     */
    std::condition_variable receivedPacketsQueue_wakeup;

    /**
     * A queue of packets ready to be consumed by upper layers.
     */
    std::queue<std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>>
        receivedPacketsQueue;

    std::atomic<bool> running = {true};


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
    void sendPacket(
        std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers);

    /**
     * Synchronously wait for a new packet to arrive.
     *
     * It waits for a ISO-OSI level 3 packet (Network layer). It therefore
     * contains all the layers but the ones below the network.
     * Note: to forcefully stop the waiting, call \a NetworkCard::shutdown()
     *
     * @return A new packet arrived, or null in case the network card is
     * shutting down.
     */
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> waitForL3Packet();

    /**
     * Force the exit from \a NetworkCard::waitForL3Packet() even if no packets
     * are present in the queue.
     */
    void shutdown();

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
