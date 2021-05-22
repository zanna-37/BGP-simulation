#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <IPv4Layer.h>
#include <TcpLayer.h>

#include <mutex>
#include <stack>
#include <thread>

#include "../entities/Device.h"
#include "TCPEvent.h"
#include "fsm/TCPState.h"
#include "fsm/TCPStateMachine.h"

// forward declarations
#include "../entities/Device.fwd.h"
#include "fsm/TCPState.fwd.h"
#include "fsm/TCPStateMachine.fwd.h"

class TCPConnection {
   public:
    std::string name = "TCPConnection";
    /**
     * The connection state machine. It is based on RFC 793
     */
    TCPStateMachine* stateMachine = nullptr;

    /**
     * The owner of the connection
     */
    Device* owner = nullptr;
    /**
     * The connection source IP address
     */
    pcpp::IPv4Address srcAddr = pcpp::IPv4Address::Zero;
    /**
     * The connection source port
     */
    uint16_t srcPort = 0;
    /**
     * The connection destination IP address
     */
    pcpp::IPv4Address dstAddr = pcpp::IPv4Address::Zero;
    /**
     * The connection source port
     */
    uint16_t dstPort = 0;

    /**
     * Mutex to access the ready variable
     */
    std::mutex ready_mutex;


    /**
     * Mutex to safely access the connected variable
     */
    std::mutex connected_mutex;

    bool running = false;


    // TCP receiving queue

    /**
     * The queue of received messages
     */
    std::queue<std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>>
                                          receivingQueue;
    std::mutex                            receivingQueue_mutex;
    std::condition_variable               receivingQueue_wakeup;
    std::thread*                          receivingThread = nullptr;

    // TCP sending queue
    /**
     * The queue of messsages to be sent
     */
    std::queue<std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>>
                                          sendingQueue;
    std::mutex                            sendingQueue_mutex;
    std::condition_variable               sendingQueue_wakeup;
    std::thread*                          sendingThread = nullptr;

    // Application receiving queue
    /**
     * The application packets queue. It is filled through the socket
     */
    std::queue<std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>>
                            appReceivingQueue;
    std::mutex              appReceivingQueue_mutex;
    std::condition_variable               appReceivingQueue_wakeup;

    // We do not need an application sending queue, since we immediately push
    // the packet into the TCP sending queue

    TCPConnection(Device* owner);
    ~TCPConnection();
    /**
     * Enqueue a TCPEvent in the TCPStateMachine event queue
     * @param event the event to trigger
     */
    void enqueueEvent(TCPEvent);
    /**
     * Take the TCP header as input and return a \a unint8_t indicating what
     * flags are set and what are not
     * @param tcpHeader the TCP header
     * @return a \a uint8_t value indicating what flags are set in the header
     */
    static uint8_t parseTCPFlags(pcpp::tcphdr* tcpHeader);

    /**
     * Process a newly arrived packet at the TCP layer, based on the flags int
     * the TCP header. It enqueues event accordingly, based on TCPStateMachine
     * defined in RFC 793
     */
    void processMessage(
        std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
            receivedLayers);

    /**
     * Return a pointer to the current state of the connection
     * @return pointer to the current state of the connection
     */
    TCPState* getCurrentState();

    /**
     * Start TCP connection threads, used to send and receive messages
     */
    void start();

    /**
     * Start the TCP connection in passive mode and listen for incoming
     * connections
     */
    void listen();

    /**
     * Return the value of the ready variable
     * @return ready variable value
     */
    bool isReady();

    /**
     * Return the value of the connected variable
     * @return connected variable value
     */
    bool isConnected();

    /**
     * Set the connected value
     * @param value the new value of connected attribute
     */
    void setConnected(bool value);


    /**
     * Accept the incoming TCP connection request from the remote peer
     */
    void accept();

    /**
     * Connect to a remote TCP server
     * @param dstAddr the destination address
     * @param dstPort the destination port
     */
    void connect(const pcpp::IPv4Address& dstAddr, uint16_t dstPort);

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
     * push the packet to the sending queue
     * @param layers the stack abstraction of the packet layers
     */
    void sendPacket(
        std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers);

    /**
     * Push the TCP packet to the receiving queue
     * @param layers the TCP packet abstraction layers
     */
    void receivePacket(
        std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers);

    void closeConnection();

    /**
     * Blocking function that waits for the receiving application queue to
     * receive data
     * @return the application layers
     */
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
    waitForApplicationData();

    /**
     * Enquque the application layers to the application layers queue
     * @param layers the application layers
     */
    void enqueueApplicationLayers(
        std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers);

    /**
     * craft the TCP header and send the packet (by pushing it to the
     * sendingQueue)
     * @param layers the stack abstraction of the packet with application data
     * and TCP header
     */
    void sendApplicationData(
        std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers);

   private:
    /**
     * When a TCP connection is pending this value is set to true
     */
    bool ready = false;

    /**
     * When the TCP connection is enstablished, this value is set to true
     */
    bool connected = false;
};

#endif