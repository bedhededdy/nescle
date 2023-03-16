#include "Mapper004.h"

Mapper004::Mapper004(Cart* cart) {
    id = 4;
    this->cart = cart;
}

uint8_t Mapper004::MapCPURead(uint16_t addr) {
    return 0;
}

bool Mapper004::MapCPUWrite(uint16_t addr, uint8_t data) {
    return false;
}

uint8_t Mapper004::MapPPURead(uint16_t addr) {
    return 0;
}

bool Mapper004::MapPPUWrite(uint16_t addr, uint8_t data) {
    return false;
}

void Mapper004::SaveToDisk(FILE* file) {
}

void Mapper004::LoadFromDisk(FILE* file) {
}
