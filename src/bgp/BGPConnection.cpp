#include "BGPConnection.h"

#include "../logger/Logger.h"
#include "fsm/BGPStateIdle.h"
#include "packets/BGPLayer.h"
#include "packets/BGPOpenLayer.h"
#include "packets/BGPUpdateLayer.h"


BGPConnection::BGPConnection(Router* owner) : owner(owner) {
    stateMachine = new BGPStateMachine(this);
    this->stateMachine->changeState(new BGPStateIdle(this->stateMachine));
    stateMachine->start();
}
BGPConnection::~BGPConnection() {
    running = false;
    Socket* s =
        owner->getNewSocket(Socket::Domain::AF_INET, Socket::Type::SOCK_STREAM);
    enqueueEvent(BGPEvent::ManualStop);
    // create fake connection to stop the
    s->connect(pcpp::IPv4Address::Zero, 179);
    delete connectedSocket;


    delete stateMachine;
}

void BGPConnection::enqueueEvent(BGPEvent event) {
    stateMachine->enqueueEvent(event);
}


void BGPConnection::processMessage(std::stack<pcpp::Layer*>* layers) {
    L_DEBUG(owner->ID, "Processing BGP Message");
    while (!layers->empty()) {
        BGPLayer* bgpLayer = dynamic_cast<BGPLayer*>(layers->top());
        layers->pop();
        BGPLayer::BGPCommonHeader* bgpHeader =
            bgpLayer->getCommonHeaderOrNull();
        if (bgpHeader) {
            BGPOpenLayer* bgpOpenLayer = nullptr;
            switch (bgpHeader->type) {
                case BGPLayer::BGPMessageType::OPEN:
                    L_DEBUG(owner->ID, "OPEN message arrived");
                    bgpOpenLayer = dynamic_cast<BGPOpenLayer*>(bgpLayer);
                    holdTime     = std::chrono::seconds(be16toh(
                        bgpOpenLayer->getOpenHeaderOrNull()->holdTime_be));
                    owner->bgpApplication->collisionDetection(this);
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

        delete bgpLayer;
    }
}


void BGPConnection::connect() {
    connectedSocket =
        owner->getNewSocket(Socket::Domain::AF_INET, Socket::Type::SOCK_STREAM);

    connectedSocket->connect(dstAddr, 179);

    enqueueEvent(BGPEvent::TcpConnection_Valid);
}

void BGPConnection::closeConnection() {
    enqueueEvent(BGPEvent::TcpConnectionFails);
}

void BGPConnection::receiveData() {
    receivingThread = new std::thread([&]() {
        while (running) {
            std::stack<pcpp::Layer*>* layers = connectedSocket->recv();

            processMessage(layers);
        }
    });
}