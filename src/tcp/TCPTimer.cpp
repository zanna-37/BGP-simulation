#include "TCPTimer.h"

#include <utility>

#include "../entities/Device.h"
#include "../utils/Timer.h"
#include "TCPConnection.h"
#include "fsm/TCPStateMachine.h"


TCPTimer ::TCPTimer(std::string          name,
                    TCPStateMachine*     stateMachine,
                    TCPEvent             eventToSendUponExpire,
                    std::chrono::seconds totalDuration)
    : Timer(
          std::move(name), stateMachine, eventToSendUponExpire, totalDuration) {
}
