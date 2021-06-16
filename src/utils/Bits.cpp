#include "Bits.h"

bool isFlag8Set(uint8_t flagsToTest, uint8_t flagValue) {
    return (flagsToTest & flagValue) == flagValue;
}
