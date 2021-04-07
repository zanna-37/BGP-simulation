#ifndef TCPEVENT_H
#define TCPEVENT_H
#include <string>

enum TCPEvent {
    PassiveOpen                 = 1,
    ActiveOpen_SendSYN          = 2,
    ReceiveClientSYN_SendSYNACK = 3,
    ReceiveSYN_SendACK          = 4,
    ReceiveSYNACKSendACK        = 5,
    ReceiveACK                  = 6,
    CloseSendFIN                = 7,
    ReceiveACKforFIN            = 8,
    ReceiveFINSendACK           = 9,
    TimerExpiration             = 10


};

std::string getEventName(TCPEvent);
#endif