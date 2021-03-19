#ifndef BGPSIMULATION_ENTITIES_LINK_H
#define BGPSIMULATION_ENTITIES_LINK_H

#include <string>

#include "Device.h"


using namespace std;

enum Connection_status { active, failed };

class Device;  // forward declaration
class Link {
   public:
    pair<Device *, Device *> devices;
    pair<string, string>     device_source_interfaces;
    Connection_status        connection_status;

    Link(pair<Device *, Device *> devices,
         pair<string, string>     device_source_interfaces,
         Connection_status        connection_status)
        : devices(devices),
          device_source_interfaces(device_source_interfaces),
          connection_status(connection_status) {}

    void disconnect(Device *device) {
        if (devices.first == device) {
            devices.first = nullptr;
        }  // do not "else if" this to account for the case in which
           // ID1==ID2
        if (devices.second == device) {
            devices.second = nullptr;
        }

        if (devices.first == nullptr && devices.second == nullptr) {
            delete this;
        }
    }

    Device *getPeerOrNull(Device *device) {
        if (devices.first == device) {
            return devices.second;
        } else if (devices.second == device) {
            return devices.first;
        } else {
            return nullptr;
        }
    }


   private:
    ~Link() = default;
};


#endif  // BGPSIMULATION_ENTITIES_LINK_H
