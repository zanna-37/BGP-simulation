#ifndef BGPEVENT_H
#define BGPEVENT_H

#include <map>
#include <string>

/**
 * A list of event the BGP State Machine is able to handle (mandatory and
 * optional)
 */
enum BGPEvent {
    ManualStart                                                          = 1,
    ManualStop                                                           = 2,
    AutomaticStart                                                       = 3,
    ManualStart_with_PassiveTcpEstablishment                             = 5,
    AutomaticStart_with_PassiveTcpEstablishment                          = 4,
    AutomaticStart_with_DampPeerOscillations                             = 6,
    AutomaticStart_with_DampPeerOscillations_and_PassiveTcpEstablishment = 7,
    AutomaticStop                                                        = 8,
    ConnectRetryTimer_Expires                                            = 9,
    HoldTimer_Expires                                                    = 10,
    KeepaliveTimer_Expires                                               = 11,
    DelayOpenTimer_Expires                                               = 12,
    IdleHoldTimer_Expires                                                = 13,
    TcpConnection_Valid                                                  = 14,
    Tcp_CR_Invalid                                                       = 15,
    Tcp_CR_Acked                                                         = 16,
    TcpConnectionConfirmed                                               = 17,
    TcpConnectionFails                                                   = 18,
    BGPOpen                                                              = 19,
    BGPOpen_with_DelayOpenTimer_running                                  = 20,
    BGPHeaderErr                                                         = 21,
    BGPOpenMsgErr                                                        = 22,
    OpenCollisionDump                                                    = 23,
    NotifMsgVerErr                                                       = 24,
    NotifMsg                                                             = 25,
    KeepAliveMsg                                                         = 26,
    UpdateMsg                                                            = 27,
    UpdateMsgErr                                                         = 28
};

/**
 * Return the string name of the event
 * @param event the event code
 * @return string name of the event
 */
std::string getEventName(BGPEvent event);

#endif
