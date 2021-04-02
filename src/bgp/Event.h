#ifndef EVENT_H
#define EVENT_H

#include <map>
#include <string>

enum Event {
    __SHUTDOWN,
    ManualStart,
    ManualStop,
    AutomaticStart,
    ManualStart_with_PassiveTcpEstablishment,
    AutomaticStart_with_PassiveTcpEstablishment,
    AutomaticStart_with_DampPeerOscillations,
    AutomaticStart_with_DampPeerOscillations_and_PassiveTcpEstablishment,
    AutomaticStop,
    ConnectRetryTimer_Expires,
    HoldTimer_Expires,
    KeepaliveTimer_Expires,
    DelayOpenTimer_Expires,
    IdleHoldTimer_Expires,
    TcpConnection_Valid,
    Tcp_CR_Invalid,
    Tcp_CR_Acked,
    TcpConnectionConfirmed,
    TcpConnectionFails,
    BGPOpen,
    BGPOpen_with_DelayOpenTimer_running,
    BGPHeaderErr,
    BGPOpenMsgErr,
    OpenCollisionDump,
    NotifMsgVerErr,
    NotifMsg,
    KeepAliveMsg,
    UpdateMsg,
    UpdateMsgErr
};

/**
 * Return the string name of the event
 * @param event the event code
 * @return string name of the event
 */
std::string getEventName(Event event);

#endif
