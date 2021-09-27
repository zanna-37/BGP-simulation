#include "BGPTimer.h"

#include <utility>

#include "../entities/Router.h"
#include "../utils/Timer.h"
#include "BGPConnection.h"
#include "fsm/BGPStateMachine.h"


BGPTimer ::BGPTimer(std::string          name,
                    BGPStateMachine*     stateMachine,
                    BGPEvent             eventToSendUponExpire,
                    std::chrono::seconds totalDuration)
    : Timer(std::move(name),
            stateMachine,
            std::move(eventToSendUponExpire),
            totalDuration) {}
