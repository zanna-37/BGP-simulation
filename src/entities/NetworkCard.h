#ifndef BGPSIMULATION_ENTITIES_NET_DETAILS_H
#define BGPSIMULATION_ENTITIES_NET_DETAILS_H

#include <memory>
#include <string>

#include "Link.h"

using namespace std;


class NetworkCard {
   public:
    string           net_interface;
    string           IP;
    string           netmask;
    string           default_gateway;
    shared_ptr<Link> link;

    NetworkCard(string net_interface,
                string IP,
                string netmask,
                string default_gateway)
        : net_interface(std::move(net_interface)),
          IP(std::move(IP)),
          netmask(std::move(netmask)),
          default_gateway(std::move(default_gateway)) {}

    ~NetworkCard() { link = nullptr; }


    void connect(shared_ptr<Link> linkToConnect) {
        if (link.get() == nullptr) {
            linkToConnect->connect(this);
            link = linkToConnect;
        } else {
            cout << "[-] (" << net_interface
                 << ") This interface is already connected"
                 << endl;  // TODO ERROR
        }
    }

    void disconnect(shared_ptr<Link> linkToDisconnect) {
        if (link.get() == linkToDisconnect.get()) {
            linkToDisconnect->connect(this);
            link = nullptr;
        } else if (link.get() == nullptr) {
            cout << "[-] (" << net_interface
                 << ") This interface has no link connected"
                 << endl;  // TODO ERROR
        } else {
            cout << "[-] (" << net_interface
                 << ") This interface is not connected with the link specified"
                 << endl;  // TODO ERROR
        }
    }
};

#endif  // BGPSIMULATION_ENTITIES_NET_DETAILS_H
