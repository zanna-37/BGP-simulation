#include "TCPEvent.h"

#include "../logger/Logger.h"


std::string getEventName(TCPEvent event) {
    switch (event) {
        case TCPEvent::PassiveOpen:
            return "[" + std::to_string(TCPEvent::PassiveOpen) +
                   "-PassiveOpen]";
        case TCPEvent::ActiveOpen_SendSYN:
            return "[" + std::to_string(TCPEvent::ActiveOpen_SendSYN) +
                   "-ActiveOpen_SendSYN]";
        case TCPEvent::ReceiveClientSYN_SendSYNACK:
            return "[" + std::to_string(TCPEvent::ReceiveClientSYN_SendSYNACK) +
                   "-ReceiveClientSYN_SendSYNACK]";
        case TCPEvent::ReceiveSYN_SendACK:
            return "[" + std::to_string(TCPEvent::ReceiveSYN_SendACK) +
                   "-ReceiveSYN_SendACK]";
        case TCPEvent::ReceiveSYNACKSendACK:
            return "[" + std::to_string(TCPEvent::ReceiveSYNACKSendACK) +
                   "-ReceiveSYNACKSendACK]";
        case TCPEvent::ReceiveACK:
            return "[" + std::to_string(TCPEvent::ReceiveACK) + "-ReceiveACK]";
        case TCPEvent::CloseSendFIN:
            return "[" + std::to_string(TCPEvent::CloseSendFIN) +
                   "-CloseSendFIN]";
        case TCPEvent::ReceiveACKforFIN:
            return "[" + std::to_string(TCPEvent::ReceiveACKforFIN) +
                   "-ReceiveACKforFIN]";
        case TCPEvent::ReceiveFINSendACK:
            return "[" + std::to_string(TCPEvent::ReceiveFINSendACK) +
                   "-ReceiveFINSendACK]";
        case TCPEvent::TimerExpiration:
            return "[" + std::to_string(TCPEvent::TimerExpiration) +
                   "-TimerExpiration]";
        case TCPEvent::SendRST:
            return "[" + std::to_string(TCPEvent::SendRST) + "-SendRST]";
        case TCPEvent::ReceiveRST:
            return "[" + std::to_string(TCPEvent::ReceiveRST) + "-ReceiveRST]";

        default:
            L_ERROR("TCPEvent", "THIS SHOULD NEVER HAPPEN: unknown TCP Event");
            return "[" + std::to_string(event) + "-Unknown]";
    }
}