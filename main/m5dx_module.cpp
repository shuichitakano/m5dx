/*
 * author : Shuichi TAKANO
 * since  : Sun Mar 03 2019 0:54:50
 */

#include "m5dx_module.h"
#include "debug.h"
#include "target.h"
#include <array>
#include <stdint.h>
#include <wire.h>

namespace M5DX
{

namespace
{

using ModuleUUID = std::array<uint8_t, 16>;

constexpr ModuleUUID UUID_YM2151 = {
    0x9f,
    0x84,
    0xe4,
    0x1e,
    0xd3,
    0x09,
    0x4b,
    0xd9,
    0x9c,
    0x28,
    0x71,
    0xa3,
    0x68,
    0xde,
    0xd0,
    0x60,
};

constexpr ModuleUUID UUID_YMF288 = {
    0xf9,
    0xd6,
    0x99,
    0x44,
    0x43,
    0x70,
    0x46,
    0xd1,
    0x9b,
    0xec,
    0xac,
    0xa5,
    0x81,
    0x85,
    0x76,
    0x6a,
};

static constexpr int I2CAddrEEPROM = 0b1010011;
static uint8_t uuidAddr            = 0;

} // namespace

ModuleID
readModuleID()
{
    target::startI2C();

    auto r = [] {
        Wire.beginTransmission(I2CAddrEEPROM);
        Wire.write(uuidAddr);
        if (auto r = Wire.endTransmission(false))
        {
            DBOUT(("EEPROM read address write error %d\n", r));
            return ModuleID::UNKNOWN;
        }
        auto byteReceived = Wire.requestFrom(I2CAddrEEPROM, 16);
        if (byteReceived != 16)
        {
            DBOUT(("EEPROM read error %d", byteReceived));
            return ModuleID::UNKNOWN;
        }

        DBOUT(("module uuid: "));
        ModuleUUID uuid;
        for (int i = 0; i < 16; ++i)
        {
            uint8_t v = Wire.read();
            uuid[i]   = v;
            DBOUT(("%02x ", v));
        }
        DBOUT(("\n"));

        if (uuid == UUID_YM2151)
        {
            DBOUT(("YM2151 module found.\n"));
            return ModuleID::YM2151;
        }
        else if (uuid == UUID_YMF288)
        {
            DBOUT(("YMF288 module found.\n"));
            return ModuleID::YMF288;
        }
        DBOUT(("Unknown module.\n"));
        return ModuleID::UNKNOWN;
    }();

    target::endI2C();
    return r;
}

bool
writeModuleID(ModuleID id)
{
    auto write = [](const ModuleUUID& uuid) {
        target::startI2C();

        Wire.beginTransmission(I2CAddrEEPROM);
        Wire.write(uuidAddr);

        for (auto v : uuid)
        {
            Wire.write(v);
        }
        auto r = Wire.endTransmission();
        if (r)
        {
            DBOUT(("EEPROM write error %d\n", r));
        }

        target::endI2C();
        return r == 0;
    };

    switch (id)
    {
    default:
        return false;

    case ModuleID::YM2151:
        return write(UUID_YM2151);

    case ModuleID::YMF288:
        return write(UUID_YMF288);
    };
}

} // namespace M5DX
