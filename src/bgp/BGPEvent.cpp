#include "BGPEvent.h"

#include "../logger/Logger.h"

std::string getEventName(BGPEvent &event) {
    switch (event.eventType) {
        case BGPEventType::ManualStart:
            return "[" + std::to_string(BGPEventType::ManualStart) +
                   "-Manual start]";
        case BGPEventType::ManualStop:
            return "[" + std::to_string(BGPEventType::ManualStop) +
                   "-Manual stop]";
        case BGPEventType::AutomaticStart:
            return "[" + std::to_string(BGPEventType::AutomaticStart) +
                   "-Automatic start]";
        case BGPEventType::ManualStart_with_PassiveTcpEstablishment:
            return "[" +
                   std::to_string(
                       BGPEventType::ManualStart_with_PassiveTcpEstablishment) +
                   "-Manual start with Passive Tcp Establishment]";
        case BGPEventType::AutomaticStart_with_PassiveTcpEstablishment:
            return "[" +
                   std::to_string(
                       BGPEventType::
                           AutomaticStart_with_PassiveTcpEstablishment) +
                   "-Automatic start with Passive Tcp Establishment]";
        case BGPEventType::AutomaticStart_with_DampPeerOscillations:
            return "[" +
                   std::to_string(
                       BGPEventType::AutomaticStart_with_DampPeerOscillations) +
                   "-Automatic start with Damp Peer Oscillations]";
        case BGPEventType::
            AutomaticStart_with_DampPeerOscillations_and_PassiveTcpEstablishment:
            return "[" +
                   std::to_string(
                       BGPEventType::
                           AutomaticStart_with_DampPeerOscillations_and_PassiveTcpEstablishment) +
                   "-Automatic start with Damp Peer Oscillations and Passive "
                   "Tcp Establishment]";
        case BGPEventType::AutomaticStop:
            return "[" + std::to_string(BGPEventType::AutomaticStop) +
                   "-Automatic stop]";
        case BGPEventType::ConnectRetryTimer_Expires:
            return "[" +
                   std::to_string(BGPEventType::ConnectRetryTimer_Expires) +
                   "-ConnectRetryTimer expires]";
        case BGPEventType::HoldTimer_Expires:
            return "[" + std::to_string(BGPEventType::HoldTimer_Expires) +
                   "-HoldTimer expires]";
        case BGPEventType::KeepaliveTimer_Expires:
            return "[" + std::to_string(BGPEventType::KeepaliveTimer_Expires) +
                   "-KeepaliveTimer expires]";
        case BGPEventType::DelayOpenTimer_Expires:
            return "[" + std::to_string(BGPEventType::DelayOpenTimer_Expires) +
                   "-DelayOpenTimer expires]";
        case BGPEventType::IdleHoldTimer_Expires:
            return "[" + std::to_string(BGPEventType::IdleHoldTimer_Expires) +
                   "-IdleHoldTimer expires]";
        case BGPEventType::TcpConnection_Valid:
            return "[" + std::to_string(BGPEventType::TcpConnection_Valid) +
                   "-Tcp connection valid]";
        case BGPEventType::Tcp_CR_Invalid:
            return "[" + std::to_string(BGPEventType::Tcp_CR_Invalid) +
                   "-Tcp CR Invalid]";
        case BGPEventType::Tcp_CR_Acked:
            return "[" + std::to_string(BGPEventType::Tcp_CR_Acked) +
                   "-Tcp CR Acked]";
        case BGPEventType::TcpConnectionConfirmed:
            return "[" + std::to_string(BGPEventType::TcpConnectionConfirmed) +
                   "-Tcp connection confirmed]";
        case BGPEventType::TcpConnectionFails:
            return "[" + std::to_string(BGPEventType::TcpConnectionFails) +
                   "-Tcp connection fails]";
        case BGPEventType::BGPOpen:
            return "[" + std::to_string(BGPEventType::BGPOpen) + "-BGP open]";
        case BGPEventType::BGPOpen_with_DelayOpenTimer_running:
            return "[" +
                   std::to_string(
                       BGPEventType::BGPOpen_with_DelayOpenTimer_running) +
                   "-BGP open with DelayOpenTimer running]";
        case BGPEventType::BGPHeaderErr:
            return "[" + std::to_string(BGPEventType::BGPHeaderErr) +
                   "-BGP header error]";
        case BGPEventType::BGPOpenMsgErr:
            return "[" + std::to_string(BGPEventType::BGPOpenMsgErr) +
                   "-BGPOpenMsg error]";
        case BGPEventType::OpenCollisionDump:
            return "[" + std::to_string(BGPEventType::OpenCollisionDump) +
                   "-Open collision dump]";
        case BGPEventType::NotifMsgVerErr:
            return "[" + std::to_string(BGPEventType::NotifMsgVerErr) +
                   "-NotifMsgVer error]";
        case BGPEventType::NotifMsg:
            return "[" + std::to_string(BGPEventType::NotifMsg) + "-NotifMsg]";
        case BGPEventType::KeepAliveMsg:
            return "[" + std::to_string(BGPEventType::KeepAliveMsg) +
                   "-KeepAliveMsg]";
        case BGPEventType::UpdateMsg:
            return "[" + std::to_string(BGPEventType::UpdateMsg) +
                   "-UpdateMsg]";
        case BGPEventType::UpdateMsgErr:
            return "[" + std::to_string(event.eventType) + "-UpdateMsg error]";
        default:
            L_ERROR("BGPEventType",
                    "THIS SHOULD NEVER HAPPEN: unknown BGP Event");
            return "[" + std::to_string(event.eventType) + "-Unknown]";
    }
}
