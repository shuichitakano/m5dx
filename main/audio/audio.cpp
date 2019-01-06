/*
 * author : Shuichi TAKANO
 * since  : Sun Oct 21 2018 20:37:32
 */

#include "audio.h"
#include "../debug.h"
#include "../target.h"
#include "ym_sample_decoder.h"

//
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/i2s.h>

namespace audio
{
/* note
A2DPは 1512 byte リクエストがきてた.
1512/4= 378 sample

378/44100=8.571ms
20ms以上のバッファは欲しい。
50msくらい？
44100*0.05=2205sample

*/

class InternalSpeakerOut
{
    static constexpr i2s_port_t port_ = I2S_NUM_0;

  public:
    InternalSpeakerOut()
    {
        i2s_config_t cfg{};
        cfg.mode =
            (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN);
        cfg.sample_rate = 62500;
        cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
        cfg.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
        cfg.communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S_MSB);
        cfg.intr_alloc_flags = 0;
        cfg.dma_buf_count = 4;
        cfg.dma_buf_len = 128;
        cfg.use_apll = false;

        auto r = i2s_driver_install(port_, &cfg, 0, nullptr);
        assert(r == ESP_OK);

        i2s_set_dac_mode(I2S_DAC_CHANNEL_RIGHT_EN);
    }

    void write(uint16_t *data, int n)
    {
        size_t writeBytes;
        i2s_write(port_, data, n * 4, &writeBytes, portMAX_DELAY);
    }

    static InternalSpeakerOut &instance()
    {
        static InternalSpeakerOut inst;
        return inst;
    }
};

class FMOutputHandler
{
    static constexpr i2s_port_t port_ = I2S_NUM_1;
    //    QueueHandle_t queue_;

  public:
    FMOutputHandler();
    ~FMOutputHandler();

    void setSampleRate(uint32_t sampleRate);

    void read(uint32_t *buf, size_t size);

    static FMOutputHandler &instance()
    {
        static FMOutputHandler inst;
        return inst;
    }

  private:
    static void receiveTaskEntry(void *p)
    {
        ((FMOutputHandler *)p)->receiveTask();
    }
    void receiveTask();
};

FMOutputHandler::FMOutputHandler()
{
    auto sampleRate = 62500;

    {
        i2s_config_t cfg{};
        cfg.mode = i2s_mode_t(I2S_MODE_SLAVE | I2S_MODE_RX);
        cfg.sample_rate = sampleRate;
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
        cfg.dma_buf_count = 4;
        cfg.dma_buf_len = 128; // sample数
        // 64sampleで1.024ms
        cfg.use_apll = false;

        //        auto r = i2s_driver_install(port_, &cfg, 2, &queue_);
        auto r = i2s_driver_install(port_, &cfg, 0, nullptr);
        assert(r == ESP_OK);
    }

    {
        i2s_pin_config_t cfg{};
        cfg.bck_io_num = target::config::SD_CLK;
        cfg.ws_io_num = target::config::SD_WS;
        cfg.data_out_num = I2S_PIN_NO_CHANGE;
        cfg.data_in_num = target::config::SD_DATA;

        auto r = i2s_set_pin(port_, &cfg);
        assert(r == ESP_OK);
    }

    // constexpr int prio = 5;
    // auto r = xTaskCreate(receiveTaskEntry, "FMrcv", 2048, this, prio,
    // nullptr); assert(r);

    // 4018
    //  010 000000001 1 000
    // ^000 000000000 1 111
    //  010 000000001 0 111

    // idle状態の波形から観測
    // 0010000000001100
}

FMOutputHandler::~FMOutputHandler()
{
    i2s_driver_uninstall(port_);
}

void FMOutputHandler::setSampleRate(uint32_t sampleRate)
{
    auto r = i2s_set_sample_rates(port_, sampleRate);
    assert(r == ESP_OK);
}

/*
void
FMOutputHandler::receiveTask()
{
    DBOUT(("Enter FM sound receive task.\n"));
    while (1)
    {
        i2s_event_t event;
        if (xQueueReceive(queue_, &event, portMAX_DELAY))
        {
            DBOUT(("t%d,%d\n", event.type, event.size));
        }
    }
}
*/

void FMOutputHandler::read(uint32_t *buf, size_t size)
{
    size_t bytesRead;
    i2s_read(port_, buf, size, &bytesRead, portMAX_DELAY);
}

////////////////////////////////

namespace
{
static constexpr int unitCount = 128;

int16_t recentSample[unitCount * 2];
uint16_t outSampleBuffer[unitCount * 2];
} // namespace

void audioTask(void *)
{
    DBOUT(("start Audio task.\n"));

    //    static constexpr int unitCount = 128;
    static uint32_t sampleBuffer[unitCount];
    //    uint32_t accumBuffer[unitCount * 2];

    while (1)
    {
        FMOutputHandler::instance().read(sampleBuffer, unitCount * 4);
        // auto* decodeBuffer = (int16_t*)sampleBuffer;
        //        decodeYM3012Sample(decodeBuffer, sampleBuffer, unitCount);
        decodeYM3012Sample(recentSample, sampleBuffer, unitCount);

        int scale = int(0.3f * 65535);
        const auto *src = recentSample;
        auto *dst = outSampleBuffer;
        for (int i = 0; i < unitCount; ++i)
        {
            int v = (((src[0] + src[1]) * scale) >> 17) + 32768;
            dst[0] = v;
            dst[1] = v;
            src += 2;
            dst += 2;
        }

        //        InternalSpeakerOut::instance().write(outSampleBuffer,
        //        unitCount);
    }
}

void startAudio()
{
    FMOutputHandler::instance();    // init
    InternalSpeakerOut::instance(); // init
    setFMClock(4000000, 64);

    constexpr int prio = 5;
    auto r = xTaskCreate(audioTask, "FMrcv", 2048, nullptr, prio, nullptr);
    assert(r);
}

////////////////////////////////

void setFMClock(uint32_t freq, int sampleRateDiv)
{
    auto sampleRate = (freq * 2 / sampleRateDiv + 1) >> 1;
    FMOutputHandler::instance().setSampleRate(sampleRate);

    target::startFMClock(freq);
}

void initialize()
{
    //    FMOutputHandler::instance(); // init
    //    setFMClock(4000000, 64);

    startAudio();
}

const int16_t *
getRecentSampleForTest()
{
    //    FMOutputHandler::instance().read((uint32_t*)recentSample, 4 * 128);
    return recentSample;
}

} // namespace audio
