#ifndef TCPEVENT_H
#define TCPEVENT_H
#include <string>

/**
 * A list og the possible event that are handled by the TCPStateMachine,
 * according to RFC 793
 */
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
    TimerExpiration             = 10,
    SendRST                     = 11,
    ReceiveRST                  = 12


};
/**
 * Print the event name
 * @param event the TCPEvent
 * @return a string name of the event
 */
std::string getEventName(TCPEvent);
#endif