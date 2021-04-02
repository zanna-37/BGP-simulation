#include "BGPStateActive.h"

#include "BGPStateConnect.h"
#include "BGPStateOpenSent.h"
BGPStateActive ::~BGPStateActive() {}

bool BGPStateActive ::onEvent(BGPEvent event) {
    bool handled = true;
    switch (event) {
        case BGPEvent::ManualStop:
            if (stateMachine->delayOpenTimer->getState() == TICKING &&
                stateMachine->getSendNOTIFICATIONwithoutOPEN()) {
                // - If the DelayOpenTimer is running and the
                //   SendNOTIFICATIONwithoutOPEN session attribute is set, the
                //   local system sends a NOTIFICATION with a Cease,
            }

            // TODO releases all BGP resources including stopping the
            //   DelayOpenTimer
            stateMachine->resetDelayOpenTimer();

            // TODO drops the TCP connection,

            // - sets ConnectRetryCounter to zero,
            stateMachine->setConnectRetryCounter(0);

            // - stops the ConnectRetryTimer and sets the ConnectRetryTimer to
            //   zero, and
            stateMachine->resetConnectRetryTimer();

            // - changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));

            break;
        case BGPEvent::ConnectRetryTimer_Expires:
            //  - restarts the ConnectRetryTimer (with initial value),
            stateMachine->resetConnectRetryTimer();
            stateMachine->connectRetryTimer->start();

            // TODO initiates a TCP connection to the other BGP peer,

            // TODO continues to listen for a TCP connection that may be
            // initiated
            //   by a remote BGP peer, and

            // - changes its state to Connect.
            stateMachine->changeState(new BGPStateConnect(stateMachine));
            break;
        case BGPEvent::DelayOpenTimer_Expires:
            //  - sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // - stops and clears the DelayOpenTimer (set to zero),
            stateMachine->resetDelayOpenTimer();

            // TODO completes the BGP initialization,

            // TODO sends the OPEN message to its remote peer,

            // - sets its hold timer to a large value ( A HoldTimer value of 4
            // minutes is also suggested for this state transition.), and
            stateMachine->holdTimer = new Timer(
                "holdTimer", stateMachine, BGPEvent::HoldTimer_Expires, 240s);
            stateMachine->holdTimer->start();

            // - changes its state to OpenSent
            stateMachine->changeState(new BGPStateOpenSent(stateMachine));
            break;
        case BGPEvent::TcpConnection_Valid:
            // TODO the local system processes the TCP connection flags and
            // stays in the Active state.
            break;
        case BGPEvent::Tcp_CR_Invalid:
            // TODO the local system rejects the TCP connection and stays in the
            // Active State.
            break;
        case BGPEvent::Tcp_CR_Acked:
        case BGPEvent::TcpConnectionConfirmed:

            // If the DelayOpen attribute is set to TRUE, the local system:
            if (stateMachine->getDelayOpen()) {
                //   - stops the ConnectRetryTimer and sets the
                //   ConnectRetryTimer
                //     to zero,
                stateMachine->resetConnectRetryTimer();

                //   - sets the DelayOpenTimer to the initial value
                //     (DelayOpenTime), and
                stateMachine->resetDelayOpenTimer();
                stateMachine->delayOpenTimer->start();

                //   - stays in the Active state.

            } else {
                // If the DelayOpen attribute is set to FALSE, the local system:

                //   - sets the ConnectRetryTimer to zero,
                stateMachine->resetConnectRetryTimer();

                //   TODO completes the BGP initialization,

                //   TODO sends the OPEN message to its peer,

                //   - sets its HoldTimer to a large value, and
                stateMachine->resetHoldTimer();
                stateMachine->holdTimer = new Timer("holdTimer",
                                                    stateMachine,
                                                    BGPEvent::HoldTimer_Expires,
                                                    240s);
                stateMachine->holdTimer->start();


                //   - changes its state to OpenSent.
                stateMachine->changeState(new BGPStateOpenSent(stateMachine));
            }
            break;
        case BGPEvent::TcpConnectionFails:
            // - restarts the ConnectRetryTimer (with the initial value),
            stateMachine->resetConnectRetryTimer();
            stateMachine->connectRetryTimer->start();

            // - stops and clears the DelayOpenTimer (sets the value to zero),
            stateMachine->resetDelayOpenTimer();

            // TODO releases all BGP resource,

            // - increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            // - optionally performs peer oscillation damping if the
            //   DampPeerOscillations attribute is set to TRUE, and
            if (stateMachine->getDampPeerOscillations()) {
            }

            // - changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;
        case BGPEvent::BGPOpen_with_DelayOpenTimer_running:
            //  - stops the ConnectRetryTimer (if running) and sets the
            //   ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // - stops and clears the DelayOpenTimer (sets to zero),
            stateMachine->resetDelayOpenTimer();

            // TODO completes the BGP initialization,

            // TODO sends an OPEN message,

            // TODO sends a KEEPALIVE message,

            // - if the HoldTimer value is non-zero, // FIXME
            if (stateMachine->holdTimer->getDuration() != 0ms) {
                //     - starts the KeepaliveTimer to initial value,
                stateMachine->keepAliveTimer->start();

                //     - resets the HoldTimer to the negotiated value,
                stateMachine->resetHoldTimer();
                stateMachine->holdTimer->start();
            } else {
                //   else if the HoldTimer is zero

                //     - resets the KeepaliveTimer (set to zero),
                stateMachine->resetKeepAliveTimer();

                //     - resets the HoldTimer to zero, and
                stateMachine->resetHoldTimer();
            }

            // - changes its state to OpenConfirm.
            stateMachine->changeState(new BGPStateOpenSent(stateMachine));
            break;
        case BGPEvent::BGPOpenMsgErr:
        case BGPEvent::BGPHeaderErr:
            //  - (optionally) sends a NOTIFICATION message with the appropriate
            //   error code if the SendNOTIFICATIONwithoutOPEN attribute is set
            //   to TRUE,
            if (stateMachine->getSendNOTIFICATIONwithoutOPEN()) {
                // TODO
            }

            // - sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO releases all BGP resources,

            // TODO  drops the TCP connection,

            // - increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            // - (optionally) performs peer oscillation damping if the
            //   DampPeerOscillations attribute is set to TRUE, and
            if (stateMachine->getDampPeerOscillations()) {
            }

            // - changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;
        case BGPEvent::NotifMsgVerErr:
            // - stops the ConnectRetryTimer (if running) and sets the
            //   ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // - stops and resets the DelayOpenTimer (sets to zero),
            stateMachine->resetDelayOpenTimer();

            // TODO releases all BGP resources,

            // TODO drops the TCP connection, and

            // - changes its state to Idle.
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

            // - sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO releases all BGP resources,

            // TODO drops the TCP connection,

            // - increments the ConnectRetryCounter by one,
            stateMachine->incrementConnectRetryCounter();

            // - (optionally) performs peer oscillation damping if the
            //   DampPeerOscillations attribute is set to TRUE, and
            if (stateMachine->getDampPeerOscillations()) {
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

// TODO
// If the DelayOpenTimer is not running, the local system:

//         - sets the ConnectRetryTimer to zero,

//         - releases all BGP resources,

//         - drops the TCP connection,

//         - increments the ConnectRetryCounter by 1,

//         - (optionally) performs peer oscillation damping if the
//           DampPeerOscillations attribute is set to TRUE, and

//         - changes its state to Idle.