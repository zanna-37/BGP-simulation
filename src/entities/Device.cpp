#include "Device.h"

NetworkCard *Device::getNetworkCardByInterfaceOrNull(string interfaceToSearch) {
    for (const auto &networkCard : *networkCards) {
        if (networkCard->net_interface == interfaceToSearch) {
            return networkCard;
        }
    }
    return nullptr;
}
