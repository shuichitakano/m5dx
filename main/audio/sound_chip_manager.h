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

SoundChipBase* allocateYMF288();
void freeYMF288(SoundChipBase* p);

void setYMF288FMVolume(int adj);
void setYMF288RhythmVolume(int adj);

void resetSoundChip();

void attachYM2151();
void atatchYMF288();
void detachAllChip();

} // namespace audio

#endif /* _1F321D27_9133_F071_1F24_8F8A844D1FA3 */
