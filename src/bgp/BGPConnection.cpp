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

            // here we will call sendData if we want to send back a packet to
            // our peer
            switch (bgpHeader->type) {
                case BGPLayer::BGPMessageType::OPEN:
                    L_DEBUG(owner->ID, "OPEN message arrived");
                    bgpOpenLayer_weak =
                        dynamic_cast<BGPOpenLayer*>(bgpLayer_weak);
                    holdTime = std::chrono::seconds(be16toh(
                        bgpOpenLayer_weak->getOpenHeaderOrNull()->holdTime_be));
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
            std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers =
                connectedSocket->recv();

            processMessage(std::move(layers));
        }
    });
}

void BGPConnection::sendData(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers) {
    connectedSocket->send(std::move(layers));
}