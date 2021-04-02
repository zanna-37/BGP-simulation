#include "Event.h"

std::string getEventName(Event event) {
    switch (event) {
        case __SHUTDOWN:
            return "[" + std::to_string(__SHUTDOWN) + "-INTERNAL SHUTDOWN]";
        case ManualStart:
            return "[" + std::to_string(ManualStart) + "-Manual start]";
        case ManualStop:
            return "[" + std::to_string(ManualStop) + "-Manual stop]";
        case AutomaticStart:
            return "[" + std::to_string(AutomaticStart) + "-Automatic start]";
        case ManualStart_with_PassiveTcpEstablishment:
            return "[" +
                   std::to_string(ManualStart_with_PassiveTcpEstablishment) +
                   "-Manual start with Passive Tcp Establishment]";
        case AutomaticStart_with_PassiveTcpEstablishment:
            return "[" +
                   std::to_string(AutomaticStart_with_PassiveTcpEstablishment) +
                   "-Automatic start with Passive Tcp Establishment]";
        case AutomaticStart_with_DampPeerOscillations:
            return "[" +
                   std::to_string(AutomaticStart_with_DampPeerOscillations) +
                   "-Automatic start with Damp Peer Oscillations]";
        case AutomaticStart_with_DampPeerOscillations_and_PassiveTcpEstablishment:
            return "[" +
                   std::to_string(
                       AutomaticStart_with_DampPeerOscillations_and_PassiveTcpEstablishment) +
                   "-Automatic start with Damp Peer Oscillations and Passive "
                   "Tcp Establishment]";
        case AutomaticStop:
            return "[" + std::to_string(AutomaticStop) + "-Automatic stop]";
        case ConnectRetryTimer_Expires:
            return "[" + std::to_string(ConnectRetryTimer_Expires) +
                   "-ConnectRetryTimer expires]";
        case HoldTimer_Expires:
            return "[" + std::to_string(HoldTimer_Expires) +
                   "-HoldTimer expires]";
        case KeepaliveTimer_Expires:
            return "[" + std::to_string(KeepaliveTimer_Expires) +
                   "-KeepaliveTimer expires]";
        case DelayOpenTimer_Expires:
            return "[" + std::to_string(DelayOpenTimer_Expires) +
                   "-DelayOpenTimer expires]";
        case IdleHoldTimer_Expires:
            return "[" + std::to_string(IdleHoldTimer_Expires) +
                   "-IdleHoldTimer expires]";
        case TcpConnection_Valid:
            return "[" + std::to_string(TcpConnection_Valid) +
                   "-Tcp connection valid]";
        case Tcp_CR_Invalid:
            return "[" + std::to_string(Tcp_CR_Invalid) + "-Tcp CR Invalid]";
        case Tcp_CR_Acked:
            return "[" + std::to_string(Tcp_CR_Acked) + "-Tcp CR Acked]";
        case TcpConnectionConfirmed:
            return "[" + std::to_string(TcpConnectionConfirmed) +
                   "-Tcp connection confirmed]";
        case TcpConnectionFails:
            return "[" + std::to_string(TcpConnectionFails) +
                   "-Tcp connection fails]";
        case BGPOpen:
            return "[" + std::to_string(BGPOpen) + "-BGP open]";
        case BGPOpen_with_DelayOpenTimer_running:
            return "[" + std::to_string(BGPOpen_with_DelayOpenTimer_running) +
                   "-BGP open with DelayOpenTimer running]";
        case BGPHeaderErr:
            return "[" + std::to_string(BGPHeaderErr) + "-BGP header error]";
        case BGPOpenMsgErr:
            return "[" + std::to_string(BGPOpenMsgErr) + "-BGPOpenMsg error]";
        case OpenCollisionDump:
            return "[" + std::to_string(OpenCollisionDump) +
                   "-Open collision dump]";
        case NotifMsgVerErr:
            return "[" + std::to_string(NotifMsgVerErr) + "-NotifMsgVer error]";
        case NotifMsg:
            return "[" + std::to_string(NotifMsg) + "-NotifMsg]";
        case KeepAliveMsg:
            return "[" + std::to_string(KeepAliveMsg) + "-KeepAliveMsg]";
        case UpdateMsg:
            return "[" + std::to_string(UpdateMsg) + "-UpdateMsg]";
        case UpdateMsgErr:
            return "[" + std::to_string(UpdateMsgErr) + "-UpdateMsg error]";
        default:
            return "THIS SHOULD NEVER HAPPEN";
    }
}
