#include "Device.h"

NetworkCard *Device::getNetworkCardByInterfaceOrNull(
    const string &interfaceToSearch) {
    for (const auto &networkCard : *networkCards) {
        if (networkCard->netInterface == interfaceToSearch) {
            return networkCard;
        }
    }
    return nullptr;
}
