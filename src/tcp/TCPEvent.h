#ifndef TCPEVENT_H
#define TCPEVENT_H
#include <string>

#include "../logger/Logger.h"
enum TCPEvent {
    PassiveOpen                 = 1,
    ActiveOpen_SendSYN          = 2,
    ReceiveClientSYN_SendSYNACK = 3,
    ReceiveSYN_SendACK          = 4,
    ReceiveSYNACKSendACK        = 5,
    ReceiveACK                  = 6,
    CloseSendFIN                = 7,
    ReceiveFIN                  = 8,
    ReceiveACKforFIN            = 9,
    ReceiveFINSendACK           = 10,
    TimerExpiration             = 11


};

std::string getEventName(TCPEvent);
#endif