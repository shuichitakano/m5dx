/*
 * author : Shuichi TAKANO
 * since  : Sun Oct 21 2018 20:37:32
 */

#include "audio.h"
#include "../debug.h"
#include "../target.h"
#include "audio_out.h"
#include "audio_stream.h"
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

    //    float volume_ = 1.0f;
    float volume_ = 0.5f;

public:
    InternalSpeakerOut()
    {
        i2s_config_t cfg{};
        cfg.mode =
            (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN);
        cfg.sample_rate          = getSampleRate();
        cfg.bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT;
        cfg.channel_format       = I2S_CHANNEL_FMT_RIGHT_LEFT;
        cfg.communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S_MSB);
        cfg.intr_alloc_flags     = 0;
        cfg.dma_buf_count        = 4;
        cfg.dma_buf_len          = 128;
        cfg.use_apll             = false;

        auto r = i2s_driver_install(port_, &cfg, 0, nullptr);
        assert(r == ESP_OK);

        i2s_set_dac_mode(I2S_DAC_CHANNEL_RIGHT_EN);
        writeZero();
    }

    void write(uint16_t* data, int n)
    {
        size_t writeBytes;
        i2s_write(port_, data, n * 4, &writeBytes, portMAX_DELAY);
    }

    void writeZero()
    {
        for (int i = 0; i < 128 * 4; ++i)
        {
            uint16_t zero[] = {0, 0};
            write(zero, 1);
        }
    }

    uint32_t getSampleRate() const override { return 44100; };
    void setVolume(float v) override { volume_ = v; }
    float getVolume() const override { return volume_; }

    bool isDriverUseUpdate() const override { return true; };
    void onAttach() override{};
    void onDetach() override { writeZero(); };

    void onUpdate(const std::array<int32_t, 2>* data, size_t n) override
    {
        uint16_t outSampleBuffer[UNIT_SAMPLE_COUNT << 1];

        static constexpr float baseScale = 0.3f; // 歪み避け

        int scale = int(volume_ * baseScale * 128);
        // int bias  = 32768;
        int bias = int(volume_ * baseScale * 32768);
        // 0.5Vccを中心にせず、0-Volume で振る (ノイズ対策)

        const auto* src = data;
        auto* dst       = outSampleBuffer;

        for (auto ct = n; ct; --ct)
        {
            int v  = ((((*src)[0] + (*src)[1]) * scale) >> 16) + bias;
            dst[0] = v;
            dst[1] = v;
            src += 1;
            dst += 2;
        }
        write(outSampleBuffer, n);
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
attachInternalSpeaker()
{
    AudioOutDriverManager::instance().setDriver(
        &InternalSpeakerOut::instance());
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
