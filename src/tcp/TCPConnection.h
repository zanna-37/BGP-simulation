#ifndef BGPSIMULATION_TCP_TCPCONNECTION_H
#define BGPSIMULATION_TCP_TCPCONNECTION_H

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>
#include <stack>
#include <string>
#include <utility>

#include "IpAddress.h"
#include "Layer.h"
#include "TcpLayer.h"

// forward declarations
#include "../entities/Device.fwd.h"
#include "fsm/TCPState.fwd.h"
#include "fsm/TCPStateMachine.fwd.h"


class TCPConnection {
   public:
    std::string      name         = "TCPConnection";
    /**
     * The connection state machine. It is based on RFC 793
     */
    TCPStateMachine* stateMachine = nullptr;

    /**
     * The owner of the connection
     */
    Device*           owner   = nullptr;
    /**
     * The connection source IP address
     */
    pcpp::IPv4Address srcAddr = pcpp::IPv4Address::Zero;
    /**
     * The connection source port
     */
    uint16_t          srcPort = 0;
    /**
     * The connection destination IP address
     */
    pcpp::IPv4Address dstAddr = pcpp::IPv4Address::Zero;
    /**
     * The connection source port
     */
    uint16_t          dstPort = 0;

    TCPConnection(Device* owner);
    ~TCPConnection();

    /**
     * Take the TCP header as input and return a \a unint8_t indicating what
     * flags are set and what are not
     * @param tcpHeader the TCP header
     * @return a \a uint8_t value indicating what flags are set in the header
     */
    static uint8_t parseTCPFlags(pcpp::tcphdr* tcpHeader);

    /**
     * Process a newly arrived packet at the TCP layer, based on the flags in
     * the TCP header. It enqueues event accordingly, based on TCPStateMachine
     * defined in RFC 793.
     */
    void segmentArrives(
        std::pair<pcpp::IPv4Address,
                  std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>>
            receivedLayers);

    /**
     * Start the TCP connection in passive mode and listen for incoming
     * connections
     */
    int listen();

    /**
     * Accept the incoming TCP connection request from the remote peer
     */
    std::shared_ptr<TCPConnection> accept();

    /**
     * Connect to a remote TCP server
     * @param dstAddr the destination address
     * @param dstPort the destination port
     */
    int connect();

    void close();

    void abort();

    /**
     * Blocking function that waits for the receiving application queue to
     * receive data.
     * @param layers The application layers that are filled with the received
     * data, ready to be passed to the application. Always pass an empty object.
     * In case of errors the content of this variable is not defined.
     *
     * @return 0 if successful or -1 in case of errors.
     */
    [[nodiscard]] int waitForApplicationData(
        std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>& layers);

    /**
     * Enquque the application layers to the application layers queue
     * @param layers the application layers
     */
    void enqueuePacketToInbox(
        std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers);

    /**
     * craft the TCP header and send the packet (by pushing it to the
     * sendingQueue)
     * @param layers the stack abstraction of the packet with application data
     * and TCP header
     */
    void send(std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers);


    std::string toString();

   private:
    std::atomic<bool> running = {false};

    // TCP receiving queue
    /**
     * The queue of received transport-level messages.
     */
    std::queue<
        std::pair<pcpp::IPv4Address,
                  std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>>>
               receivingQueue;
    std::mutex receivingQueue_mutex;

    // TCP sending queue
    /**
     * The queue of transport-level messages to be sent.
     */
    std::queue<std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>>
                            sendingQueue;
    std::mutex              sendingQueue_mutex;
    std::condition_variable sendingQueue_wakeup;

    /**
     * The queue of received application-level messages. This queue is read by
     * the application.
     */
    std::queue<std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>>
                            appReceivingQueue;
    std::mutex              appReceivingQueue_mutex;
    std::condition_variable appReceivingQueue_wakeup;

    /**
     * The queue of application-level messages to be sent. This queue is be
     * filled by the application.
     */
    std::queue<std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>>
               appSendingQueue;
    std::mutex appSendingQueue_mutex;

    // Pending connection queue
    std::queue<std::shared_ptr<TCPConnection>> pendingConnections;
    std::mutex                                 pendingConnections_mutex;
    std::condition_variable                    pendingConnections_wakeup;


    bool                    tryingToEstablish;
    std::mutex              tryingToEstablish_mutex;
    std::condition_variable tryingToEstablish_wakeup;

    /**
     * Craft a TCP layer with the header flags specified.
     * @warning it should be called only inside the onEvent method
     * @param srcPort the source port
     * @param dstPort the destination port
     * @param flags an integer indicating the TCP flag (bit notation, according
     * to RFC)
     * @return a pointer to the newly created TCP layer
     */
    static std::unique_ptr<pcpp::TcpLayer> craftTCPLayer(uint16_t srcPort,
                                                         uint16_t dstPort,
                                                         int      flags);

    /**
     * Fork the listening connection into a new one.
     *
     * In this way the old one continue listening while the new can
     * evolve and connect.
     *
     * @warning The caller (so not this function) need to push the new
     * connection into the pendingConnection queue and notify the accept()
     * method.
     *
     * @param remoteSrcAddr The remote address the SYN come from.
     * @param remoteSrcPort The remote port the SYN come from.
     * @return A new connection that is technically put in the listening state,
     * but is ready to evolve.
     */
    std::shared_ptr<TCPConnection> createConnectedConnectionFromListening(
        const pcpp::IPv4Address& remoteSrcAddr, const uint16_t remoteSrcPort);

    /**
     * Push the packet to the sending queue to be hand over to the lower stack
     * layers.
     * @warning Low-level call, do not use to send application packets directly.
     * Call \a TCPConnection::send instead.
     * @param layers The stack of the packet layers to send.
     */
    void enqueuePacketToOutbox(
        std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers,
        pcpp::IPv4Address&                                        dstAddr1);

    void enqueuePacketToPeerOutbox(
        std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers);

    void sendSynToPeer();

    void sendFinToPeer();

    void sendAckToPeer();

    void sendSynAckToPeer();

    void sendRstToPeer();

    void sendRstTo(pcpp::IPv4Address dstAddr, uint16_t dstPort);

    /**
     * TODO
     * @warning This should only be called after having received a \a
     * TCPEvent::SegmentArrives event.
     * @return
     */
    std::pair<pcpp::IPv4Address,
              std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>>
    getNextSegment();

    friend class TCPStateClosed;
    friend class TCPStateCloseWait;
    friend class TCPStateClosing;
    friend class TCPStateEstablished;
    friend class TCPStateFINWait1;
    friend class TCPStateFINWait2;
    friend class TCPStateLastACK;
    friend class TCPStateListen;
    friend class TCPStateSYNReceived;
    friend class TCPStateSYNSent;
    friend class TCPStateTimeWait;
};

#endif  // BGPSIMULATION_TCP_TCPCONNECTION_H
