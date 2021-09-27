#ifndef BGPSIMULATION_BGP_BGPEVENT_H
#define BGPSIMULATION_BGP_BGPEVENT_H

#include <memory>
#include <string>

#include "packets/BGPLayer.h"


/**
 * A list of event the BGP State Machine is able to handle (mandatory and
 * optional)
 */
enum BGPEventType {
    ManualStart                                 = 1,  // Mandatory
    ManualStop                                  = 2,  // Mandatory
    AutomaticStart                              = 3,  // Optional
    ManualStart_with_PassiveTcpEstablishment    = 5,  // Optional
    AutomaticStart_with_PassiveTcpEstablishment = 4,  // Optional
    AutomaticStart_with_DampPeerOscillations    = 6,  // Optional
    AutomaticStart_with_DampPeerOscillations_and_PassiveTcpEstablishment =
        7,                                     // Optional
    AutomaticStop                       = 8,   // Optional
    ConnectRetryTimer_Expires           = 9,   // Mandatory
    HoldTimer_Expires                   = 10,  // Mandatory
    KeepaliveTimer_Expires              = 11,  // Mandatory
    DelayOpenTimer_Expires              = 12,  // Optional
    IdleHoldTimer_Expires               = 13,  // Optional
    TcpConnection_Valid                 = 14,  // Optional
    Tcp_CR_Invalid                      = 15,  // Optional
    Tcp_CR_Acked                        = 16,  // Mandatory
    TcpConnectionConfirmed              = 17,  // Mandatory
    TcpConnectionFails                  = 18,  // Mandatory
    BGPOpen                             = 19,  // Mandatory
    BGPOpen_with_DelayOpenTimer_running = 20,  // Optional
    BGPHeaderErr                        = 21,  // Mandatory
    BGPOpenMsgErr                       = 22,  // Mandatory
    OpenCollisionDump                   = 23,  // Optional
    NotifMsgVerErr                      = 24,  // Mandatory
    NotifMsg                            = 25,  // Mandatory
    KeepAliveMsg                        = 26,  // Mandatory
    UpdateMsg                           = 27,  // Mandatory
    UpdateMsgErr                        = 28   // Mandatory
};

struct BGPEvent {
    BGPEventType              eventType;
    std::unique_ptr<BGPLayer> layers;
};

/**
 * Return the string name of the event
 * @param event the event code
 * @return string name of the event
 */
std::string getEventName(BGPEvent &event);

#endif  // BGPSIMULATION_BGP_BGPEVENT_H
