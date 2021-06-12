#include "MacGenerator.h"

#include <cstdlib>
#include <string>

#include "MacAddress.h"


void macGenerator(pcpp::MacAddress& macAddress) {
    std::string mac_str;
    for (int i = 0; i < 6; i++) {
        int tp = rand() % 256;
        mac_str.append(tp < 16 ? "0" : "");
        mac_str.append(std::to_string(tp));
        mac_str.append(i < 5 ? ":" : "");
    }
    macAddress = pcpp::MacAddress(mac_str);
}
