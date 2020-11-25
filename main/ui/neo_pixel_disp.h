/*
 * author : Shuichi TAKANO
 * since  : Thu Oct 22 2020 02:50:09
 */
#ifndef _99392E8B_0134_3E2C_2626_D5F99B8804F8
#define _99392E8B_0134_3E2C_2626_D5F99B8804F8

#include <array>
#include <stdint.h>

namespace ui
{

class NeoPixelDisp
{

public:
    NeoPixelDisp();

    void update(const std::array<float, 2>& lv,
                const int16_t* wave,
                int n,
                float dt);

private:
    struct LevelMeter
    {
        void update(const std::array<float, 2>& lv);
    };

    struct SpectrumMeter
    {
        float phase_ = 0;
        void update(const std::array<float, 2>& lv,
                    const int16_t* wave,
                    int n,
                    float dt);
    };

    struct Gaming
    {
        std::array<float, 2> phase_{};
        std::array<float, 2> prevValue_{};

        void update(const std::array<float, 2>& lv,
                    const int16_t* wave,
                    int n,
                    float dt);
    };

    LevelMeter lvMeter_;
    SpectrumMeter spectrumMeter_;
    Gaming gaming_;
};

} // namespace ui

#endif /* _99392E8B_0134_3E2C_2626_D5F99B8804F8 */
