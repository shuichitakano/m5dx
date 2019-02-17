/* -*- mode:C++; -*-
 *
 * author(s) : Shuichi TAKANO
 * since 2014/11/01(Sat) 15:40:36
 */
#ifndef _MXDRV_SYS_H
#define _MXDRV_SYS_H

#include <sound_sys/swpcm8.h>
#include <sound_sys/ym2151.h>

struct MXDRVSoundSystemSet
{
    sound_sys::YM2151* ym2151{};
    sound_sys::SWPCM8* m6258{};

    MXDRVSoundSystemSet() {}
};

extern MXDRVSoundSystemSet mxdrvSoundSystemSet_;

inline MXDRVSoundSystemSet&
getMXDRVSoundSystemSet()
{
    return mxdrvSoundSystemSet_;
}

#endif /* _MXDRV_SYS_H */
