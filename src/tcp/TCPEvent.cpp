#include "TCPEvent.h"

#include "../logger/Logger.h"


std::string getEventName(TCPEvent event) {
    switch (event) {
        case TCPEvent::OpenPassive:
            return "[" + std::to_string(event) + "-OpenPassive]";
        case TCPEvent::OpenActive:
            return "[" + std::to_string(event) + "-OpenActive]";
        case TCPEvent::Send:
            return "[" + std::to_string(event) + "-Send]";
        case TCPEvent::Receive:
            return "[" + std::to_string(event) + "-Receive]";
        case TCPEvent::Close:
            return "[" + std::to_string(event) + "-Close]";
        case TCPEvent::Abort:
            return "[" + std::to_string(event) + "-Abort]";
        case TCPEvent::Status:
            return "[" + std::to_string(event) + "-Status]";
        case TCPEvent::SegmentArrives:
            return "[" + std::to_string(event) + "-SegmentArrives]";
        case TCPEvent::UserTimeout:
            return "[" + std::to_string(event) + "-UserTimeout]";
        case TCPEvent::RetransmissionTimeout:
            return "[" + std::to_string(event) + "-RetransmissionTimeout]";
        case TCPEvent::TimeWaitTimeout:
            return "[" + std::to_string(event) + "-TimeWaitTimeout]";
        default:
            L_ERROR("TCPEvent",
                    "THIS SHOULD NEVER HAPPEN: unknown TCP Event " +
                        std::to_string(event));
            return "[" + std::to_string(event) + "-Unknown]";
    }
}
