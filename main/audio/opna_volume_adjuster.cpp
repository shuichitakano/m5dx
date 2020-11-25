/*
 * author : Shuichi TAKANO
 * since  : Sat Oct 17 2020 17:51:35
 */

#include "opna_volume_adjuster.h"
#include "../debug.h"
#include "opn_slot.h"
#include <algorithm>

namespace audio
{

void
OPNAVolumeAdjuster::setValue(int addr, int v)
{
    int a1 = addr >> 1;
    if ((addr & 1) == 0)
    {
        currentReg_[a1] = v;
    }
    else
    {
        int reg = currentReg_[a1];
        if (reg >= 0xb0 && reg <= 0xb2)
        {
            int ch             = reg - 0xb0;
            algorithm_[a1][ch] = v & 7;

            // todo: algorithm 7 の feedback を制御すべきでは？
        }
        else if ((reg & 0xf0) == 0x40)
        {
            int ch   = reg & 3;
            int slot = (reg >> 2) & 3;
            int cm   = audio::OPNSlot::getCarrierMask(algorithm_[a1][ch]);

            if (cm & (1 << slot))
            {
                // carrier
                v &= 127;
                v = std::max(0, std::min(127, v - adjFM_));
            }
        }
        else if (reg == 0x11 && a1 == 0)
        {
            v &= 63;
            v = std::max(0, std::min(63, v + adjRhythm_));
        }
    }
    base_.setValue(addr, v);
}

} // namespace audio
