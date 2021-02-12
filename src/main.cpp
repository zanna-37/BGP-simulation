#include <iostream>
#include "tins/tins.h"
// using namespace std;
using namespace Tins;
int main()
{
    std::cout << "Hello, World!" << std::endl;

    IP packet = IP("192.168.0.1") / TCP(22,1234);
    packet.rfind_pdu<TCP>().set_flag(TCP::SYN,1);
    TCP &tcp = packet.rfind_pdu<TCP>();
    // Check if the SYN Flag is set
    if(tcp.get_flag(TCP::SYN)){
        std::cout << "SYN flag set" << std::endl;
    }else{
        std::cout << "SYN flag unset" << std::endl;
    }

    return 0;
}
