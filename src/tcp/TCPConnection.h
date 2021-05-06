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
    // TODO remove, we do not need it
    uint16_t BGPPort = 179;

    bool         ready = false;
    std::mutex   ready_mutex;
    std::thread* listeningThread = nullptr;

    bool       connected = false;
    std::mutex connected_mutex;

    bool running = false;


    // TCP receiving queue
    std::queue<std::stack<pcpp::Layer*>*> receivingQueue;
    std::mutex                            receivingQueue_mutex;
    std::condition_variable               receivingQueue_wakeup;
    std::thread*                          receivingThread = nullptr;

    // TCP sending queue
    std::queue<std::stack<pcpp::Layer*>*> sendingQueue;
    std::mutex                            sendingQueue_mutex;
    std::condition_variable               sendingQueue_wakeup;
    std::thread*                          sendingThread = nullptr;

    // Application receiving queue
    std::queue<std::stack<pcpp::Layer*>*> appReceivingQueue;
    std::mutex                            appReceivingQueue_mutex;
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
    uint8_t parseTCPFlags(pcpp::tcphdr* tcpHeader);

    /**
     * Process a newly arrived packet at the TCP layer, based on the flags int
     * the TCP header. It enqueues event accordingly, based on TCPStateMachine
     * defined in RFC 793
     */
    void processMessage(std::stack<pcpp::Layer*>* layers);

    /**
     * Return a pointer to the current state of the connection
     * @return pointer to the current state of the connection
     */
    TCPState* getCurrentState();


    void sendPacket(std::stack<pcpp::Layer*>* layers);

    void start();

    void listen();

    bool isReady();

    bool isConnected();

    void setConnected(bool value);


    void accept();

    void connect(const pcpp::IPv4Address& dstAddr, uint16_t dstPort);

    pcpp::TcpLayer* craftTCPLayer(uint16_t srcPort,
                                  uint16_t dstPort,
                                  int      flags);

    void processFlags(uint8_t                   flags,
                      std::stack<pcpp::Layer*>* applicationLayers);

    void sendPacket(std::stack<pcpp::Layer*>* layers);

    void receivePacket(std::stack<pcpp::Layer*>* layers);

    void closeConnection();

    std::stack<pcpp::Layer*>* waitForApplicationData();
    void enqueueApplicationLayers(std::stack<pcpp::Layer*>* applicationLayers);
    void sendApplicationData(std::stack<pcpp::Layer*>* layers);

   private:
    void stopThread();
};

#endif