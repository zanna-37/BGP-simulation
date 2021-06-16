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
        L_ERROR(
            owner->ID,
            "BGPConnection has not been shutdown before deletion.\nCall "
            "shutdown() before deleting the BGPConnection.\nCurrent state is " +
                stateMachine->getCurrentState()->name);
        shutdown();
    }

    //    if (connectedSocket != nullptr || connectThread != nullptr) {
    //        L_ERROR(
    //            owner->ID,
    //            "BGPConnection has not dropped connections before
    //            deletion.\nCall " "dropConnection() before deleting the
    //            BGPConnection.\nCurrent state is " +
    //            stateMachine->getCurrentState()->name);
    //        dropConnection();
    //    }

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
                    //                    bgpOpenLayer_weak =
                    //                        dynamic_cast<BGPOpenLayer*>(bgpLayer_weak);
                    //                    holdTime =
                    //                    std::chrono::seconds(be16toh(
                    //                        bgpOpenLayer_weak->getOpenHeaderOrNull()->holdTime_be));
                    //                    bgpApplication->collisionDetection(this);
                    //                    enqueueEvent(BGPEvent::BGPOpen);
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
                getConnectedSocket()->recv();

            if (running) {
                if (layers != nullptr) {
                    processMessage(std::move(layers));
                } else {
                    // if layers == nullptr it means that the TCP state machine
                    // is not running anymore so we should shutdown this
                    // connection. Ideally this should not happen because the
                    // TCP connection should notify the Socket about the
                    // shutdown and the Socket should notify the BGP connection.
                    // TODO this notify mechanism is not yet implemented at
                    // 2021-06-15.

                    shutdown();
                }
            }
        }
    });
}

void BGPConnection::closeConnection() {
    enqueueEvent(BGPEvent::TcpConnectionFails);
}

void BGPConnection::sendData(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers) {
    getConnectedSocket()->send(std::move(layers));
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
                    // Bind connected Socket to this (or a new) BGP Connection
                    BGPConnection* bgpConnection_weak =
                        setConnectedSocket(newArrivedSocket);

                    if (bgpConnection_weak != this) {
                        // If it is a new BGPConnection set it up
                        bgpConnection_weak->enqueueEvent(
                            BGPEvent::ManualStart_with_PassiveTcpEstablishment);
                    }
                    bgpConnection_weak->enqueueEvent(
                        BGPEvent::TcpConnectionConfirmed);
                    bgpConnection_weak->startReceivingThread();
                }
            } else {
                L_DEBUG(owner->ID, "accept() failed");
            }
        }
    });
}

void BGPConnection::asyncConnectToPeer() {
    // TODO this is probably an async task

    BGPConnection* bgpConnection_weak = setConnectedSocket(new Socket(owner));

    assert(bgpConnection_weak->getConnectedSocket() != nullptr);
    assert(bgpConnection_weak->connectThread == nullptr);

    bgpConnection_weak->connectThread = new thread([&, bgpConnection_weak] {
        if (bgpConnection_weak->getConnectedSocket()->connect(
                bgpConnection_weak->dstAddr, BGPApplication::BGPDefaultPort) ==
            0) {
            bgpConnection_weak->startReceivingThread();
            bgpConnection_weak->enqueueEvent(BGPEvent::TcpConnection_Valid);
        }
    });
}

void BGPConnection::dropConnection() {
    running = false;

    if (getConnectedSocket()) {
        getConnectedSocket()->close();
    }
    delete getConnectedSocket();
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    connectedSocket = nullptr;
#pragma clang diagnostic pop

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

BGPConnection* BGPConnection::setConnectedSocket(Socket* newConnectedSocket) {
    BGPConnection*          filledBGPConnection;
    std::unique_lock<mutex> connectedSocket_uniqueLock(connectedSocket_mutex);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if (this->connectedSocket == nullptr) {
        // This BGP Connection is empty, bind connected Socket to this
        this->connectedSocket = newConnectedSocket;
        connectedSocket_uniqueLock.unlock();
        filledBGPConnection = this;
    } else {
        // This BGP Connection is full, bind connected Socket to
        // a new BgpConnection
        auto* newBGPConnection_weak = bgpApplication->createNewBgpConnection();
        newBGPConnection_weak->connectedSocket = newConnectedSocket;
        connectedSocket_uniqueLock.unlock();

        newBGPConnection_weak->srcAddr =
            newConnectedSocket->tcpConnection->srcAddr;
        newBGPConnection_weak->dstAddr =
            newConnectedSocket->tcpConnection->dstAddr;

        filledBGPConnection = newBGPConnection_weak;
    }
#pragma clang diagnostic pop

    return filledBGPConnection;
}

Socket* BGPConnection::getConnectedSocket() {
    Socket* result;
    connectedSocket_mutex.lock();
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    result = this->connectedSocket;
#pragma clang diagnostic pop
    connectedSocket_mutex.unlock();

    return result;
}
