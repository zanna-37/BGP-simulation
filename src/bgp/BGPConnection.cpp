#include "BGPConnection.h"

#include "fsm/BGPStateIdle.h"
#include "packets/BGPLayer.h"
#include "packets/BGPOpenLayer.h"
#include "packets/BGPUpdateLayer.h"

// C++ equivalent of Java instanceof
template <typename Base, typename T>
inline bool instanceof (const T*) {
    return is_base_of<Base, T>::value;
}


BGPConnection::BGPConnection(Device* owner) : owner(owner) {
    stateMachine = new BGPStateMachine(this);
    this->stateMachine->changeState(new BGPStateIdle(this->stateMachine));
    stateMachine->start();
}
BGPConnection::~BGPConnection() { delete stateMachine; }

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