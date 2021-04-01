#include "TCPEvent.h"

std::string getEventName(TCPEvent event) {
    switch (event) {
        case __TCP_SHUTDOWN:
            return "[" + std::to_string(__TCP_SHUTDOWN) + "-TCP_SHUTDOWN]";
        case PassiveOpen:
            return "[" + std::to_string(PassiveOpen) + "-PassiveOpen]";
        case ActiveOpen_SendSYN:
            return "[" + std::to_string(ActiveOpen_SendSYN) +
                   "-ActiveOpen_SendSYN]";
        case ReceiveClientSYN_SendSYNACK:
            return "[" + std::to_string(ReceiveClientSYN_SendSYNACK) +
                   "-ReceiveClientSYN_SendSYNACK]";
        case ReceiveSYN_SendACK:
            return "[" + std::to_string(ReceiveSYN_SendACK) +
                   "-ReceiveSYN_SendACK]";
        case ReceiveSYNACKSendACK:
            return "[" + std::to_string(ReceiveSYNACKSendACK) +
                   "-ReceiveSYNACKSendACK]";
        case ReceiveACK:
            return "[" + std::to_string(ReceiveACK) + "-ReceiveACK]";
        case CloseSendFIN:
            return "[" + std::to_string(CloseSendFIN) + "-CloseSendFIN]";
        case ReceiveFIN:
            return "[" + std::to_string(ReceiveFIN) + "-ReceiveFIN]";
        case ReceiveACKforFIN:
            return "[" + std::to_string(ReceiveACKforFIN) +
                   "-ReceiveACKforFIN]";
        case ReceiveFINSendACK:
            return "[" + std::to_string(ReceiveFINSendACK) +
                   "-ReceiveFINSendACK]";
        case TimerExpiration:
            return "[" + std::to_string(TimerExpiration) + "-TimerExpiration]";

        default:
            L_ERROR("THIS SHOULD NEVER HAPPEN");
            break;
    }
}