/*
 * author : Shuichi TAKANO
 * since  : Sun Oct 21 2018 19:34:47
 */

#include "target.h"
#include "driver/gpio.h"
#include <SPI.h>
#include <assert.h>
#include <driver/ledc.h>
#include <esp32-hal-gpio.h>
#include <esp32-hal-spi.h>
#include <soc/gpio_sig_map.h>

namespace target
{

void
initGPIO()
{
    uint64_t _1 = 1;

    {
        gpio_config_t cnf{};
        cnf.mode         = GPIO_MODE_OUTPUT;
        cnf.pin_bit_mask = ( //
            (_1 << config::D0) | (_1 << config::D1) | (_1 << config::D2) |
            (_1 << config::D3) | (_1 << config::D4) | (_1 << config::D7) |
            (_1 << config::CS) | (_1 << config::A0) |
            (_1 << config::NEO_PIXEL_DATA) | 0);
        cnf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        cnf.pull_up_en   = GPIO_PULLUP_DISABLE;
        cnf.intr_type    = GPIO_INTR_DISABLE;

        auto r = gpio_config(&cnf);
        assert(r == ESP_OK);
    }
    negateFMCS();
    {
        gpio_config_t cnf{};
        cnf.mode         = GPIO_MODE_OUTPUT_OD;
        cnf.pin_bit_mask = (_1 << config::D5) | (_1 << config::D6);
        cnf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        cnf.pull_up_en   = GPIO_PULLUP_ENABLE;
        cnf.intr_type    = GPIO_INTR_DISABLE;

        auto r = gpio_config(&cnf);
        assert(r == ESP_OK);
    }

    {
        gpio_config_t cnf{};
        cnf.mode         = GPIO_MODE_INPUT;
        cnf.pin_bit_mask = ( //
            (_1 << config::BUTTON_A) | (_1 << config::BUTTON_B) |
            (_1 << config::BUTTON_C));
        cnf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        cnf.pull_up_en   = GPIO_PULLUP_ENABLE;
        cnf.intr_type    = GPIO_INTR_DISABLE;

        auto r = gpio_config(&cnf);
        assert(r == ESP_OK);
    }
}

void
lockBus()
{
    spiSimpleTransaction(SPI.bus());
}

void
unlockBus()
{
    spiEndTransaction(SPI.bus());
}

void
setupBus(bool useA1)
{
    lockBus();

    gpio_matrix_out(config::D7, SIG_GPIO_OUT_IDX, false, false);
    gpio_matrix_out(config::D2, SIG_GPIO_OUT_IDX, false, false);
    gpio_matrix_out(config::D0, SIG_GPIO_OUT_IDX, false, false);

    pinMode(config::D3, OUTPUT);
    gpio_matrix_out(config::D3, SIG_GPIO_OUT_IDX, false, false);

    if (useA1)
    {
        // ESP32側RXDとUSB側TXDが470Ωで接続されている
        // 極性が違うと7mA程度流れる
        pinMode(config::A1, OUTPUT);
        gpio_matrix_out(config::A1, SIG_GPIO_OUT_IDX, false, false);
    }
}

void
restoreBus(bool useA1)
{
    gpio_matrix_out(config::D7, VSPID_OUT_IDX, false, false);
    gpio_matrix_out(config::D2, VSPICLK_OUT_IDX, false, false);
    gpio_matrix_out(config::D0, U0TXD_OUT_IDX, false, false);

    if (useA1)
    {
        pinMode(config::A1, INPUT);
        gpio_matrix_in(config::A1, U0RXD_IN_IDX, false);
    }

    pinMode(config::D3, INPUT);
    gpio_matrix_in(config::D3, VSPIQ_OUT_IDX, false);

    unlockBus();
}

void
startI2C()
{
    lockBus();

    pinMode(config::D6, OPEN_DRAIN | PULLUP | INPUT | OUTPUT); // scl
    pinMode(config::D5, OPEN_DRAIN | PULLUP | INPUT | OUTPUT); // sda
    gpio_matrix_out(config::D6, I2CEXT0_SCL_OUT_IDX, false, false);
    gpio_matrix_in(config::D6, I2CEXT0_SCL_OUT_IDX, false);
    gpio_matrix_out(config::D5, I2CEXT0_SDA_OUT_IDX, false, false);
    gpio_matrix_in(config::D5, I2CEXT0_SDA_OUT_IDX, false);
}

void
endI2C()
{
    pinMode(config::D6, OPEN_DRAIN | PULLUP | OUTPUT);
    pinMode(config::D5, OPEN_DRAIN | PULLUP | OUTPUT);
    gpio_matrix_out(config::D6, SIG_GPIO_OUT_IDX, false, false);
    gpio_matrix_out(config::D5, SIG_GPIO_OUT_IDX, false, false);

    setBusIdle();

    unlockBus();
}

void
writeBusData(int d)
{
    //  x, x, x, 5, x, x, 2, 1 : 00010011 : 0x13
    // 23,22,21, x,19,18, x, x : 11101100 : 0xec

    int s0 = d & 0x13;
    int s1 = d & 0xec;
    int id = ~d;
    int c0 = id & 0x13;
    int c1 = id & 0xec;

    GPIO.out_w1ts = (s0 << 1) | (s1 << 16);
    GPIO.out_w1tc = (c0 << 1) | (c1 << 16);
}

void
setBusIdle()
{
    // SCL, SDAのビットを1にしておく
    // - Start Condition になったら即 Stop Conditionに戻す
    // - Open Drain による立ち上がりの遅さを防ぐ

    GPIO.out_w1ts = (1 << 21) | (1 << 22);
}

void
negateFMCS()
{
    gpio_set_level((gpio_num_t)config::CS, 1);
}

void
assertFMCS()
{
    gpio_set_level((gpio_num_t)config::CS, 0);
}

void
setFMA0(int i)
{
    gpio_set_level((gpio_num_t)config::A0, i);
}

void
setFMA1(int i)
{
    gpio_set_level((gpio_num_t)config::A1, i);
}

bool
getButtonA()
{
    return gpio_get_level((gpio_num_t)config::BUTTON_A);
}

bool
getButtonB()
{
    return gpio_get_level((gpio_num_t)config::BUTTON_B);
}

bool
getButtonC()
{
    return gpio_get_level((gpio_num_t)config::BUTTON_C);
}

void
startFMClock(uint32_t freq)
{
    // 無理やり四捨五入させる
    auto adjustFreq = [](int f) -> int {
        // a/f + 0.5 = a/g
        // g = a/(a/f + 0.5) = af/(a + 0.5f) = 2af/(2a + f)
        constexpr uint64_t base = INT64_C(80000000 * 256);
        return base * f / (base + f);
    };

    {
        ledc_timer_config_t cfg{};
        cfg.duty_resolution = LEDC_TIMER_1_BIT;
        cfg.freq_hz         = adjustFreq(freq);
        cfg.speed_mode      = LEDC_HIGH_SPEED_MODE;
        cfg.timer_num       = LEDC_TIMER_0;

        auto r = ledc_timer_config(&cfg);
        assert(r == ESP_OK);
    }
    {
        ledc_channel_config_t cfg{};
        cfg.channel    = LEDC_CHANNEL_0;
        cfg.duty       = 1;
        cfg.gpio_num   = config::CLK;
        cfg.speed_mode = LEDC_HIGH_SPEED_MODE;
        cfg.timer_sel  = LEDC_TIMER_0;
        auto r         = ledc_channel_config(&cfg);
        assert(r == ESP_OK);
    }
}

} // namespace target
