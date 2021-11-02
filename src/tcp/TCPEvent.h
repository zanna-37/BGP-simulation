#ifndef TCPEVENT_H
#define TCPEVENT_H
#include <string>

/**
 * A list og the possible event that are handled by the TCPStateMachine,
 * according to RFC 793
 */
enum TCPEvent {
    // User Calls
    OpenPassive,
    OpenActive,
    Send,
    Receive,
    Close,
    Abort,
    Status,

    // Arriving Segments
    SegmentArrives,

    // Timeouts
    UserTimeout,
    RetransmissionTimeout,
    TimeWaitTimeout
};

/**
 * Print the event name
 * @param event the TCPEvent
 * @return a string name of the event
 */
std::string getEventName(TCPEvent);
#endif
