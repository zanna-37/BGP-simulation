#include "BGPEvent.h"

#include "../logger/Logger.h"

std::string getEventName(BGPEvent event) {
    switch (event) {
        case BGPEvent::ManualStart:
            return "[" + std::to_string(BGPEvent::ManualStart) +
                   "-Manual start]";
        case BGPEvent::ManualStop:
            return "[" + std::to_string(BGPEvent::ManualStop) + "-Manual stop]";
        case BGPEvent::AutomaticStart:
            return "[" + std::to_string(BGPEvent::AutomaticStart) +
                   "-Automatic start]";
        case BGPEvent::ManualStart_with_PassiveTcpEstablishment:
            return "[" +
                   std::to_string(
                       BGPEvent::ManualStart_with_PassiveTcpEstablishment) +
                   "-Manual start with Passive Tcp Establishment]";
        case BGPEvent::AutomaticStart_with_PassiveTcpEstablishment:
            return "[" +
                   std::to_string(
                       BGPEvent::AutomaticStart_with_PassiveTcpEstablishment) +
                   "-Automatic start with Passive Tcp Establishment]";
        case BGPEvent::AutomaticStart_with_DampPeerOscillations:
            return "[" +
                   std::to_string(
                       BGPEvent::AutomaticStart_with_DampPeerOscillations) +
                   "-Automatic start with Damp Peer Oscillations]";
        case BGPEvent::
            AutomaticStart_with_DampPeerOscillations_and_PassiveTcpEstablishment:
            return "[" +
                   std::to_string(
                       BGPEvent::
                           AutomaticStart_with_DampPeerOscillations_and_PassiveTcpEstablishment) +
                   "-Automatic start with Damp Peer Oscillations and Passive "
                   "Tcp Establishment]";
        case BGPEvent::AutomaticStop:
            return "[" + std::to_string(BGPEvent::AutomaticStop) +
                   "-Automatic stop]";
        case BGPEvent::ConnectRetryTimer_Expires:
            return "[" + std::to_string(BGPEvent::ConnectRetryTimer_Expires) +
                   "-ConnectRetryTimer expires]";
        case BGPEvent::HoldTimer_Expires:
            return "[" + std::to_string(BGPEvent::HoldTimer_Expires) +
                   "-HoldTimer expires]";
        case BGPEvent::KeepaliveTimer_Expires:
            return "[" + std::to_string(BGPEvent::KeepaliveTimer_Expires) +
                   "-KeepaliveTimer expires]";
        case BGPEvent::DelayOpenTimer_Expires:
            return "[" + std::to_string(BGPEvent::DelayOpenTimer_Expires) +
                   "-DelayOpenTimer expires]";
        case BGPEvent::IdleHoldTimer_Expires:
            return "[" + std::to_string(BGPEvent::IdleHoldTimer_Expires) +
                   "-IdleHoldTimer expires]";
        case BGPEvent::TcpConnection_Valid:
            return "[" + std::to_string(BGPEvent::TcpConnection_Valid) +
                   "-Tcp connection valid]";
        case BGPEvent::Tcp_CR_Invalid:
            return "[" + std::to_string(BGPEvent::Tcp_CR_Invalid) +
                   "-Tcp CR Invalid]";
        case BGPEvent::Tcp_CR_Acked:
            return "[" + std::to_string(BGPEvent::Tcp_CR_Acked) +
                   "-Tcp CR Acked]";
        case BGPEvent::TcpConnectionConfirmed:
            return "[" + std::to_string(BGPEvent::TcpConnectionConfirmed) +
                   "-Tcp connection confirmed]";
        case BGPEvent::TcpConnectionFails:
            return "[" + std::to_string(BGPEvent::TcpConnectionFails) +
                   "-Tcp connection fails]";
        case BGPEvent::BGPOpen:
            return "[" + std::to_string(BGPEvent::BGPOpen) + "-BGP open]";
        case BGPEvent::BGPOpen_with_DelayOpenTimer_running:
            return "[" +
                   std::to_string(
                       BGPEvent::BGPOpen_with_DelayOpenTimer_running) +
                   "-BGP open with DelayOpenTimer running]";
        case BGPEvent::BGPHeaderErr:
            return "[" + std::to_string(BGPEvent::BGPHeaderErr) +
                   "-BGP header error]";
        case BGPEvent::BGPOpenMsgErr:
            return "[" + std::to_string(BGPEvent::BGPOpenMsgErr) +
                   "-BGPOpenMsg error]";
        case BGPEvent::OpenCollisionDump:
            return "[" + std::to_string(BGPEvent::OpenCollisionDump) +
                   "-Open collision dump]";
        case BGPEvent::NotifMsgVerErr:
            return "[" + std::to_string(BGPEvent::NotifMsgVerErr) +
                   "-NotifMsgVer error]";
        case BGPEvent::NotifMsg:
            return "[" + std::to_string(BGPEvent::NotifMsg) + "-NotifMsg]";
        case BGPEvent::KeepAliveMsg:
            return "[" + std::to_string(BGPEvent::KeepAliveMsg) +
                   "-KeepAliveMsg]";
        case BGPEvent::UpdateMsg:
            return "[" + std::to_string(BGPEvent::UpdateMsg) + "-UpdateMsg]";
        case BGPEvent::UpdateMsgErr:
            return "[" + std::to_string(event) + "-UpdateMsg error]";
        default:
            L_ERROR("BGPEvent", "THIS SHOULD NEVER HAPPEN: unknown BGP Event");
            return "[" + std::to_string(event) + "-Unknown]";
    }
}
