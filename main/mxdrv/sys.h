/* -*- mode:C++; -*-
 *
 * author(s) : Shuichi TAKANO
 * since 2014/11/01(Sat) 15:40:36
 */
#ifndef _MXDRV_SYS_H
#define _MXDRV_SYS_H

#include <sound_sys/m6258.h>
#include <sound_sys/ym2151.h>

struct MXDRVSoundSystemSet
{
    sound_sys::YM2151* ym2151;
    sound_sys::M6258* m6258;

    MXDRVSoundSystemSet()
        : ym2151(0)
        , m6258(0)
    {
    }
};

extern MXDRVSoundSystemSet mxdrvSoundSystemSet_;

inline MXDRVSoundSystemSet&
getMXDRVSoundSystemSet()
{
    return mxdrvSoundSystemSet_;
}

#endif /* _MXDRV_SYS_H */
