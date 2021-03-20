#include "NetworkCard.h"

#include <iostream>


void NetworkCard::connect(const shared_ptr<Link>& linkToConnect) {
    if (link == nullptr) {
        linkToConnect->connect(this);
        link = linkToConnect;
    } else {
        cout << "[-] (" << net_interface
             << ") This interface is already connected" << endl;  // TODO ERROR
    }
}

void NetworkCard::disconnect(const shared_ptr<Link>& linkToDisconnect) {
    if (link.get() == linkToDisconnect.get()) {
        linkToDisconnect->connect(this);
        link = nullptr;
    } else if (link == nullptr) {
        cout << "[-] (" << net_interface
             << ") This interface has no link connected" << endl;  // TODO ERROR
    } else {
        cout << "[-] (" << net_interface
             << ") This interface is not connected with the link specified"
             << endl;  // TODO ERROR
    }
}
