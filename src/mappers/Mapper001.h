#pragma once

#include "MapperBase.h"
#include <vector>

class Mapper001 : public MapperBase {
private:
    uint8_t chr_select4_lo = 0;
    uint8_t chr_select4_hi = 0;
    uint8_t chr_select8 = 0;

    uint8_t prg_select16_lo = 0;
    uint8_t prg_select16_hi = 0;
    uint8_t prg_select32 = 0;

    uint8_t load = 0;
    // Write count
    uint8_t load_reg_ct = 0;
    uint8_t ctrl = 0;

    std::vector<uint8_t> sram;



    // OLC has mirroring, but we handle that
    // from the cartridge, so we need no var

    // OLC has some vram vector, but I don't think we need it unless
    // we want to be able to save games to the vram backup

    void reset();

public:
    Mapper001(Cart* cart);

    uint8_t MapCPURead(uint16_t addr) override;
    bool MapCPUWrite(uint16_t addr, uint8_t data) override;
    uint8_t MapPPURead(uint16_t addr) override;
    bool MapPPUWrite(uint16_t addr, uint8_t data) override;

    void SaveToDisk(FILE* file) override;
    void LoadFromDisk(FILE* file) override;
};
