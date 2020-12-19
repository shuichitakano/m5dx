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

//
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
//
#include <driver/i2s.h>

#define ENABLE_FMDATA_DEBUG 0

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
        //        cfg.intr_alloc_flags     = ESP_INTR_FLAG_LEVEL2;
        cfg.dma_buf_count = 2;
        cfg.dma_buf_len   = 128 << overSampleShift_;
        cfg.use_apll      = false;

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

#if ENABLE_FMDATA_DEBUG
uint32_t debugRawFMData_[4];
uint32_t debugSRCFMData_[4];
#endif

void
dumpFMDataDebug()
{
#if ENABLE_FMDATA_DEBUG
    printf("raw: %08x %08x %08x %08x\n",
           debugRawFMData_[0],
           debugRawFMData_[1],
           debugRawFMData_[2],
           debugRawFMData_[3]);
    printf("SRC: %08x %08x %08x %08x\n",
           debugSRCFMData_[0],
           debugSRCFMData_[1],
           debugSRCFMData_[2],
           debugSRCFMData_[3]);
#endif
}

////
class FMOutputHandler
{
public:
    enum class SourceFormat
    {
        YM2151,
        YMF288,
    };

private:
    static constexpr i2s_port_t port_ = I2S_NUM_1;

    SourceFormat srcFormat_ = SourceFormat::YM2151;
    uint32_t sampleRate_    = 62500;
    int clockDiv_           = 64;
    int bytesPerSample_     = 4;
    int unitReadSamples_    = 128;
    bool installed_         = false;

    static constexpr size_t MAX_UPDATE_SAMPLE_COUNT = UNIT_SAMPLE_COUNT * 2;
    static constexpr size_t BUFFER_SIZE = MAX_UPDATE_SAMPLE_COUNT * 2;
    int16_t buffer_[BUFFER_SIZE];
    util::RingBuffer<int16_t> ring_{buffer_, BUFFER_SIZE};
    SimpleLinearSamplingRateConverter src_;

    int nextSampleRate_ = 0;

public:
    FMOutputHandler() { install(); }
    ~FMOutputHandler() { uninstall(); }

    void setFormat(SourceFormat fmt)
    {
        if (srcFormat_ != fmt)
        {
            uninstall();

            srcFormat_ = fmt;
            install();
        }
    }

    void install()
    {
        switch (srcFormat_)
        {
        case SourceFormat::YM2151:
            install(16, 4);
            break;

        case SourceFormat::YMF288:
            install(24, 8);
            break;

        default:
            break;
        }
    }

    void install(int bitsPerSample, int bytesPerSample);
    void uninstall();

    void setClockDiv(int v) { clockDiv_ = v; }

    void setSampleRateByClock(uint32_t clock)
    {
        auto sampleRate = (clock * 2 / clockDiv_ + 1) >> 1;
        DBOUT(("setSampleRate: clock %d, rate %d\n", clock, sampleRate));
        setSampleRate(sampleRate);
    }

    void setSampleRate(uint32_t sampleRate) { nextSampleRate_ = sampleRate; }

    uint32_t getSampleRate() const { return sampleRate_; }

    void applyChanges()
    {
        if (nextSampleRate_)
        {
            sampleRate_ = nextSampleRate_;
            auto r      = i2s_set_sample_rates(port_, sampleRate_);
            assert(r == ESP_OK);

            src_.setSamplingStep(sampleRate_ / (float)DEFAULT_SAMPLE_RATE);

            nextSampleRate_ = 0;
        }
    }

    void read(uint32_t* buf, size_t n)
    {
        size_t bytesRead;
        i2s_read(port_, buf, n * bytesPerSample_, &bytesRead, portMAX_DELAY);
        // i2s_read(port_, buf, n * bytesPerSample_, &bytesRead, (TickType_t)1);
    }

    size_t decode(uint32_t* buf, size_t n)
    {
        n = std::min<size_t>(n, ring_.getWritableSize() >> 1);
        switch (srcFormat_)
        {
        case SourceFormat::YM2151:
            decodeYM3012Sample(ring_.getWritePointer(), buf, n);
            break;

        case SourceFormat::YMF288:
            decodeYMF288Sample(ring_.getWritePointer(), buf, n);
            break;
        }
        ring_.advanceWritePointer(n << 1);
        return n;
    }

    void updateRing(size_t n)
    {
        uint32_t tmp[MAX_UPDATE_SAMPLE_COUNT];
        assert(n * bytesPerSample_ <=
               MAX_UPDATE_SAMPLE_COUNT * sizeof(uint32_t));
        read(tmp, n);

        auto ct = decode(tmp, n);
        ct      = decode(tmp + ct * (bytesPerSample_ >> 2), n - ct);

#if ENABLE_FMDATA_DEBUG
        debugRawFMData_[0] = tmp[0];
        debugRawFMData_[1] = tmp[1];
        debugRawFMData_[2] = tmp[2];
        debugRawFMData_[3] = tmp[3];
#endif
    }

    bool accum(std::array<int32_t, 2>* data, size_t nSamples, size_t sampleRate)
    {
        int sourceCt = sampleRate_ * nSamples / sampleRate + 2;
        int updateCt = sourceCt - (ring_.getFullReadableSize() >> 1);
        if (updateCt > 0)
        {
            int ct = std::min(updateCt, unitReadSamples_);
            updateRing(ct);

            ct = updateCt - ct;
            if (ct)
            {
                updateRing(ct);
            }
        }
        bool r = src_.convertAccum(data, nSamples, ring_);
#if ENABLE_FMDATA_DEBUG
        debugSRCFMData_[0] = data[0][0];
        debugSRCFMData_[1] = data[0][1];
        debugSRCFMData_[2] = data[1][0];
        debugSRCFMData_[3] = data[1][1];
#endif
        return r;
    }

    void setVolume(float v) { src_.setScale(v); }

    static FMOutputHandler& instance()
    {
        static FMOutputHandler inst;
        return inst;
    }
};

void
FMOutputHandler::install(int bitsPerSample, int bytesPerSample)
{
    if (installed_)
    {
        return;
    }

    DBOUT(("install FMOutputHandler %dbps, %dBps\n",
           bitsPerSample,
           bytesPerSample));

    {
        i2s_config_t cfg{};
        cfg.mode        = i2s_mode_t(I2S_MODE_SLAVE | I2S_MODE_RX);
        cfg.sample_rate = sampleRate_;
        switch (bitsPerSample)
        {
        case 16:
            cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
            break;

        case 24:
            cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_24BIT;
            break;

        case 32:
            cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
            break;

        default:
            return;
        }
        // cfg.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
        cfg.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
        cfg.communication_format =
            i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB);
        cfg.intr_alloc_flags = 0;
        // cfg.intr_alloc_flags = ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL3;
        //        cfg.intr_alloc_flags = ESP_INTR_FLAG_LEVEL3;
        cfg.dma_buf_count = 4;
        cfg.dma_buf_len   = 128; // sample数
        // 64sampleで1.024ms
        cfg.use_apll = false;

        //        auto r = i2s_driver_install(port_, &cfg, 2, &queue_);
        auto r = i2s_driver_install(port_, &cfg, 0, nullptr);
        assert(r == ESP_OK);

        bytesPerSample_  = bytesPerSample;
        unitReadSamples_ = MAX_UPDATE_SAMPLE_COUNT * 4 / bytesPerSample_;
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

    // YM2151:
    // 4018
    //  010 000000001 1 000
    // ^000 000000000 1 111
    //  010 000000001 0 111

    // idle状態の波形から観測
    // 0010000000001100

    // YMF288:
    // 24bitの後半16bitに符号付きPCM

    installed_ = true;
}

void
FMOutputHandler::uninstall()
{
    if (installed_)
    {
        i2s_driver_uninstall(port_);
        installed_ = false;
    }
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
        fm.applyChanges();
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
setFMClock(uint32_t freq)
{
    FMOutputHandler::instance().setSampleRateByClock(freq);
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
detachInternalSpeaker()
{
    auto& m = AudioOutDriverManager::instance();
    if (m.getDriver() == &InternalSpeakerOut::instance())
    {
        m.setDriver(nullptr);
    }
}

void
setInternalSpeaker3rdDeltaSigmaMode(bool f)
{
    InternalSpeakerOut::instance().set3rdDeltaSigmaMode(f);
}

void
setFMAudioModeYM2151()
{
    auto& fmout = FMOutputHandler::instance();
    fmout.setClockDiv(64);
    fmout.setFormat(FMOutputHandler::SourceFormat::YM2151);
}

void
setFMAudioModeYMF288()
{
    auto& fmout = FMOutputHandler::instance();
    fmout.setClockDiv(144);
    fmout.setFormat(FMOutputHandler::SourceFormat::YMF288);
}

void
startFMAudio()
{
    InternalSpeakerOut::instance(); // init
    FMOutputHandler::instance();    // init
                                    //    setFMClock(4000000, 64);
                                    //    setFMClock(8000000, 144);

    AudioOutDriverManager::instance().setAudioStreamOut(&streamOutHandler_);
}

} // namespace audio
