#pragma once

#include "MapperBase.h"

class Mapper066 : public MapperBase {
private:
    uint8_t bank_select;

public:
    Mapper066(Cart* cart);

    uint8_t MapCPURead(uint16_t addr) override;
    bool MapCPUWrite(uint16_t addr, uint8_t data) override;
    uint8_t MapPPURead(uint16_t addr) override;
    bool MapPPUWrite(uint16_t addr, uint8_t data) override;
};
