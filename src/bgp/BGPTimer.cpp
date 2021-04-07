#include "BGPTimer.h"

BGPTimer ::BGPTimer(std::string          name,
                    BGPStateMachine*     stateMachine,
                    BGPEvent             eventToSendUponExpire,
                    std::chrono::seconds totalDuration)
    : Timer(name, stateMachine, eventToSendUponExpire, totalDuration) {}