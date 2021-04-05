#include "BGPEvent.h"

#include "../logger/Logger.h"

std::string getEventName(BGPEvent event) {
    switch (event) {
        case BGPEvent::ManualStart:
            return "[" + std::to_string((int)BGPEvent::ManualStart) +
                   "-Manual start]";
        case BGPEvent::ManualStop:
            return "[" + std::to_string((int)BGPEvent::ManualStop) +
                   "-Manual stop]";
        case BGPEvent::AutomaticStart:
            return "[" + std::to_string((int)BGPEvent::AutomaticStart) +
                   "-Automatic start]";
        case BGPEvent::ManualStart_with_PassiveTcpEstablishment:
            return "[" +
                   std::to_string((
                       int)BGPEvent::ManualStart_with_PassiveTcpEstablishment) +
                   "-Manual start with Passive Tcp Establishment]";
        case BGPEvent::AutomaticStart_with_PassiveTcpEstablishment:
            return "[" +
                   std::to_string(
                       (int)BGPEvent::
                           AutomaticStart_with_PassiveTcpEstablishment) +
                   "-Automatic start with Passive Tcp Establishment]";
        case BGPEvent::AutomaticStart_with_DampPeerOscillations:
            return "[" +
                   std::to_string((
                       int)BGPEvent::AutomaticStart_with_DampPeerOscillations) +
                   "-Automatic start with Damp Peer Oscillations]";
        case BGPEvent::
            AutomaticStart_with_DampPeerOscillations_and_PassiveTcpEstablishment:
            return "[" +
                   std::to_string(
                       (int)BGPEvent::
                           AutomaticStart_with_DampPeerOscillations_and_PassiveTcpEstablishment) +
                   "-Automatic start with Damp Peer Oscillations and Passive "
                   "Tcp Establishment]";
        case BGPEvent::AutomaticStop:
            return "[" + std::to_string((int)BGPEvent::AutomaticStop) +
                   "-Automatic stop]";
        case BGPEvent::ConnectRetryTimer_Expires:
            return "[" +
                   std::to_string((int)BGPEvent::ConnectRetryTimer_Expires) +
                   "-ConnectRetryTimer expires]";
        case BGPEvent::HoldTimer_Expires:
            return "[" + std::to_string((int)BGPEvent::HoldTimer_Expires) +
                   "-HoldTimer expires]";
        case BGPEvent::KeepaliveTimer_Expires:
            return "[" + std::to_string((int)BGPEvent::KeepaliveTimer_Expires) +
                   "-KeepaliveTimer expires]";
        case BGPEvent::DelayOpenTimer_Expires:
            return "[" + std::to_string((int)BGPEvent::DelayOpenTimer_Expires) +
                   "-DelayOpenTimer expires]";
        case BGPEvent::IdleHoldTimer_Expires:
            return "[" + std::to_string((int)BGPEvent::IdleHoldTimer_Expires) +
                   "-IdleHoldTimer expires]";
        case BGPEvent::TcpConnection_Valid:
            return "[" + std::to_string((int)BGPEvent::TcpConnection_Valid) +
                   "-Tcp connection valid]";
        case BGPEvent::Tcp_CR_Invalid:
            return "[" + std::to_string((int)BGPEvent::Tcp_CR_Invalid) +
                   "-Tcp CR Invalid]";
        case BGPEvent::Tcp_CR_Acked:
            return "[" + std::to_string((int)BGPEvent::Tcp_CR_Acked) +
                   "-Tcp CR Acked]";
        case BGPEvent::TcpConnectionConfirmed:
            return "[" + std::to_string((int)BGPEvent::TcpConnectionConfirmed) +
                   "-Tcp connection confirmed]";
        case BGPEvent::TcpConnectionFails:
            return "[" + std::to_string((int)BGPEvent::TcpConnectionFails) +
                   "-Tcp connection fails]";
        case BGPEvent::BGPOpen:
            return "[" + std::to_string((int)BGPEvent::BGPOpen) + "-BGP open]";
        case BGPEvent::BGPOpen_with_DelayOpenTimer_running:
            return "[" +
                   std::to_string(
                       (int)BGPEvent::BGPOpen_with_DelayOpenTimer_running) +
                   "-BGP open with DelayOpenTimer running]";
        case BGPEvent::BGPHeaderErr:
            return "[" + std::to_string((int)BGPEvent::BGPHeaderErr) +
                   "-BGP header error]";
        case BGPEvent::BGPOpenMsgErr:
            return "[" + std::to_string((int)BGPEvent::BGPOpenMsgErr) +
                   "-BGPOpenMsg error]";
        case BGPEvent::OpenCollisionDump:
            return "[" + std::to_string((int)BGPEvent::OpenCollisionDump) +
                   "-Open collision dump]";
        case BGPEvent::NotifMsgVerErr:
            return "[" + std::to_string((int)BGPEvent::NotifMsgVerErr) +
                   "-NotifMsgVer error]";
        case BGPEvent::NotifMsg:
            return "[" + std::to_string((int)BGPEvent::NotifMsg) + "-NotifMsg]";
        case BGPEvent::KeepAliveMsg:
            return "[" + std::to_string((int)BGPEvent::KeepAliveMsg) +
                   "-KeepAliveMsg]";
        case BGPEvent::UpdateMsg:
            return "[" + std::to_string((int)BGPEvent::UpdateMsg) +
                   "-UpdateMsg]";
        case BGPEvent::UpdateMsgErr:
            return "[" + std::to_string((int)event) + "-UpdateMsg error]";
        default:
            L_ERROR("THIS SHOULD NEVER HAPPEN: unknown BGP Event");
            return "[" + std::to_string((int)event) + "-Unknown]";
    }
}