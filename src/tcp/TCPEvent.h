#ifndef TCPEVENT_H
#define TCPEVENT_H
#include <string>

#include "../logger/Logger.h"
enum TCPEvent {
    __TCP_SHUTDOWN,
    PassiveOpen,
    ActiveOpen_SendSYN,
    ReceiveClientSYN_SendSYNACK,
    ReceiveSYN_SendACK,
    ReceiveSYNACKSendACK,
    ReceiveACK,
    CloseSendFIN,
    ReceiveFIN,
    ReceiveACKforFIN,
    ReceiveFINSendACK,
    TimerExpiration


};

std::string getEventName(TCPEvent);
#endif