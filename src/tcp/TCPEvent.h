#ifndef TCPEVENT_H
#define TCPEVENT_H

#include <string>
enum TCPEvent {
    __INTERNAL_SHUTDOWN,
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