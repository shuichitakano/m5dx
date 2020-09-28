/*
 * author : Shuichi TAKANO
 * since  : Sun Oct 21 2018 20:37:32
 */

#include "audio.h"
#include "../debug.h"
#include "../target.h"
#include "audio_out.h"
#include "audio_stream.h"
#include "sample_generator.h"
#include "sampling_rate_converter.h"
#include "util/ring_buffer.h"
#include "ym_sample_decoder.h"
#include <string.h>

#include "sound_chip_manager.h"

//
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
//
#include <driver/i2s.h>

namespace audio
{
namespace
{
static constexpr size_t UNIT_SAMPLE_COUNT =
    AudioOutDriverManager::getUnitSampleCount();

static constexpr int DEFAULT_SAMPLE_RATE =
    AudioOutDriverManager::getSampleRate();

} // namespace

/* note
A2DPは 1512 byte リクエストがきてた.
1512/4= 378 sample

378/44100=8.571ms
20ms以上のバッファは欲しい。
50msくらい？
44100*0.05=2205sample

*/

class InternalSpeakerOut : public AudioOutDriver
{
    static constexpr i2s_port_t port_ = I2S_NUM_0;

    float volume_ = 1.0f;

    int prevSample_ = 0;
    int pv_         = 0;
    int pv1_        = 0;
    int pv2_        = 0;
    int pv3_        = 0;
    int pv4_        = 0;

    bool deltaSigma3rd_ = false;

    static constexpr int overSampleShift_ = 2;

public:
    InternalSpeakerOut()
    {
        i2s_config_t cfg{};
#if 1
        cfg.mode =
            (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN);
        cfg.sample_rate          = getSampleRate() << overSampleShift_;
        cfg.bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT;
        cfg.channel_format       = I2S_CHANNEL_FMT_RIGHT_LEFT;
        cfg.communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S_MSB);
        cfg.intr_alloc_flags     = 0;
        cfg.dma_buf_count        = 2;
        cfg.dma_buf_len          = 128 << overSampleShift_;
        cfg.use_apll             = false;

        auto r = i2s_driver_install(port_, &cfg, 0, nullptr);
        assert(r == ESP_OK);

        i2s_set_dac_mode(I2S_DAC_CHANNEL_RIGHT_EN);
#else
        // パルス駆動実験
        // 25ピンをGPIOににするとか、dac_output_disableしないと出力されない
        cfg.mode            = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
        cfg.sample_rate     = getSampleRate() << overSampleShift_;
        cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
        cfg.channel_format  = I2S_CHANNEL_FMT_RIGHT_LEFT;
        cfg.communication_format =
            i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB);
        cfg.intr_alloc_flags = 0;
        cfg.dma_buf_count    = 2;
        cfg.dma_buf_len      = 128 << overSampleShift_;
        cfg.use_apll         = false;

        auto r = i2s_driver_install(port_, &cfg, 0, nullptr);
        assert(r == ESP_OK);

        {
            i2s_pin_config_t cfg{};
            cfg.bck_io_num   = I2S_PIN_NO_CHANGE;
            cfg.ws_io_num    = I2S_PIN_NO_CHANGE;
            cfg.data_out_num = 25;
            cfg.data_in_num  = I2S_PIN_NO_CHANGE;

            auto r = i2s_set_pin(port_, &cfg);
            assert(r == ESP_OK);
        }

#endif
        writeZero();
    }

    void write(uint16_t* data, int n)
    {
        size_t writeBytes;
        i2s_write(port_, data, n * 4, &writeBytes, portMAX_DELAY);
    }

    void writeZero()
    {
        auto n = 128 * 2 << overSampleShift_;
        for (int i = 0; i < n; ++i)
        {
            uint16_t zero[] = {0, 0};
            write(zero, 1);
        }
    }

    void set3rdDeltaSigmaMode(bool f) { deltaSigma3rd_ = f; }

    uint32_t getSampleRate() const override { return 44100; };
    void setVolume(float v) override { volume_ = v; }
    float getVolume() const override { return volume_; }

    bool isDriverUseUpdate() const override { return true; };
    void onAttach() override{};
    void onDetach() override { writeZero(); };

    void onUpdate(const std::array<int32_t, 2>* data, size_t n) override
    {
        uint16_t outSampleBuffer[UNIT_SAMPLE_COUNT << 1];

        static constexpr float baseScale = 0.3f * 2; // 歪み避け

        int scale = int(volume_ * baseScale * 128);
        // int bias  = 32768;
        int bias = int(volume_ * baseScale * 32768);
        // 0.5Vccを中心にせず、0-Volume で振る (ノイズ対策)

        auto ns         = n >> overSampleShift_;
        const auto* src = data;
        int prevSample  = prevSample_;
        int pv          = pv_;
        int pv1         = pv1_;
        int pv2         = pv2_;
        int pv3         = pv3_;
        int pv4         = pv4_;

        for (auto osct = 1 << overSampleShift_; osct; --osct)
        {
            auto* dst = outSampleBuffer;

            if (deltaSigma3rd_)
            {
                for (auto ct = ns; ct; --ct)
                {
                    int v = ((((*src)[0] + (*src)[1]) * scale) >> 16) + bias;

                    auto update = [&](int v0, int ofs) {
                        pv1 += (v0 - pv);
                        pv2 += (pv1 - pv);
                        pv3 += (pv2 - pv);
                        int vq       = pv3 & 0xff00;
                        pv           = vq;
                        dst[ofs + 0] = vq;
                        dst[ofs + 1] = vq;
                    };

                    update((prevSample * 3 + v) >> 2, 0);
                    update((prevSample + v) >> 1, 2);
                    update((prevSample + v * 3) >> 2, 4);
                    update(v, 6);

                    src += 1;
                    dst += 8;
                    prevSample = v;
                }
            }
            else
            {
                for (auto ct = ns; ct; --ct)
                {
                    int v = ((((*src)[0] + (*src)[1]) * scale) >> 16) + bias;

                    auto update = [&](int v0, int ofs) {
                        pv1 += (v0 - pv);
                        int vq       = pv1 & 0xff00;
                        pv           = vq;
                        dst[ofs + 0] = vq;
                        dst[ofs + 1] = vq;
                    };

                    update((prevSample * 3 + v) >> 2, 0);
                    update((prevSample + v) >> 1, 2);
                    update((prevSample + v * 3) >> 2, 4);
                    update(v, 6);

                    src += 1;
                    dst += 8;
                    prevSample = v;
                }
            }
            write(outSampleBuffer, n);
        }

        prevSample_ = prevSample;
        pv_         = pv;
        pv1_        = pv1;
        pv2_        = pv2;
        pv3_        = pv3;
        pv4_        = pv4;
    }

    static InternalSpeakerOut& instance()
    {
        static InternalSpeakerOut inst;
        return inst;
    }
};

////
class FMOutputHandler
{
    static constexpr i2s_port_t port_ = I2S_NUM_1;
    uint32_t sampleRate_              = 62500;

    static constexpr size_t BUFFER_SIZE = UNIT_SAMPLE_COUNT * 2 * 2;
    int16_t buffer_[BUFFER_SIZE];
    util::RingBuffer<int16_t> ring_{buffer_, BUFFER_SIZE};
    SimpleLinearSamplingRateConverter src_;

public:
    FMOutputHandler();
    ~FMOutputHandler() { i2s_driver_uninstall(port_); }

    void setSampleRate(uint32_t sampleRate)
    {
        auto r = i2s_set_sample_rates(port_, sampleRate);
        assert(r == ESP_OK);
        sampleRate_ = sampleRate;

        src_.setSamplingStep(sampleRate / (float)DEFAULT_SAMPLE_RATE);
    }
    uint32_t getSampleRate() const { return sampleRate_; }

    void read(uint32_t* buf, size_t n)
    {
        size_t bytesRead;
        i2s_read(port_, buf, n << 2, &bytesRead, portMAX_DELAY);
    }

    size_t decode(uint32_t* buf, size_t n)
    {
        n = std::min<size_t>(n, ring_.getWritableSize() >> 1);
        decodeYM3012Sample(ring_.getWritePointer(), buf, n);
        ring_.advanceWritePointer(n << 1);
        return n;
    }

    void updateRing(size_t n)
    {
        uint32_t tmp[UNIT_SAMPLE_COUNT * 2];
        read(tmp, n);

        auto ct = decode(tmp, n);
        ct      = decode(tmp + ct, n - ct);
    }

    bool accum(std::array<int32_t, 2>* data, size_t nSamples, size_t sampleRate)
    {
        int sourceCt = sampleRate_ * nSamples / sampleRate + 2;
        int updateCt = sourceCt - (ring_.getFullReadableSize() >> 1);
        if (updateCt > 0)
        {
            updateRing(updateCt);
        }
        bool r = src_.convertAccum(data, nSamples, ring_);
        return r;
    }

    void setVolume(float v) { src_.setScale(v); }

    static FMOutputHandler& instance()
    {
        static FMOutputHandler inst;
        return inst;
    }
};

FMOutputHandler::FMOutputHandler()
{
    {
        i2s_config_t cfg{};
        cfg.mode        = i2s_mode_t(I2S_MODE_SLAVE | I2S_MODE_RX);
        cfg.sample_rate = sampleRate_;
        // cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
        cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
        // cfg.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
        cfg.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
        //        cfg.communication_format =
        //        i2s_comm_format_t(I2S_COMM_FORMAT_PCM |
        //        I2S_COMM_FORMAT_PCM_SHORT); cfg.communication_format =
        //        i2s_comm_format_t(I2S_COMM_FORMAT_PCM);
        cfg.communication_format =
            i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB);
        cfg.intr_alloc_flags = 0;
        cfg.dma_buf_count    = 4;
        cfg.dma_buf_len      = 128; // sample数
        // 64sampleで1.024ms
        cfg.use_apll = false;

        //        auto r = i2s_driver_install(port_, &cfg, 2, &queue_);
        auto r = i2s_driver_install(port_, &cfg, 0, nullptr);
        assert(r == ESP_OK);
    }

    {
        i2s_pin_config_t cfg{};
        cfg.bck_io_num   = target::config::SD_CLK;
        cfg.ws_io_num    = target::config::SD_WS;
        cfg.data_out_num = I2S_PIN_NO_CHANGE;
        cfg.data_in_num  = target::config::SD_DATA;

        auto r = i2s_set_pin(port_, &cfg);
        assert(r == ESP_OK);
    }

    // 4018
    //  010 000000001 1 000
    // ^000 000000000 1 111
    //  010 000000001 0 111

    // idle状態の波形から観測
    // 0010000000001100
}

////////////////////////////////

class AudioStreamOutHandler : public AudioStreamOut
{
public:
    void onUpdateAudioStream(std::array<int32_t, 2>* data,
                             size_t nSamples,
                             size_t sampleRate) override
    {
        memset(data, 0, nSamples * sizeof(std::array<int32_t, 2>));

        auto& fm = FMOutputHandler::instance();
        fm.accum(data, nSamples, sampleRate);

        getSampleGeneratorManager().setSampleRate(sampleRate);
        getSampleGeneratorManager().accumSamples(data, nSamples);
    }
};

namespace
{

AudioStreamOutHandler streamOutHandler_;

} // namespace

////////////////////////////////

void
setFMClock(uint32_t freq, int sampleRateDiv)
{
    auto sampleRate = (freq * 2 / sampleRateDiv + 1) >> 1;
    FMOutputHandler::instance().setSampleRate(sampleRate);

    target::startFMClock(freq);
}

void
setFMVolume(float v)
{
    FMOutputHandler::instance().setVolume(v);
}

void
attachInternalSpeaker()
{
    AudioOutDriverManager::instance().setDriver(
        &InternalSpeakerOut::instance());
}

void
setInternalSpeaker3rdDeltaSigmaMode(bool f)
{
    InternalSpeakerOut::instance().set3rdDeltaSigmaMode(f);
}

void
initialize()
{
    target::startFMClock(4000000);
    resetSoundChip(); // FMOutputHandlerを動かす前に

    InternalSpeakerOut::instance(); // init
    FMOutputHandler::instance();    // init
    setFMClock(4000000, 64);

    AudioOutDriverManager::instance().setAudioStreamOut(&streamOutHandler_);
    attachInternalSpeaker();
}

} // namespace audio
