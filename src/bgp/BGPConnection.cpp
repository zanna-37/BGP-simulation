#include "BGPConnection.h"

#include <endian.h>

#include <cassert>
#include <utility>

#include "../entities/Router.h"
#include "../logger/Logger.h"
#include "../socket/Socket.h"
#include "../tcp/TCPConnection.h"
#include "BGPApplication.h"
#include "Layer.h"
#include "fsm/BGPState.h"
#include "fsm/BGPStateMachine.h"
#include "packets/BGPLayer.h"
#include "packets/BGPOpenLayer.h"


BGPConnection::BGPConnection(Router* owner, BGPApplication* bgpApplication)
    : owner(owner), bgpApplication(bgpApplication) {
    running      = true;
    stateMachine = new BGPStateMachine(this);
    // start() cannot be called in the constructor because we need a fully
    // constructed object
    stateMachine->start();
}

BGPConnection::~BGPConnection() {
    if (running) {
        L_ERROR(owner->ID,
                "BGPConnection has not been shutdown before deletion.\nCall "
                "shutdown() before deleting the BGPConnection.");
        shutdown();
    }

    dropConnection();

    if (listeningThread) {
        listeningThread->join();
    }
    delete listeningThread;

    if (receivingThread) {
        receivingThread->join();
    }
    delete receivingThread;

    delete stateMachine;
}

void BGPConnection::enqueueEvent(BGPEvent event) {
    stateMachine->enqueueEvent(event);
}


void BGPConnection::processMessage(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers) {
    L_DEBUG(owner->ID, "Processing BGP Message");
    while (!layers->empty()) {
        std::unique_ptr<pcpp::Layer> layer = std::move(layers->top());
        layers->pop();
        auto* bgpLayer_weak = dynamic_cast<BGPLayer*>(layer.get());

        BGPLayer::BGPCommonHeader* bgpHeader =
            bgpLayer_weak->getCommonHeaderOrNull();
        if (bgpHeader) {
            BGPOpenLayer* bgpOpenLayer_weak;

            // Call BGPConnection::sendData(...) if we want to send back a
            // packet to our peer

            // TODO write the logic of the handling
            L_FATAL("BGPConnection", "TODO write the logic of the handling");
            switch (bgpHeader->type) {
                case BGPLayer::BGPMessageType::OPEN:
                    L_DEBUG(owner->ID, "OPEN message arrived");
                    bgpOpenLayer_weak =
                        dynamic_cast<BGPOpenLayer*>(bgpLayer_weak);
                    holdTime = std::chrono::seconds(be16toh(
                        bgpOpenLayer_weak->getOpenHeaderOrNull()->holdTime_be));
                    bgpApplication->collisionDetection(this);
                    enqueueEvent(BGPEvent::BGPOpen);
                    break;
                case BGPLayer::BGPMessageType::UPDATE:
                    L_DEBUG(owner->ID, "UPDATE message arrived");
                    break;
                case BGPLayer::BGPMessageType::NOTIFICATION:
                    L_DEBUG(owner->ID, "NOTIFICATION message arrived");
                    break;
                case BGPLayer::BGPMessageType::KEEPALIVE:
                    L_DEBUG(owner->ID, "KEEPALIVE message arrived");
                    break;

                default:
                    break;
            }
        }
    }
}

void BGPConnection::startReceivingThread() {
    assert(receivingThread == nullptr);
    receivingThread = new std::thread([&]() {
        while (running) {
            std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers =
                connectedSocket->recv();

            processMessage(std::move(layers));
        }
    });
}

void BGPConnection::closeConnection() {
    enqueueEvent(BGPEvent::TcpConnectionFails);
}

void BGPConnection::sendData(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers) {
    connectedSocket->send(std::move(layers));
}

void BGPConnection::listenForRemotelyInitiatedConnections() {
    /**
     * @internal Fully managed by the BgpApplication. Do not close nor
     * deallocate.
     */
    Socket* socketListen_weak =
        bgpApplication->getCorrespondingListeningSocket(srcAddr, srcPort);

    // socketListen_weak can't be null except for bugs
    assert(socketListen_weak);

    listeningThread = new thread([&, socketListen_weak]() {
        socketListen_weak->listen();

        while (running) {
            // wait for the socket to have TCP bgpConnection pending
            Socket* newArrivedSocket = socketListen_weak->accept();

            if (running) {
                if (newArrivedSocket) {
                    // Bind connected Socket to this BGP Connection
                    connectedSocket_mutex.lock();
                    if (this->connectedSocket == nullptr) {
                        this->connectedSocket = newArrivedSocket;
                        connectedSocket_mutex.unlock();
                        this->enqueueEvent(BGPEvent::TcpConnectionConfirmed);
                        this->startReceivingThread();
                    } else {
                        // This BGP Connection is full, bind connected Socket to
                        // a new BgpConnection
                        auto* newBGPConnection_weak =
                            bgpApplication->createNewBgpConnection();
                        newBGPConnection_weak->connectedSocket =
                            newArrivedSocket;
                        connectedSocket_mutex.unlock();

                        newBGPConnection_weak->enqueueEvent(
                            BGPEvent::ManualStart_with_PassiveTcpEstablishment);
                        newBGPConnection_weak->srcAddr =
                            newArrivedSocket->tcpConnection->srcAddr;
                        newBGPConnection_weak->dstAddr =
                            newArrivedSocket->tcpConnection->dstAddr;

                        newBGPConnection_weak->enqueueEvent(
                            BGPEvent::TcpConnectionConfirmed);
                        newBGPConnection_weak->startReceivingThread();
                    }
                } else {
                    L_DEBUG(owner->ID, "accept() failed");
                }
            }
        }
    });
}

void BGPConnection::asyncConnectToPeer() {
    // TODO this is probably an async task
    connectedSocket_mutex.lock();
    assert(connectedSocket == nullptr);
    assert(connectThread == nullptr);

    connectedSocket = new Socket(owner);

    connectThread = new thread([&] {
        if (connectedSocket->connect(dstAddr, BGPApplication::BGPDefaultPort) ==
            0) {
            enqueueEvent(BGPEvent::TcpConnection_Valid);
        }
    });
    connectedSocket_mutex.unlock();
}


void BGPConnection::dropConnection() {
    connectedSocket_mutex.lock();
    if (connectedSocket) {
        connectedSocket->close();
    }
    delete connectedSocket;
    connectedSocket = nullptr;
    connectedSocket_mutex.unlock();

    if (connectThread) {
        connectThread->join();
    }
    delete connectThread;
    connectThread = nullptr;
}

void BGPConnection::shutdown() {
    running = false;
    enqueueEvent(BGPEvent::ManualStop);
}
