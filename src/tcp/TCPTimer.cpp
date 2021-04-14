#include "TCPTimer.h"

TCPTimer ::TCPTimer(std::string          name,
                    TCPStateMachine*     stateMachine,
                    TCPEvent             eventToSendUponExpire,
                    std::chrono::seconds totalDuration)
    : Timer(name, stateMachine, eventToSendUponExpire, totalDuration) {}