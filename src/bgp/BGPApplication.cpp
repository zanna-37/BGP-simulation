#include "BGPApplication.h"

#include <assert.h>

#include <memory>
#include <string>

#include "../entities/NetworkCard.h"
#include "../entities/Router.h"
#include "../logger/Logger.h"
#include "../socket/Socket.h"
#include "../tcp/TCPConnection.h"
#include "./packets/BGPUpdatePathAttribute.h"
#include "BGPConnection.h"
#include "BGPEvent.h"
#include "packets/BGPNotificationLayer.h"

BGPApplication::BGPApplication(Router* router, pcpp::IPv4Address BGPIdentifier)
    : router(router), BGPIdentifier(BGPIdentifier) {}

BGPApplication::~BGPApplication() {
    for (BGPConnection* connection : bgpConnections) {
        connection->shutdown();
    }
    for (BGPConnection* connection : bgpConnections) {
        delete connection;
    }
    // listeningSocketModules should be already empty, this is a garbage
    // collector
    for (auto* listeningSocketModule : listeningSocketModules) {
        delete listeningSocketModule;
    }
}

void BGPApplication::passiveOpenAll() {
    // Foreach BGP peer in the BGP Routing table
    for (pcpp::IPv4Address peerAddr : router->peer_addresses) {
        NetworkCard* egressNetCard =
            router->getNextHopNetworkCardOrNull(peerAddr);

        if (egressNetCard == nullptr) {
            L_ERROR(router->ID + " BGP App",
                    peerAddr.toString() + ": Destination unreachable");
        } else {
            pcpp::IPv4Address srcAddr = egressNetCard->IP;

            auto* bgpConnection = createNewBgpConnection(srcAddr, peerAddr);

            BGPEvent event = {
                BGPEventType::ManualStart_with_PassiveTcpEstablishment,
                nullptr,
            };

            bgpConnection->enqueueEvent(std::move(event));
        }
    }
}

void BGPApplication::collisionDetection(BGPConnection*    connectionToCheck,
                                        pcpp::IPv4Address bgpIdentifier) {
    L_DEBUG(connectionToCheck->owner->ID, "Collision detection");
    for (BGPConnection* connection : bgpConnections) {
        if (connection->getCurrentStateName() == "OPEN_CONFIRM") {
            if (connectionToCheck->dstAddr == connection->dstAddr &&
                connection != connectionToCheck) {
                L_INFO(
                    connection->owner->ID,
                    "Found Collision between " + bgpIdentifier.toString() +
                        " and " +
                        connection->bgpApplication->BGPIdentifier.toString());

                BGPEvent event = {BGPEventType::OpenCollisionDump, nullptr};
                if (bgpIdentifier < connection->bgpApplication->BGPIdentifier) {
                    connectionToCheck->enqueueEvent(std::move(event));
                } else {
                    connection->enqueueEvent(std::move(event));
                }
            }
        }
    }
}

BGPConnection* BGPApplication::createNewBgpConnection(
    pcpp::IPv4Address srcAddress, pcpp::IPv4Address dstAddress) {
    auto* newBGPConnection = new BGPConnection(
        router, this, srcAddress, dstAddress, BGPApplication::BGPDefaultPort);
    bgpConnections.push_back(newBGPConnection);
    return newBGPConnection;
}

ListeningSocketModule*
BGPApplication::getOrCreateCorrespondingListeningSocketModule(
    pcpp::IPv4Address srcAddress, uint16_t srcPort) {
    ListeningSocketModule* result = nullptr;
    // Search existing listening socket
    for (auto* listeningSocketModule : listeningSocketModules) {
        Socket* listeningSocket = listeningSocketModule->getSocket();
        if (listeningSocket->tcpConnection->srcAddr == srcAddress &&
            listeningSocket->tcpConnection->srcPort == srcPort) {
            result = listeningSocketModule;
            L_DEBUG(router->ID + " BGP App",
                    "Reusing listening Socket at " + srcAddress.toString() +
                        ":" + std::to_string(srcPort));
        }
    }
    if (result == nullptr) {
        // Create a new listening socket if it doesn't already exist
        L_DEBUG(router->ID + " BGP App",
                "Creating new listening Socket listen at " +
                    srcAddress.toString() + ":" + std::to_string(srcPort));
        auto* newSocketListenModule =
            new ListeningSocketModule(new Socket(router), this);
        listeningSocketModules.push_back(newSocketListenModule);
        newSocketListenModule->getSocket()->bind(
            srcAddress, BGPApplication::BGPDefaultPort);
        result = newSocketListenModule;
    }

    return result;
}

void BGPApplication::sendBGPUpdateMessage(
    BGPConnection*                 bgpConnectionToAvoid,
    std::vector<LengthAndIpPrefix> withdrawnroutes,
    std::vector<uint16_t>          asPath,
    std::vector<LengthAndIpPrefix> nlri,
    bool                           hasPathAttributes) {
    for (BGPConnection* bgpConnection : bgpConnections) {
        if (bgpConnection != bgpConnectionToAvoid) {
            std::vector<PathAttribute> newPathAttributes;

            if (hasPathAttributes) {
                // NextHop PathAttribute
                uint32_t     routerIp_int      = bgpConnection->srcAddr.toInt();
                const size_t nextHopDataLength = 4;
                uint8_t      nextHopData[nextHopDataLength] = {
                    (uint8_t)routerIp_int,
                    (uint8_t)(routerIp_int >> 8),
                    (uint8_t)(routerIp_int >> 16),
                    (uint8_t)(routerIp_int >> 24)};
                PathAttribute nextHopAttribute;
                nextHopAttribute.setAttributeLengthAndValue(nextHopData,
                                                            nextHopDataLength);
                nextHopAttribute.attributeTypeCode =
                    PathAttribute::AttributeTypeCode_uint8_t::NEXT_HOP;
                nextHopAttribute.setFlags(
                    PathAttribute::AttributeTypeFlags_uint8_t::OPTIONAL, 0);
                nextHopAttribute.setFlags(
                    PathAttribute::AttributeTypeFlags_uint8_t::TRANSITIVE, 1);
                nextHopAttribute.setFlags(
                    PathAttribute::AttributeTypeFlags_uint8_t::PARTIAL, 0);
                newPathAttributes.push_back(nextHopAttribute);

                // AS_Path PathAttribute
                std::vector<uint8_t> asPath_be8;
                uint16_t new_as_num = (uint16_t)bgpConnection->owner->AS_number;
                asPath.push_back(new_as_num);

                uint8_t asPathType = 2;  // TODO: Check that this is AS_SEQUENCE
                uint8_t asPathLen  = asPath.size();

                PathAttribute::asPathToAttributeDataArray_be(
                    asPathType, asPathLen, asPath, asPath_be8);

                size_t        asPathDataLength = asPath_be8.size();
                uint8_t*      asPathData       = asPath_be8.data();
                PathAttribute asPathAttribute;
                asPathAttribute.setAttributeLengthAndValue(asPathData,
                                                           asPathDataLength);
                asPathAttribute.attributeTypeCode =
                    PathAttribute::AttributeTypeCode_uint8_t::AS_PATH;
                asPathAttribute.setFlags(
                    PathAttribute::AttributeTypeFlags_uint8_t::OPTIONAL, 0);
                asPathAttribute.setFlags(
                    PathAttribute::AttributeTypeFlags_uint8_t::TRANSITIVE, 1);
                asPathAttribute.setFlags(
                    PathAttribute::AttributeTypeFlags_uint8_t::PARTIAL, 0);
                newPathAttributes.push_back(asPathAttribute);

                // Origin PathAttribute
                const size_t  originDataLength             = 1;
                uint8_t       originData[originDataLength] = {2};
                PathAttribute originPathAttribute;
                originPathAttribute.setAttributeLengthAndValue(
                    originData, originDataLength);
                originPathAttribute.attributeTypeCode =
                    PathAttribute::AttributeTypeCode_uint8_t::ORIGIN;
                originPathAttribute.setFlags(
                    PathAttribute::AttributeTypeFlags_uint8_t::OPTIONAL, 0);
                originPathAttribute.setFlags(
                    PathAttribute::AttributeTypeFlags_uint8_t::TRANSITIVE, 1);
                originPathAttribute.setFlags(
                    PathAttribute::AttributeTypeFlags_uint8_t::PARTIAL, 0);
                newPathAttributes.push_back(originPathAttribute);

                // Send BGPUpdateMessage
                std::unique_ptr<BGPUpdateLayer> bgpUpdateLayer =
                    std::make_unique<BGPUpdateLayer>(
                        withdrawnroutes, newPathAttributes, nlri);
                bgpUpdateLayer->computeCalculateFields();
                L_DEBUG(bgpConnection->owner->ID + " BGPfsm",
                        bgpUpdateLayer->toString());
                /*std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                    layers =
                        make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();
                layers->push(std::move(bgpUpdateLayer));*/

                // Enqueue new BGPUpdateMessage
                BGPEvent event = {BGPEventType::SendUpdateMsg,
                                  std::move(bgpUpdateLayer)};
                bgpConnection->enqueueEvent(std::move(event));

                L_INFO(bgpConnection->owner->ID + " BGPfsm",
                       "Enqueuing UPDATE message in the events");

                /*bgpConnection->sendData(std::move(layers));

                L_INFO(bgpConnection->owner->ID + " BGPfsm",
                       "Sending UPDATE message");*/
            } else {
                // Send BGPUpdateMessage
                std::unique_ptr<BGPUpdateLayer> bgpUpdateLayer =
                    std::make_unique<BGPUpdateLayer>(
                        withdrawnroutes, newPathAttributes, nlri);
                bgpUpdateLayer->computeCalculateFields();
                L_DEBUG(bgpConnection->owner->ID + " BGPfsm",
                        bgpUpdateLayer->toString());

                BGPEvent event = {BGPEventType::SendUpdateMsg,
                                  std::move(bgpUpdateLayer)};
                bgpConnection->enqueueEvent(std::move(event));

                L_INFO(bgpConnection->owner->ID + " BGPfsm",
                       "Enqueuing UPDATE message in the events");
            }
        }
    }
}

void BGPApplication::startListeningOnSocket(pcpp::IPv4Address srcAddress) {
    std::unique_lock<std::mutex> listeningSockets_uniqueLock(
        listeningSockets_mutex);
    ListeningSocketModule* listeningSocketModule_weak =
        getOrCreateCorrespondingListeningSocketModule(srcAddress,
                                                      BGPDefaultPort);

    // socketListen_weak can't be null except for bugs
    assert(listeningSocketModule_weak);

    listeningSocketModule_weak->startListeningThread(srcAddress);

    listeningSockets_uniqueLock.unlock();
}

void BGPApplication::stopListeningOnSocket(pcpp::IPv4Address srcAddress) {
    std::unique_lock<std::mutex> listeningSockets_uniqueLock(
        listeningSockets_mutex);
    ListeningSocketModule* listeningSocketModule_weak =
        getOrCreateCorrespondingListeningSocketModule(srcAddress,
                                                      BGPDefaultPort);

    // socketListen_weak can't be null except for bugs
    assert(listeningSocketModule_weak);

    listeningSocketModule_weak->stopListeningThread();
    listeningSocketModule_weak->getSocket()->close();

    // remove element
    listeningSocketModules.erase(std::remove(listeningSocketModules.begin(),
                                             listeningSocketModules.end(),
                                             listeningSocketModule_weak),
                                 listeningSocketModules.end());
    delete listeningSocketModule_weak;
    listeningSockets_uniqueLock.unlock();
}

BGPConnection* BGPApplication::setConnectedSocketToAvailableBGPConn(
    Socket*           newConnectedSocket,
    pcpp::IPv4Address bgpConnectionSrcAddressToBindTo,
    pcpp::IPv4Address bgpConnectionDstAddressToBindTo) {
    // Try to bind the new connected Socket to an existing BGP Connection
    BGPConnection* bgpConnectionFilled = nullptr;
    for (auto bgpConnection_it = bgpConnections.begin();
         bgpConnectionFilled == nullptr &&
         bgpConnection_it != bgpConnections.end();
         bgpConnection_it++) {
        if ((*bgpConnection_it)->srcAddr == bgpConnectionSrcAddressToBindTo &&
            (*bgpConnection_it)->dstAddr == bgpConnectionDstAddressToBindTo) {
            // We try to fill this connection...
            if ((*bgpConnection_it)
                    ->setConnectedSocketIfFree(newConnectedSocket)) {
                bgpConnectionFilled = *bgpConnection_it;
            }
        }
    }
    // ... otherwise we create a new one
    if (bgpConnectionFilled == nullptr) {
        bgpConnectionFilled = createNewBgpConnection(
            bgpConnectionSrcAddressToBindTo, bgpConnectionDstAddressToBindTo);
        assert(
            bgpConnectionFilled->setConnectedSocketIfFree(newConnectedSocket));

        // If it is a new BGPConnection we align it to other existing
        // connections
        BGPEvent event = {
            BGPEventType::ManualStart_with_PassiveTcpEstablishment,
            nullptr,
        };
        bgpConnectionFilled->enqueueEvent(std::move(event));
    }

    BGPEvent event = {
        BGPEventType::TcpConnectionConfirmed,
        nullptr,
    };
    bgpConnectionFilled->enqueueEvent(std::move(event));

    return bgpConnectionFilled;
}

ListeningSocketModule::ListeningSocketModule(Socket*         socket,
                                             BGPApplication* BGPAppParent)
    : listeningSocket(socket),
      listeningSocketsThread(nullptr),
      bgpApp(BGPAppParent) {}

ListeningSocketModule::~ListeningSocketModule() {
    if (running) {
        L_ERROR(
            listeningSocket->device->ID + " BGPApp",
            "Listening thread has not been stopped!\nThe BGPConnection should "
            "call stopListeningOnSocket(...) before the application is "
            "shutdown");
        stopListeningThread();
    }
    if (listeningSocket->running) {
        L_ERROR(
            listeningSocket->device->ID + " BGPApp",
            "Listening socket has not been closed!\nThe BGPConnection should "
            "call stopListeningOnSocket(...) before the application is "
            "shutdown");
        listeningSocket->close();
    }
    if (listeningSocketsThread) {
        listeningSocketsThread->join();
    }
    delete listeningSocket;
    delete listeningSocketsThread;
}

Socket* ListeningSocketModule::getSocket() { return listeningSocket; }

void ListeningSocketModule::startListeningThread(pcpp::IPv4Address srcAddress) {
    assert(listeningSocket);
    assert(listeningSocketsThread == nullptr || running == true);

    if (!running) {
        running                = true;
        listeningSocketsThread = new thread([&, srcAddress]() {
            listeningSocket->listen();

            while (running) {
                // wait for the socket to have TCP bgpConnection pending
                Socket* newArrivedSocket = listeningSocket->accept();

                if (newArrivedSocket) {
                    if (running) {
                        // Check that the arrived socket, created by the
                        // accept(), was really meant for srcAddress
                        assert(srcAddress ==
                               newArrivedSocket->tcpConnection->srcAddr);
                        BGPConnection* bgpConnectionFilled =
                            bgpApp->setConnectedSocketToAvailableBGPConn(
                                newArrivedSocket,
                                newArrivedSocket->tcpConnection->srcAddr,
                                newArrivedSocket->tcpConnection->dstAddr);
                        bgpConnectionFilled->startReceivingThread();
                    } else {
                        L_DEBUG(listeningSocket->device->ID,
                                "connection is dropped, discarding wrongly "
                                "accepted "
                                "socket");
                        newArrivedSocket->close();
                        delete newArrivedSocket;
                    }
                }
            }
        });
    }
}

void ListeningSocketModule::stopListeningThread() { running = false; }
