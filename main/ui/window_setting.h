/*
 * author : Shuichi TAKANO
 * since  : Mon Feb 04 2019 3:10:56
 */
#ifndef F8C00411_B134_13F9_2F64_6754B60F001A
#define F8C00411_B134_13F9_2F64_6754B60F001A

#include "types.h"

namespace ui
{

struct WindowSettings
{
    uint16_t titleBarHeight = 12;
    uint16_t scrollBarWidth = 4;

    uint32_t titleColor         = 0xffffff;
    uint32_t bgColor            = 0x778899;
    uint32_t frameColor         = 0x000000;
    uint32_t highlightedBGColor = 0xbc8f8f;
};

} // namespace ui

#endif /* F8C00411_B134_13F9_2F64_6754B60F001A */
