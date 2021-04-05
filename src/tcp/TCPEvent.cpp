#include "TCPEvent.h"

#include "../logger/Logger.h"


std::string getEventName(TCPEvent event) {
    switch (event) {
        case TCPEvent::PassiveOpen:
            return "[" + std::to_string((int)TCPEvent::PassiveOpen) +
                   "-PassiveOpen]";
        case TCPEvent::ActiveOpen_SendSYN:
            return "[" + std::to_string((int)TCPEvent::ActiveOpen_SendSYN) +
                   "-ActiveOpen_SendSYN]";
        case TCPEvent::ReceiveClientSYN_SendSYNACK:
            return "[" +
                   std::to_string((int)TCPEvent::ReceiveClientSYN_SendSYNACK) +
                   "-ReceiveClientSYN_SendSYNACK]";
        case TCPEvent::ReceiveSYN_SendACK:
            return "[" + std::to_string((int)TCPEvent::ReceiveSYN_SendACK) +
                   "-ReceiveSYN_SendACK]";
        case TCPEvent::ReceiveSYNACKSendACK:
            return "[" + std::to_string((int)TCPEvent::ReceiveSYNACKSendACK) +
                   "-ReceiveSYNACKSendACK]";
        case TCPEvent::ReceiveACK:
            return "[" + std::to_string((int)TCPEvent::ReceiveACK) +
                   "-ReceiveACK]";
        case TCPEvent::CloseSendFIN:
            return "[" + std::to_string((int)TCPEvent::CloseSendFIN) +
                   "-CloseSendFIN]";
        case TCPEvent::ReceiveFIN:
            return "[" + std::to_string((int)TCPEvent::ReceiveFIN) +
                   "-ReceiveFIN]";
        case TCPEvent::ReceiveACKforFIN:
            return "[" + std::to_string((int)TCPEvent::ReceiveACKforFIN) +
                   "-ReceiveACKforFIN]";
        case TCPEvent::ReceiveFINSendACK:
            return "[" + std::to_string((int)TCPEvent::ReceiveFINSendACK) +
                   "-ReceiveFINSendACK]";
        case TCPEvent::TimerExpiration:
            return "[" + std::to_string((int)TCPEvent::TimerExpiration) +
                   "-TimerExpiration]";

        default:
            L_ERROR("THIS SHOULD NEVER HAPPEN: unknown TCP Event");
            return "[" + std::to_string((int)event) + "-Unknown]";
    }
}