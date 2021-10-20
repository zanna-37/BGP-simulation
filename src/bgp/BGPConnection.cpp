#include "BGPConnection.h"

#include <endian.h>

#include <cassert>
#include <utility>

#include "../entities/Router.h"
#include "../logger/Logger.h"
#include "../socket/Socket.h"
#include "../tcp/TCPConnection.h"
#include "../utils/SmartPointerUtils.h"
#include "BGPApplication.h"
#include "Layer.h"
#include "fsm/BGPState.h"
#include "fsm/BGPStateMachine.h"
#include "packets/BGPKeepaliveLayer.h"
#include "packets/BGPLayer.h"
#include "packets/BGPNotificationLayer.h"
#include "packets/BGPOpenLayer.h"
#include "packets/BGPUpdateLayer.h"


BGPConnection::BGPConnection(Router*           owner,
                             BGPApplication*   bgpApplication,
                             pcpp::IPv4Address srcAddress,
                             pcpp::IPv4Address dstAddress,
                             uint16_t          dstPort)
    : owner(owner),
      bgpApplication(bgpApplication),
      srcAddr(srcAddress),
      srcPort(0),
      dstAddr(dstAddress),
      dstPort(dstPort) {
    running      = true;
    stateMachine = new BGPStateMachine(this);
    // start() cannot be called in the constructor because we need a fully
    // constructed object
    stateMachine->start();
}

BGPConnection::~BGPConnection() {
    if (running) {
        L_ERROR(owner->ID,
                "BGPConnection has not been closed, dropped, or shutdown "
                "before deletion.\nDo that before deleting the "
                "BGPConnection.\nCurrent state is " +
                    stateMachine->getCurrentState()->name);
        shutdown();
    }

    if (receivingThread) {
        receivingThread->join();
    }
    delete receivingThread;

    delete stateMachine;
}

void BGPConnection::enqueueEvent(BGPEvent event) {
    stateMachine->enqueueEvent(std::move(event));
}


void BGPConnection::processMessage(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers) {
    L_DEBUG(owner->ID, "Processing BGP Message");
    while (!layers->empty()) {
        std::unique_ptr<pcpp::Layer> layer = std::move(layers->top());
        layers->pop();

        std::unique_ptr<BGPLayer> bgpLayer;
        dynamic_pointer_move(bgpLayer, layer);

        BGPLayer::BGPCommonHeader* bgpHeader =
            bgpLayer->getCommonHeaderOrNull();
        if (bgpHeader) {
            std::unique_ptr<BGPOpenLayer>         bgpOpenLayer;
            std::unique_ptr<BGPUpdateLayer>       bgpUpdateLayer;
            std::unique_ptr<BGPNotificationLayer> bgpNotificationLayer;

            // Call BGPConnection::sendData(...) if we want to send back a
            // packet to our peer

            // TODO write the logic of the handling
            // TODO probably this must be managed with events (e.g.
            // BGPEvent::UpdateMsg). Therefore the logic should be moved inside
            // the states and here we just enqueue the right event while pushing
            // the received Message in a queue that will be read from inside the
            // states. Check that.
            // Something similar (but different) is the SegmentArrives event of
            // TCP

            L_DEBUG(owner->ID,
                    "BGP Connection : Handling the packet arrived to the BGP");

            uint8_t              subcode = 0;
            std::vector<uint8_t> data_be8;

            if (BGPLayer::checkMessageHeader(bgpHeader, &subcode)) {
                switch (bgpHeader->type) {
                    case BGPLayer::BGPMessageType::OPEN:
                        L_DEBUG(owner->ID, "OPEN message arrived");
                        dynamic_pointer_move(bgpOpenLayer, bgpLayer);

                        if (bgpOpenLayer->checkMessageErr(&subcode,
                                                          &data_be8)) {
                            BGPEvent event = {BGPEventType::BGPOpen,
                                              std::move(bgpOpenLayer)};
                            enqueueEvent(std::move(event));
                            L_DEBUG(
                                owner->ID,
                                "Arrived Open message inserted into events");
                        } else {
                            uint16_t two_octet_version = BGPOpenLayer::version;

                            switch (subcode) {
                                case 1:
                                    // Only the version 4 of the protocol is
                                    // supported
                                    bgpNotificationLayer =
                                        make_unique<BGPNotificationLayer>(
                                            BGPNotificationLayer::OPEN_MSG_ERR,
                                            BGPNotificationLayer::
                                                ERR_2_UNSUPPORTED_VERSION_NUM,
                                            (const uint8_t*)&two_octet_version,
                                            sizeof(two_octet_version));
                                    break;
                                case 2:
                                case 3:
                                case 4:
                                case 6:
                                    bgpNotificationLayer = make_unique<
                                        BGPNotificationLayer>(
                                        BGPNotificationLayer::MSG_HEADER_ERR,
                                        (BGPNotificationLayer::
                                             ErrorSubcode_uint8_t)subcode);
                                    break;
                                default:
                                    L_FATAL(owner->ID,
                                            "This State in Open message error "
                                            "checking "
                                            "should never be reached!");
                                    break;
                            }
                            bgpNotificationLayer->computeCalculateFields();

                            // HACK Attention the message enqueued is the
                            // notification message to send for the error, not
                            // the one received!
                            BGPEvent event = {BGPEventType::BGPOpenMsgErr,
                                              std::move(bgpNotificationLayer)};
                            enqueueEvent(std::move(event));
                            L_DEBUG(owner->ID,
                                    "OPEN message Error event added + "
                                    "notification message");
                        }
                        break;
                    case BGPLayer::BGPMessageType::UPDATE:
                        L_DEBUG(owner->ID, "UPDATE message arrived");
                        dynamic_pointer_move(bgpUpdateLayer, bgpLayer);

                        // NOTE: Uncomment to debug UPDATE message
                        {
                            std::string updateMessage =
                                bgpUpdateLayer->toString();
                            L_VERBOSE(stateMachine->connection->owner->ID,
                                      "UPDATE message:\n" + updateMessage);
                        }

                        if (bgpUpdateLayer->checkMessageErr(&subcode,
                                                            &data_be8)) {
                            if (bgpUpdateLayer->checkNextHop(srcAddr)) {
                                BGPEvent event = {BGPEventType::UpdateMsg,
                                                  std::move(bgpUpdateLayer)};
                                enqueueEvent(std::move(event));
                                L_DEBUG(owner->ID,
                                        "Arrived UPDATE message inserted into "
                                        "events");
                            } else {
                                L_ERROR(
                                    owner->ID,
                                    "NEXT_HOP attribute invalid -> Route (and "
                                    "UPDATE message) is going to be ignored");
                            }
                        } else {
                            switch (subcode) {
                                case 1:
                                    // Only the version 4 of the protocol is
                                    // supported
                                    bgpNotificationLayer = make_unique<
                                        BGPNotificationLayer>(
                                        BGPNotificationLayer::UPDATE_MSG_ERR,
                                        BGPNotificationLayer::
                                            ERR_3_MALFORMED_ATTR_LIST);
                                    break;
                                default:
                                    L_FATAL(
                                        owner->ID,
                                        "This State in Update Message error "
                                        "checking should not be reached");
                                    break;
                            }
                            bgpNotificationLayer->computeCalculateFields();

                            // HACK Attention the message enqueued is the
                            // notification message to send for the error, not
                            // the one received!
                            BGPEvent event = {BGPEventType::UpdateMsgErr,
                                              std::move(bgpNotificationLayer)};
                            enqueueEvent(std::move(event));
                            L_DEBUG(owner->ID,
                                    "UPDATE message Error event added + "
                                    "notification message");
                        }

                        break;
                    case BGPLayer::BGPMessageType::NOTIFICATION:
                        L_DEBUG(owner->ID, "NOTIFICATION message arrived");
                        dynamic_pointer_move(bgpNotificationLayer, bgpLayer);

                        if (bgpNotificationLayer->checkMessageErr(&subcode,
                                                                  &data_be8)) {
                            BGPEvent event = {BGPEventType::NotifMsg,
                                              std::move(bgpNotificationLayer)};
                            enqueueEvent(std::move(event));
                            L_DEBUG(owner->ID,
                                    "Arrived NOTIFICATION message inserted "
                                    "into events");
                        } else {
                            // TODO the error should be logged,
                            L_INFO(owner->ID,
                                   "Error detected in a NOTIFICATION message, "
                                   "report to administrator");
                            // FIXME Probably unecessary to forward the erroneus
                            // message
                            BGPEvent event = {BGPEventType::UpdateMsgErr,
                                              std::move(bgpNotificationLayer)};
                            enqueueEvent(std::move(event));
                            L_DEBUG(
                                owner->ID,
                                "NOTIFICATION message Error event added + "
                                "erroneous notification message (Behaviour "
                                "different from the other error messages!)");
                        }
                        break;
                    case BGPLayer::BGPMessageType::KEEPALIVE:
                        L_DEBUG(owner->ID, "KEEPALIVE message arrived");
                        {
                            BGPEvent event = {BGPEventType::KeepAliveMsg,
                                              nullptr};
                            enqueueEvent(std::move(event));
                        }
                        L_DEBUG(
                            owner->ID,
                            "Arrived KEEPALIVE message inserted into events");

                        break;
                    default:
                        L_FATAL(owner->ID,
                                "This state in the messages processing should "
                                "never be reached!");
                        break;
                }
            } else {
                switch (subcode) {
                    case 1:
                        bgpNotificationLayer =
                            std::make_unique<BGPNotificationLayer>(
                                BGPNotificationLayer::MSG_HEADER_ERR,
                                BGPNotificationLayer::ERR_1_CONN_NOT_SYNC);
                        break;
                    case 2:
                        bgpNotificationLayer =
                            std::make_unique<BGPNotificationLayer>(
                                BGPNotificationLayer::MSG_HEADER_ERR,
                                BGPNotificationLayer::ERR_1_BAD_MSG_LENGTH,
                                (const uint8_t*)&(bgpHeader->length_be),
                                (size_t)sizeof(bgpHeader->length_be));
                        break;
                    case 3:
                        bgpNotificationLayer =
                            std::make_unique<BGPNotificationLayer>(
                                BGPNotificationLayer::MSG_HEADER_ERR,
                                BGPNotificationLayer::ERR_1_BAD_MSG_TYPE,
                                &(bgpHeader->type),
                                (size_t)sizeof(bgpHeader->type));
                        break;
                    default:
                        L_FATAL(owner->ID,
                                "This State in Common header handling "
                                "should never be reached!");
                        break;
                }
                bgpNotificationLayer->computeCalculateFields();

                std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                    layers_to_send =
                        make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();
                layers_to_send->push(std::move(bgpNotificationLayer));

                stateMachine->connection->sendData(std::move(layers_to_send));
                L_WARNING(owner->ID + " " + stateMachine->name,
                          "Sending Notification message");
            }
        }
    }
}

void BGPConnection::startReceivingThread() {
    if (receivingThread != nullptr) {
        L_WARNING(owner->ID + " " + stateMachine->name,
                  "receivingThread already set");
    } else {
        receivingThread = new std::thread([&]() {
            int recvStatus = 0;
            while (running && recvStatus == 0) {
                std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                    layers;
                recvStatus = connectedSocket->recv(layers);

                if (running) {
                    if (recvStatus == 0) {
                        assert(layers != nullptr);
                        processMessage(std::move(layers));
                    } else {
                        signalTCPConnectionFails();
                    }
                }
            }
        });
    }
}

void BGPConnection::signalTCPConnectionFails() {
    BGPEvent event = {
        BGPEventType::TcpConnectionFails,
        nullptr,
    };
    enqueueEvent(std::move(event));
}

void BGPConnection::sendData(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers) {
    std::unique_lock<mutex> connectedSocket_uniqueLock(connectedSocket_mutex);
    connectedSocket->send(std::move(layers));
}

void BGPConnection::asyncConnectToPeer() {
    // TODO this is probably an async task

    BGPConnection* bgpConnection_weak =
        bgpApplication->setConnectedSocketToAvailableBGPConn(
            new Socket(owner), this->srcAddr, this->dstAddr);

    std::unique_lock<mutex> connectedSocket_uniqueLock(connectedSocket_mutex);

    assert(bgpConnection_weak->connectedSocket != nullptr);
    assert(bgpConnection_weak->connectThread == nullptr);

    bgpConnection_weak->connectThread = new thread([&, bgpConnection_weak] {
        if (bgpConnection_weak->connectedSocket->connect(
                bgpConnection_weak->dstAddr, bgpConnection_weak->dstPort) ==
            0) {
            bgpConnection_weak->startReceivingThread();
            BGPEvent event = {
                BGPEventType::TcpConnectionConfirmed,
                nullptr,
            };
            bgpConnection_weak->enqueueEvent(std::move(event));
        }
    });
}

void BGPConnection::dropConnection(bool gentle) {
    running = false;

    std::unique_lock<mutex> connectedSocket_uniqueLock(connectedSocket_mutex);
    if (connectedSocket) {
        if (gentle) {
            // Close with FIN
            connectedSocket->close();
        } else {
            // Close with RST
            // TODO implement a forced CLOSE with RST
            connectedSocket->close();
        }
    }
    delete connectedSocket;
    connectedSocket = nullptr;

    connectedSocket_uniqueLock.unlock();

    if (connectThread) {
        connectThread->join();
    }
    delete connectThread;
    connectThread = nullptr;
}

void BGPConnection::shutdown() {
    running        = false;
    BGPEvent event = {
        BGPEventType::ManualStop,
        nullptr,
    };
    enqueueEvent(std::move(event));
    // TODO wait for the shutdown call to be completed
    L_DEBUG(stateMachine->connection->owner->ID,
            "TODO wait for the shutdown call to be completed");
    std::this_thread::sleep_for(10000ms);  // TODO remove
}

bool BGPConnection::setConnectedSocketIfFree(Socket* socket) {
    std::unique_lock<mutex> connectedSocket_uniqueLock(connectedSocket_mutex);

    if (connectedSocket == nullptr) {
        connectedSocket = socket;
        connectedSocket_uniqueLock.unlock();
        return true;
    } else {
        connectedSocket_uniqueLock.unlock();
        return false;
    }
}
