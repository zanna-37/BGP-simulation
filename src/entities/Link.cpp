#include "Link.h"

#include <iostream>

void Link::disconnect(NetworkCard *networkCard) {
    if (device_source_networkCards.first == networkCard) {
        device_source_networkCards.first = nullptr;
    } else if (device_source_networkCards.second == networkCard) {
        device_source_networkCards.second = nullptr;
    } else {
        cout << "[-] This link is not connected to the specified networkCard"
             << endl;  // TODO ERROR
    }
}

void Link::connect(NetworkCard *networkCard) {
    if (device_source_networkCards.first == nullptr) {
        device_source_networkCards.first = networkCard;
    } else if (device_source_networkCards.second == nullptr) {
        device_source_networkCards.second = networkCard;
    } else {
        cout << "[-] This link is already fully connected"
             << endl;  // TODO ERROR
    }
}

NetworkCard *Link::getPeerNetworkCardOrNull(NetworkCard *networkCard) {
    if (device_source_networkCards.first == networkCard) {
        return device_source_networkCards.second;
    } else if (device_source_networkCards.second == networkCard) {
        return device_source_networkCards.first;
    } else {
        return nullptr;
    }
}
