/*
 * author : Shuichi TAKANO
 * since  : Thu Oct 22 2020 02:51:50
 */

#include "neo_pixel_disp.h"
#include "target.h"
#include <algorithm>
#include <array>
#include <math.h>
#include <system/util.h>
#include <ui/system_setting.h>

#include <esp32-hal.h>

namespace ui
{

namespace
{

struct NeoPixel
{
    static constexpr size_t N = 10;
    std::array<uint8_t, N * 3> pixels_{};

    int brightness_    = 256;
    uint32_t prevTime_ = sys::micros();

public:
    __attribute__((noinline)) void IRAM_ATTR _show() const
    {
        static constexpr int t0h    = F_CPU / 2500000; // 0.4us
        static constexpr int t1h    = F_CPU / 1250000; // 0.8us
        static constexpr int tCycle = F_CPU / 800000;  // 1.25us

        const uint8_t* p    = pixels_.data();
        const uint8_t* pEnd = p + pixels_.size();

        auto pcc = sys::getCycleCount() - tCycle;

        for (; p != pEnd; ++p)
        {
            auto v    = *p;
            auto mask = 0x80;
            do
            {
                auto wait = [&](auto t) __attribute__((always_inline))
                {
                    while (1)
                    {
                        auto cc = sys::getCycleCount();
                        if (cc - pcc > t)
                        {
                            return cc;
                        }
                    }
                };

                pcc = wait(tCycle);

                GPIO.out_w1ts = 1 << target::config::NEO_PIXEL_DATA;
                auto th       = v & mask ? t1h : t0h;
                wait(th);

                GPIO.out_w1tc = 1 << target::config::NEO_PIXEL_DATA;

                mask >>= 1;
            } while (mask);
        }

        // 最後の wait は reset 期間に含める
    }

    void show()
    {
        static constexpr int tReset = 300; // us
        if (sys::micros() - prevTime_ < tReset)
        {
            return;
        }

        target::lockBus();
        portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
        portENTER_CRITICAL(&mux);

        _show();
        prevTime_ = sys::micros();

        portEXIT_CRITICAL(&mux);
        target::unlockBus();
    }

    void setPixel(int i, int r, int g, int b)
    {
        static constexpr int R_IDX = 1;
        static constexpr int G_IDX = 0;
        static constexpr int B_IDX = 2;

        auto* p  = &pixels_[i * 3];
        p[R_IDX] = r * brightness_ >> 8;
        p[G_IDX] = g * brightness_ >> 8;
        p[B_IDX] = b * brightness_ >> 8;
    }

    void setBrightness(int v) { brightness_ = v; }

    void clear()
    {
        if (brightness_)
        {
            brightness_ = 0;
            show();
        }
    }
};

NeoPixel pixels_;

std::array<float, 3>
hsv2rgb(float h, float s, float v)
{
    float r = v;
    float g = v;
    float b = v;
    if (s > 0.0f)
    {
        h *= 6.0f;
        int i   = (int)h;
        float f = h - (float)i;
        switch (i)
        {
        default:
        case 0:
            g *= 1 - s * (1 - f);
            b *= 1 - s;
            break;

        case 1:
            r *= 1 - s * f;
            b *= 1 - s;
            break;

        case 2:
            r *= 1 - s;
            b *= 1 - s * (1 - f);
            break;

        case 3:
            r *= 1 - s;
            g *= 1 - s * f;
            break;

        case 4:
            r *= 1 - s * (1 - f);
            g *= 1 - s;
            break;

        case 5:
            g *= 1 - s;
            b *= 1 - s * f;
            break;
        }
    }
    return {r, g, b};
}

} // namespace

NeoPixelDisp::NeoPixelDisp()
{
    pixels_.setBrightness(16);
    pixels_.show();
}

void
NeoPixelDisp::update(const std::array<float, 2>& lv,
                     const int16_t* wave,
                     int n,
                     float dt)
{
    auto mode = ui::SystemSettings::instance().getNeoPixelMode();
    if (mode == ui::NeoPixelMode::OFF)
    {
        pixels_.clear();
    }
    else
    {
        pixels_.setBrightness(
            ui::SystemSettings::instance().getNeoPixelBrightness() * 256 / 100);

        switch (mode)
        {
        case ui::NeoPixelMode::SIMPLE_LV:
            lvMeter_.update(lv);
            break;

        case ui::NeoPixelMode::GAMING_LV:
            gaming_.update(lv, wave, n, dt);
            break;

        case ui::NeoPixelMode::SPECTRUM:
            spectrumMeter_.update(lv, wave, n, dt);
            break;

        default:
            break;
        }
    }
}

namespace
{
float
computeDB(float v)
{
    constexpr float sc = 4.3429448190325175f; // 10 / log(10)
    return v > 0 ? logf(v) * sc : -50.0f;
}
} // namespace

void
NeoPixelDisp::LevelMeter::update(const std::array<float, 2>& lv)
{
    constexpr float scale = 1.0f / (30.0f / 5);
    constexpr float bias  = 10.0f * scale;

    float v[2] = {computeDB(lv[0]) * -scale - bias,
                  computeDB(lv[1]) * -scale - bias};

    for (int i = 0; i < 2; ++i)
    {
        int step = i == 0 ? 1 : -1;
        int ipix = i == 0 ? 0 : 9;
        float vv = v[i];
        for (int j = 0; j < 5; ++j)
        {
            float d = vv - (j + 1);
            if (d > 0)
            {
                pixels_.setPixel(ipix, 0, 0, 0);
            }
            else
            {
                float s = std::min(1.0f, -d);
                pixels_.setPixel(ipix, 64 * s, 32 * s, 128 * s);
            }
            ipix += step;
        }
    }

    pixels_.show();
}

void
NeoPixelDisp::SpectrumMeter::update(const std::array<float, 2>& lv,
                                    const int16_t* wave,
                                    int n,
                                    float dt)
{
    constexpr int N = 5;

    constexpr float pwScale = 1.0f / (30.0f) * 2; //  * 0.5f /* lv かける分 */;
    constexpr float pwBias = (20.0f + 30) * pwScale;

    constexpr float phaseShiftPerSec = 0.1f;
    constexpr float phaseStep        = 0.3f / N;

    constexpr float l7040_60 = 4.765018886929988f; // log(7040/60)
    const float es           = l7040_60 / (N - 1);
    const float ls           = 60.0f / (44100.0f / 3 / 256);

    phase_ += phaseShiftPerSec * dt;
    if (phase_ > 1.0f)
    {
        phase_ -= 1.0f;
    }

    for (int i = 0; i < 2; ++i)
    {
        int step    = i == 0 ? -1 : 1;
        int ipix    = i == 0 ? 4 : 5;
        float phase = phase_;
        // float phase = 0;

        float bs = lv[i] / (32768 * 32768);
        // float bs = 1.0f / (32768 * 32768);

        for (int i = 0; i < N; ++i)
        {
            float fpos = ls * expf(es * i);
            int ipos   = static_cast<int>(fpos);
            int sr     = wave[ipos];
            int si     = wave[n - ipos - 1];
            float pw   = (sr * sr + si * si) * bs;
            float v    = pwScale * computeDB(pw) + pwBias;
            float pdb  = std::min(1.0f, std::max(0.0f, v));
            float pdb2 = std::min(1.0f, std::max(0.0f, 2.0f - v));
            //            printf("%d, %f, %f, %f %f\n", i, fpos, pw, pdb, pdb2);

            auto rgb = hsv2rgb(phase, pdb2, pdb);
            int r    = static_cast<int>(rgb[0] * 255);
            int g    = static_cast<int>(rgb[1] * 255);
            int b    = static_cast<int>(rgb[2] * 255);
            pixels_.setPixel(ipix, r, g, b);

            phase += phaseStep;
            if (phase > 1.0f)
            {
                phase -= 1.0f;
            }
            ipix += step;
        }
    }

    pixels_.show();
}

void
NeoPixelDisp::Gaming::update(const std::array<float, 2>& lv,
                             const int16_t* wave,
                             int n,
                             float dt)
{
    constexpr float scale        = 1.0f / (30.0f / 5);
    constexpr float bias         = 10.0f * scale;
    constexpr float phasePerLine = 0.7f;
    constexpr float phasePerLED  = phasePerLine / 5;

    std::array<float, 2> v{computeDB(lv[0]) * -scale - bias,
                           computeDB(lv[1]) * -scale - bias};

    // 音量が小さい値が大きな値になっている
    for (int i = 0; i < 2; ++i)
    {
        float cv = v[i];
        float pv = prevValue_[i];
        if (pv > cv)
        {
            float ph = phase_[i];
            ph += phasePerLED * (pv - cv);
            if (ph > 1.0f)
            {
                ph -= 1.0f;
            }
            phase_[i] = ph;
        }
    }

    for (int i = 0; i < 2; ++i)
    {
        int step = i == 0 ? 1 : -1;
        int ipix = i == 0 ? 0 : 9;
        float vv = v[i];
        float ph = phase_[i];
        for (int j = 0; j < 5; ++j)
        {
            float d = vv - (j + 1);
            if (d > 0)
            {
                pixels_.setPixel(ipix, 0, 0, 0);
            }
            else
            {
                float s  = std::min(1.0f, -d);
                auto rgb = hsv2rgb(ph, 1.0f, s);
                int r    = static_cast<int>(rgb[0] * 255);
                int g    = static_cast<int>(rgb[1] * 255);
                int b    = static_cast<int>(rgb[2] * 255);
                pixels_.setPixel(ipix, r, g, b);

                ph += phasePerLED;
                if (ph > 1.0f)
                {
                    ph -= 1.0f;
                }
            }
            ipix += step;
        }
    }

    prevValue_ = v;

    pixels_.show();
}

} // namespace ui
