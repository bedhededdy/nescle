#pragma once

#include "../Cart.h"

class MapperBase {
protected:
    uint8_t id;
    Cart *cart;

public:
    virtual ~MapperBase() = default;

    void SetCart(Cart *cart) { this->cart = cart; }

    virtual uint8_t MapCPURead(uint16_t addr) = 0;
    virtual bool MapCPUWrite(uint16_t addr, uint8_t data) = 0;
    virtual uint8_t MapPPURead(uint16_t addr) = 0;
    virtual bool MapPPUWrite(uint16_t addr, uint8_t data) = 0;
};
