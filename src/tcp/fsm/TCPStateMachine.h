#ifndef TCPSTATEMACHINE_H
#define TCPSTATEMACHINE_H

#include <chrono>

#include "../../fsm/StateMachine.h"
#include "../TCPConnection.h"
#include "../TCPEvent.h"
#include "../TCPTimer.h"
#include "../fsm/TCPState.h"

// forward declarations
#include "../TCPConnection.fwd.h"
#include "../TCPTimer.fwd.h"
#include "TCPState.fwd.h"

/**
 *\verbatim
                              +---------+ ---------\      active OPEN
                          |  CLOSED |            \    -----------
                          +---------+<---------\   \   create TCB
                            |     ^              \   \  snd SYN
               passive OPEN |     |   CLOSE        \   \
               ------------ |     | ----------       \   \
                create TCB  |     | delete TCB         \   \
                            V     |                      \   \
                          +---------+            CLOSE    |    \
                          |  LISTEN |          ---------- |     |
                          +---------+          delete TCB |     |
               rcv SYN      |     |     SEND              |     |
              -----------   |     |    -------            |     V
+---------+      snd SYN,ACK  /       \   snd SYN          +---------+
|         |<-----------------           ------------------>|         |
|   SYN   |                    rcv SYN                     |   SYN   |
|   RCVD  |<-----------------------------------------------|   SENT  |
|         |                    snd ACK                     |         |
|         |------------------           -------------------|         |
+---------+   rcv ACK of SYN  \       /  rcv SYN,ACK       +---------+
|           --------------   |     |   -----------
|                  x         |     |     snd ACK
|                            V     V
|  CLOSE                   +---------+
| -------                  |  ESTAB  |
| snd FIN                  +---------+
|                   CLOSE    |     |    rcv FIN
V                  -------   |     |    -------
+---------+          snd FIN  /       \   snd ACK          +---------+
|  FIN    |<-----------------           ------------------>|  CLOSE  |
| WAIT-1  |------------------                              |   WAIT  |
+---------+          rcv FIN  \                            +---------+
| rcv ACK of FIN   -------   |                            CLOSE  |
| --------------   snd ACK   |                           ------- |
V        x                   V                           snd FIN V
+---------+                  +---------+                   +---------+
|FINWAIT-2|                  | CLOSING |                   | LAST-ACK|
+---------+                  +---------+                   +---------+
|                rcv ACK of FIN |                 rcv ACK of FIN |
|  rcv FIN       -------------- |    Timeout=2MSL -------------- |
|  -------              x       V    ------------        x       V
\ snd ACK                 +---------+delete TCB         +---------+
 ------------------------>|TIME WAIT|------------------>| CLOSED  |
                          +---------+                   +---------+
 *\endverbatim
 */
class TCPStateMachine : public StateMachine<TCPConnection, TCPState, TCPEvent> {
   public:
    /**
     * The connection uses this timer to wait before passing to the CLOSED state
     */
    TCPTimer* timeWaitTimer = nullptr;

    /**
     * The default value of the timeWaitTimer
     */
    std::chrono::seconds timeWaitTime = 2s;

    TCPStateMachine(TCPConnection* connection);

    ~TCPStateMachine();

    /**
     * Initialize all the timers
     */
    void initializeTimers();
    /**
     * Resert the TimeWaitTimer deleting the old one and creating a new pointer
     */
    void resetTimeWaitTimer();
};
#endif