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
    while (!layers->empty()) {
        BGPLayer* bgpLayer = dynamic_cast<BGPLayer*>(layers->top());
        layers->pop();

        if (instanceof <BGPOpenLayer>(bgpLayer)) {
        } else if (instanceof <BGPUpdateLayer>(bgpLayer)) {
        } else {
            // TODO notification
            // TODO keepalive
            // TODO not recognized
        }
    }
}