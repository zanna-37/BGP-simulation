#ifndef ASPATH_H
#define ASPATH_H

#include <string>
#include <vector>

class ASPath {
   public:
    std::vector<std::string> autonomousSystems;


    ASPath() {}

    ~ASPath() {}

    void add(std::string);
};
#endif
