#include "BGPEvent.h"

#include "../logger/Logger.h"

std::string getEventName(BGPEvent event) {
    switch (event.eventList) {
        case BGPEventList::ManualStart:
            return "[" + std::to_string(BGPEventList::ManualStart) +
                   "-Manual start]";
        case BGPEventList::ManualStop:
            return "[" + std::to_string(BGPEventList::ManualStop) +
                   "-Manual stop]";
        case BGPEventList::AutomaticStart:
            return "[" + std::to_string(BGPEventList::AutomaticStart) +
                   "-Automatic start]";
        case BGPEventList::ManualStart_with_PassiveTcpEstablishment:
            return "[" +
                   std::to_string(
                       BGPEventList::ManualStart_with_PassiveTcpEstablishment) +
                   "-Manual start with Passive Tcp Establishment]";
        case BGPEventList::AutomaticStart_with_PassiveTcpEstablishment:
            return "[" +
                   std::to_string(
                       BGPEventList::
                           AutomaticStart_with_PassiveTcpEstablishment) +
                   "-Automatic start with Passive Tcp Establishment]";
        case BGPEventList::AutomaticStart_with_DampPeerOscillations:
            return "[" +
                   std::to_string(
                       BGPEventList::AutomaticStart_with_DampPeerOscillations) +
                   "-Automatic start with Damp Peer Oscillations]";
        case BGPEventList::
            AutomaticStart_with_DampPeerOscillations_and_PassiveTcpEstablishment:
            return "[" +
                   std::to_string(
                       BGPEventList::
                           AutomaticStart_with_DampPeerOscillations_and_PassiveTcpEstablishment) +
                   "-Automatic start with Damp Peer Oscillations and Passive "
                   "Tcp Establishment]";
        case BGPEventList::AutomaticStop:
            return "[" + std::to_string(BGPEventList::AutomaticStop) +
                   "-Automatic stop]";
        case BGPEventList::ConnectRetryTimer_Expires:
            return "[" +
                   std::to_string(BGPEventList::ConnectRetryTimer_Expires) +
                   "-ConnectRetryTimer expires]";
        case BGPEventList::HoldTimer_Expires:
            return "[" + std::to_string(BGPEventList::HoldTimer_Expires) +
                   "-HoldTimer expires]";
        case BGPEventList::KeepaliveTimer_Expires:
            return "[" + std::to_string(BGPEventList::KeepaliveTimer_Expires) +
                   "-KeepaliveTimer expires]";
        case BGPEventList::DelayOpenTimer_Expires:
            return "[" + std::to_string(BGPEventList::DelayOpenTimer_Expires) +
                   "-DelayOpenTimer expires]";
        case BGPEventList::IdleHoldTimer_Expires:
            return "[" + std::to_string(BGPEventList::IdleHoldTimer_Expires) +
                   "-IdleHoldTimer expires]";
        case BGPEventList::TcpConnection_Valid:
            return "[" + std::to_string(BGPEventList::TcpConnection_Valid) +
                   "-Tcp connection valid]";
        case BGPEventList::Tcp_CR_Invalid:
            return "[" + std::to_string(BGPEventList::Tcp_CR_Invalid) +
                   "-Tcp CR Invalid]";
        case BGPEventList::Tcp_CR_Acked:
            return "[" + std::to_string(BGPEventList::Tcp_CR_Acked) +
                   "-Tcp CR Acked]";
        case BGPEventList::TcpConnectionConfirmed:
            return "[" + std::to_string(BGPEventList::TcpConnectionConfirmed) +
                   "-Tcp connection confirmed]";
        case BGPEventList::TcpConnectionFails:
            return "[" + std::to_string(BGPEventList::TcpConnectionFails) +
                   "-Tcp connection fails]";
        case BGPEventList::BGPOpen:
            return "[" + std::to_string(BGPEventList::BGPOpen) + "-BGP open]";
        case BGPEventList::BGPOpen_with_DelayOpenTimer_running:
            return "[" +
                   std::to_string(
                       BGPEventList::BGPOpen_with_DelayOpenTimer_running) +
                   "-BGP open with DelayOpenTimer running]";
        case BGPEventList::BGPHeaderErr:
            return "[" + std::to_string(BGPEventList::BGPHeaderErr) +
                   "-BGP header error]";
        case BGPEventList::BGPOpenMsgErr:
            return "[" + std::to_string(BGPEventList::BGPOpenMsgErr) +
                   "-BGPOpenMsg error]";
        case BGPEventList::OpenCollisionDump:
            return "[" + std::to_string(BGPEventList::OpenCollisionDump) +
                   "-Open collision dump]";
        case BGPEventList::NotifMsgVerErr:
            return "[" + std::to_string(BGPEventList::NotifMsgVerErr) +
                   "-NotifMsgVer error]";
        case BGPEventList::NotifMsg:
            return "[" + std::to_string(BGPEventList::NotifMsg) + "-NotifMsg]";
        case BGPEventList::KeepAliveMsg:
            return "[" + std::to_string(BGPEventList::KeepAliveMsg) +
                   "-KeepAliveMsg]";
        case BGPEventList::UpdateMsg:
            return "[" + std::to_string(BGPEventList::UpdateMsg) +
                   "-UpdateMsg]";
        case BGPEventList::UpdateMsgErr:
            return "[" + std::to_string(event.eventList) + "-UpdateMsg error]";
        default:
            L_ERROR("BGPEventList",
                    "THIS SHOULD NEVER HAPPEN: unknown BGP Event");
            return "[" + std::to_string(event.eventList) + "-Unknown]";
    }
}
