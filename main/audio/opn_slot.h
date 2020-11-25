/*
 * author : Shuichi TAKANO
 * since  : Sat Oct 17 2020 18:13:44
 */
#ifndef _85E56251_0134_3E2E_114B_2C4743D5DCA0
#define _85E56251_0134_3E2E_114B_2C4743D5DCA0

namespace audio
{

struct OPNSlot
{
    enum
    {
        SLOT1 = 0,
        SLOT2 = 2,
        SLOT3 = 1,
        SLOT4 = 3,
    };

    static constexpr int getCarrierMask(int algorithm)
    {
        constexpr int masks[] = {
            (1 << SLOT4),
            (1 << SLOT4),
            (1 << SLOT4),
            (1 << SLOT4),
            (1 << SLOT2) | (1 << SLOT4),
            (1 << SLOT2) | (1 << SLOT3) | (1 << SLOT4),
            (1 << SLOT2) | (1 << SLOT3) | (1 << SLOT4),
            (1 << SLOT1) | (1 << SLOT2) | (1 << SLOT3) | (1 << SLOT4),
        };
        return masks[algorithm & 7];
    }
};

} // namespace audio

#endif /* _85E56251_0134_3E2E_114B_2C4743D5DCA0 */
