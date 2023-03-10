#include "Mapper001.h"

Mapper001::Mapper001(Cart* cart) {
    id = 1;
    this->cart = cart;
}

uint8_t Mapper001::MapCPURead(uint16_t addr) {
    return 0;
}

bool Mapper001::MapCPUWrite(uint16_t addr, uint8_t data) {
    return false;
}

uint8_t Mapper001::MapPPURead(uint16_t addr) {
    return 0;
}

bool Mapper001::MapPPUWrite(uint16_t addr, uint8_t data) {
    return false;
}
