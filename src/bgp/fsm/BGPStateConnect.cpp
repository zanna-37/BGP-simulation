#include "BGPStateConnect.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <stack>
#include <utility>

#include "../../entities/Router.h"
#include "../../utils/Timer.h"
#include "../BGPConnection.h"
#include "../BGPEvent.h"
#include "../BGPTimer.h"
#include "../packets/BGPLayer.h"
#include "../packets/BGPOpenLayer.h"
#include "BGPStateActive.h"
#include "BGPStateIdle.h"
#include "BGPStateMachine.h"
#include "BGPStateOpenConfirm.h"
#include "BGPStateOpenSent.h"
#include "IpAddress.h"
#include "Layer.h"


bool BGPStateConnect ::onEvent(BGPEvent event) {
    bool handled = true;

    std::unique_ptr<BGPLayer>                                 bgpOpenLayer;
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers;

    switch (event) {
        case BGPEvent::ManualStop:
            // TODO drops the TCP connection,
            // dropTCPConnection();
            // TODO releases all BGP resources,

            // - sets ConnectRetryCounter to zero,
            stateMachine->setConnectRetryCounter(0);

            // - stops the ConnectRetryTimer and sets ConnectRetryTimer to
            //   zero, and
            stateMachine->resetConnectRetryTimer();

            // - changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));

            break;
        case BGPEvent::ConnectRetryTimer_Expires:
            // TODO drops the TCP connection,
            // dropTCPConnection();
            // - restarts the ConnectRetryTimer,
            stateMachine->resetConnectRetryTimer();
            stateMachine->connectRetryTimer->start();

            // - stops the DelayOpenTimer and resets the timer to zero,
            stateMachine->resetDelayOpenTimer();

            // TODO initiates a TCP connection to the other BGP peer,
            // initiateTCPConnection();
            // TODO continues to listen for a connection that may be initiated
            // by
            //   the remote BGP peer

            break;
        case BGPEvent::DelayOpenTimer_Expires:
            // TODO sends an OPEN message to its peer,

            // - NOT_SURE sets the HoldTimer to a large value, and
            stateMachine->setHoldTime(240s);   // 4 minutes suggested
            stateMachine->holdTimer->start();  // do we need to start it?

            // - changes its state to OpenSent.
            stateMachine->changeState(new BGPStateOpenSent(stateMachine));
            break;
        case BGPEvent::TcpConnection_Valid:
            // TODO the TCP connection is processed, OPTIONAL
            break;
        case BGPEvent::Tcp_CR_Invalid:

            // TODO the local system rejects the TCP connection, OPTIONAL
            break;
        case BGPEvent::Tcp_CR_Acked:
        case BGPEvent::TcpConnectionConfirmed:

            // If the DelayOpen attribute is set to TRUE, the local system:
            if (stateMachine->getDelayOpen()) {
                // - stops the ConnectRetryTimer (if running) and sets the
                //   ConnectRetryTimer to zero,
                stateMachine->resetConnectRetryTimer();

                // - sets the DelayOpenTimer to the initial value, and
                stateMachine->delayOpenTimer->start();

                // - stays in the Connect state.

                // If the DelayOpen attribute is set to FALSE, the local system:
            } else {
                // - stops the ConnectRetryTimer (if running) and sets the
                //   ConnectRetryTimer to zero,
                stateMachine->resetConnectRetryTimer();

                // TODO completes BGP initialization

                // TODO sends an OPEN message to its peer,

                // FIXME
                // dynamic_cast<Router*>(stateMachine->connection->owner)->AS_number
                bgpOpenLayer = std::make_unique<BGPOpenLayer>(
                    1111,
                    (uint16_t)(stateMachine->getHoldTime().count()),
                    pcpp::IPv4Address("1.1.1.1"));
                bgpOpenLayer->computeCalculateFields();

                layers->push(std::move(bgpOpenLayer));

                stateMachine->connection->sendData(std::move(layers));

                // - NOT_SURE sets the HoldTimer to a large value, and
                stateMachine->setHoldTime(240s);
                stateMachine->holdTimer->start();

                // - changes its state to OpenSent.
                stateMachine->changeState(new BGPStateOpenSent(stateMachine));
            }
            break;

        case BGPEvent::TcpConnectionFails:
            //     If the DelayOpenTimer is running, the local
            //   system:
            if (stateMachine->delayOpenTimer->getState() == TICKING) {
                //     - restarts the ConnectRetryTimer with the initial value,
                stateMachine->resetConnectRetryTimer();
                ;
                stateMachine->connectRetryTimer->start();

                //     - stops the DelayOpenTimer and resets its value to zero,
                stateMachine->resetDelayOpenTimer();

                //     -  TODO continues to listen for a connection that may be
                //     initiated by
                //       the remote BGP peer, and

                //     - changes its state to Active.
                stateMachine->changeState(new BGPStateActive(stateMachine));

                //   If the DelayOpenTimer is not running, the local system:
            } else {
                //     - stops the ConnectRetryTimer to zero,
                stateMachine->resetConnectRetryTimer();

                //     TODO drops the TCP connection,
                // dropTCPConnection();

                //     TODO releases all BGP resources, and

                //     - changes its state to Idle.
                stateMachine->changeState(new BGPStateActive(stateMachine));
            }
            break;
        case BGPEvent::BGPOpen_with_DelayOpenTimer_running:
            // - stops the ConnectRetryTimer (if running) and sets the
            //   ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO completes the BGP initialization,

            // - stops and clears the DelayOpenTimer (sets the value to zero),
            stateMachine->resetDelayOpenTimer();

            // TODO sends an OPEN message,

            // TODO sends a KEEPALIVE message,

            // - if the HoldTimer initial value is non-zero, // FIXME
            if (stateMachine->holdTimer->getDuration() != 0ms) {
                //     - starts the KeepaliveTimer with the initial value and
                stateMachine->keepAliveTimer->start();

                //     - resets the HoldTimer to the negotiated value, // FIXME
                stateMachine->holdTimer =
                    new BGPTimer("holdTimer",
                                 stateMachine,
                                 BGPEvent::HoldTimer_Expires,
                                 240s);
                // stateMachine->holdTimer->setRemainingTime(0ms);  // NOT_SURE

                //   else, if the HoldTimer initial value is zero,
            } else {
                //     - resets the KeepaliveTimer and
                stateMachine->resetKeepAliveTimer();
                //     - resets the HoldTimer value to zero,
                stateMachine->resetHoldTimer();
            }
            // - and changes its state to OpenConfirm.
            stateMachine->changeState(new BGPStateOpenConfirm(stateMachine));
            break;
        case BGPEvent::BGPHeaderErr:
        case BGPEvent::BGPOpenMsgErr:

            if (stateMachine->getSendNOTIFICATIONwithoutOPEN()) {
                // TODO
                // - (optionally) If the SendNOTIFICATIONwithoutOPEN attribute
                // is
                //   set to TRUE, then the local system first sends a
                //   NOTIFICATION message with the appropriate error code, and
                //   then
            }

            // - stops the ConnectRetryTimer (if running) and sets the
            //   ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO releases all BGP resources,

            // TODO drops the TCP connection,
            // dropTCPConnection();

            // - increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            if (stateMachine->getDampPeerOscillations()) {
                // - (optionally) performs peer oscillation damping if the
                //   DampPeerOscillations attribute is set to TRUE, and
            }

            // - changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));

            break;
        case BGPEvent::NotifMsgVerErr:
            if (stateMachine->delayOpenTimer->getState() == TICKING) {
                //     - stops the ConnectRetryTimer (if running) and sets the
                //       ConnectRetryTimer to zero,
                stateMachine->resetConnectRetryTimer();

                //     - stops and resets the DelayOpenTimer (sets to zero),
                stateMachine->resetDelayOpenTimer();

                //     TODO releases all BGP resources,

                //     TODO drops the TCP connection, and
                // dropTCPConnection();

                //     - changes its state to Idle.


                //   If the DelayOpenTimer is not running, the local system:
            } else {
                //     - stops the ConnectRetryTimer and sets the
                //     ConnectRetryTimer to
                //       zero,
                stateMachine->resetConnectRetryTimer();

                //     TODO releases all BGP resources,

                //     TODO drops the TCP connection,
                // dropTCPConnection();

                //     - increments the ConnectRetryCounter by 1,
                stateMachine->incrementConnectRetryCounter();

                if (stateMachine->getDampPeerOscillations()) {
                    //     - performs peer oscillation damping if the
                    //     DampPeerOscillations
                    //       attribute is set to True, and
                }

                //     - changes its state to Idle.
            }
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;
        case BGPEvent::AutomaticStop:
        case BGPEvent::HoldTimer_Expires:
        case BGPEvent::KeepaliveTimer_Expires:
        case BGPEvent::BGPOpen:
        case BGPEvent::OpenCollisionDump:
        case BGPEvent::NotifMsg:
        case BGPEvent::KeepAliveMsg:
        case BGPEvent::UpdateMsg:
        case BGPEvent::UpdateMsgErr:
            // - if the ConnectRetryTimer is running, stops and resets the
            //   ConnectRetryTimer (sets to zero),
            stateMachine->resetConnectRetryTimer();
            // - if the DelayOpenTimer is running, stops and resets the
            //   DelayOpenTimer (sets to zero),
            stateMachine->resetDelayOpenTimer();

            // TODO releases all BGP resources,

            // TODO drops the TCP connection,
            // dropTCPConnection();

            // - increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            if (stateMachine->getDampPeerOscillations()) {
                // - performs peer oscillation damping if the
                // DampPeerOscillations
                //   attribute is set to True, and
            }

            // - changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;
        default:
            handled = false;
            break;
    }
    return handled;
}

// TODO: check how to implement this
// If the DelayOpenTimer is not running, the local system:

// - stops the ConnectRetryTimer to zero,

// - drops the TCP connection,

// - releases all BGP resources, and

// - changes its state to Idle.

// TODO

//   If the value of the autonomous system field is the same as the
//   local Autonomous System number, set the connection status to an
//   internal connection; otherwise it will be "external".
