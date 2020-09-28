/*
 * author : Shuichi TAKANO
 * since  : Thu Sep 10 2020 04:03:30
 */
#ifndef _8E294522_A134_3DC8_3D88_C8E6E8578440
#define _8E294522_A134_3DC8_3D88_C8E6E8578440

#include <stdint.h>

namespace graphics
{

inline uint16_t
makeColor16(int r, int g, int b)
{
    return ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3);
}

} // namespace graphics

#endif /* _8E294522_A134_3DC8_3D88_C8E6E8578440 */
