/*
 * author : Shuichi TAKANO
 * since  : Fri Nov 09 2018 2:4:11
 */

#ifndef _1F321D27_9133_F071_1F24_8F8A844D1FA3
#define _1F321D27_9133_F071_1F24_8F8A844D1FA3

#include "sound_chip.h"

namespace audio
{

SoundChipBase* allocateYM2151();
void freeYM2151(SoundChipBase* p);

} // namespace audio

#endif /* _1F321D27_9133_F071_1F24_8F8A844D1FA3 */
