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
    static constexpr uint32_t SCREEN_WIDTH         = 320;
    static constexpr uint32_t SCREEN_HEIGHT        = 232;
    static constexpr uint32_t SCREEN_HEIGHT_ACTUAL = 240;
    static constexpr uint32_t TITLE_BAR_HEIGHT     = 12;
    static constexpr uint32_t SCROLL_BAR_WIDTH     = 4;

    uint32_t titleTextColor     = 0xffffff;
    uint32_t titleTextEdgeColor = 0x000000;
    uint32_t titleBarColor      = 0x707070;
    uint32_t bgColor            = 0x778899;
    uint32_t highlightedBGColor = 0xbc8f8f;
    uint32_t borderColor        = 0x505050;
    uint32_t frameColor         = 0x000000;
    uint32_t scrollBarBGColor   = 0x404040;
    uint32_t scrollBarColor     = 0xc0c0c0;
};

} // namespace ui

#endif /* F8C00411_B134_13F9_2F64_6754B60F001A */
