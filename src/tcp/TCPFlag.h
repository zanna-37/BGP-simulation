#ifndef BGPSIMULATION_TCP_TCPFLAG_H
#define BGPSIMULATION_TCP_TCPFLAG_H

/**
 * A list of the TCP flags implemented in this project, with the correct value
 * to fill a \a uint8_t
 */
enum TCPFlag {

    FIN = 1,
    SYN = 2,
    RST = 4,
    PSH = 8,
    ACK = 16
};

#endif  // BGPSIMULATION_TCP_TCPFLAG_H
