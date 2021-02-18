#ifndef BGPSIMULATION_ENTITIES_DEVICE_H
#define BGPSIMULATION_ENTITIES_DEVICE_H

#include <string>

using namespace std;

class Device {
   public:
    Device(string ID) : ID(std::move(ID)) {}

    string ID;
};


#endif  // BGPSIMULATION_ENTITIES_DEVICE_H
