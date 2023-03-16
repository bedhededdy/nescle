#pragma once

#include "MapperBase.h"

class Mapper004 : public MapperBase {
private:

public:
    Mapper004(Cart* cart);

    uint8_t MapCPURead(uint16_t addr) override;
    bool MapCPUWrite(uint16_t addr, uint8_t data) override;
    uint8_t MapPPURead(uint16_t addr) override;
    bool MapPPUWrite(uint16_t addr, uint8_t data) override;

    void SaveToDisk(FILE* file) override;
    void LoadFromDisk(FILE* file) override;
};
