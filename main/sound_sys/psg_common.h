/* -*- mode:C++; -*-
 *
 * author(s) : Shuichi TAKANO
 * since 2015/05/22(Fri) 01:34:11
 */
#ifndef F3415A9F_8134_3E2E_240F_9477BE19ED50
#define F3415A9F_8134_3E2E_240F_9477BE19ED50

#include <stdint.h>

namespace sound_sys
{

struct PSGState
{
    uint16_t chNonMute_;
    uint16_t chEnabled_;
    uint16_t chKeyOn_;
    uint16_t chKeyOnTrigger_;

    float fBase_;

public:
    PSGState();
    float getNote(int ch, const uint8_t* regCache) const;
    float getVolume(int ch, const uint8_t* regCache) const;
    bool mute(int ch, bool f);
    uint32_t getKeyOnTrigger();
    uint32_t getEnabledChannels() const;
    bool update(int reg, int& v, const uint8_t* regCache);
    void setClock(int clock);
};

} // namespace sound_sys

#endif /* F3415A9F_8134_3E2E_240F_9477BE19ED50 */
