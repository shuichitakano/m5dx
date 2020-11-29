#include "target.h"
#include <SD.h>
#include <array>
#include <audio/audio.h>
#include <audio/audio_out.h>
#include <graphics/display.h>
#include <graphics/framebuffer.h>
#include <io/ble_manager.h>
#include <io/ble_midi.h>
#include <io/bluetooth.h>
#include <io/file_util.h>
#include <utility/Config.h>
#include <utility>
#include <wire.h>

#include <driver/dac.h>

#include <dirent.h>

#include "application.h"

#include <system/timer.h>

#include "debug.h"

#undef min
#include <io/bt_a2dp_source_manager.h>
#include <system/job_manager.h>

#include <util/binary.h>

#include <audio/sound_chip_manager.h>
#include <ui/system_setting.h>

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

// The setup routine runs once when M5Stack starts up
void
setup()
{
    Serial.begin(115200);
    Serial.flush();
    Serial.print("M5Stack initializing...\n");

    sys::getDefaultJobManager().start(0, 4096, "JobManager0");

    graphics::getDisplay().initialize();
    //    graphics::getDisplay().setWindow(120, 30, 20, 20);
    //    graphics::getDisplay().fill(100, 50, 250, 150, 0xffff);

    if (!SD.begin(TFCARD_CS_PIN, SPI, 40000000, ""))
    {
        Serial.println("Card Mount Failed");
    }

    target::initGPIO();
    target::restoreBus(false);

    //    dacWrite(25, 0);
    dac_output_disable(DAC_CHANNEL_1);

    if (!target::getButtonA() && !target::getButtonB() && !target::getButtonC())
    {
        DBOUT(("reset settings.\n"));
    }
    else
    {
        ui::SystemSettings::instance().loadFromNVS();
    }

    // i2c initialize
    Wire.begin(21, 22);

    // i2c 初期化後
    ui::SystemSettings::instance().applySoundModuleType();

    // SoundModule 確定後
    audio::resetSoundChip();

    // SoundChip 初期化後
    audio::AudioOutDriverManager::instance().start();
    audio::startFMAudio();

    // bt
    if (!io::initializeBluetooth() || !io::BLEManager::instance().initialize())
    {
        DBOUT(("bluetooth initialize error."));
    }
    io::setBluetoothDeviceName("M5DX");

    static io::MidiMessageQueue midiIn;
    io::BLEMidiClient::instance().setMIDIIn(&midiIn);
    io::BLEManager::instance().registerClientProfile(
        io::BLEMidiClient::instance());

    //    io::BLEManager::instance().removeAllBondedDevices();
    // io::BLEManager::instance().startScan();

    //    io::removeAllBondedClassicBTDevices();
    // io::dumpBondedClassicBTDevices();
    // io::BTA2DPSourceManager a2dpManager_;
    // a2dpManager_.initialize(&jobManager_);
    // a2dpManager_.start();

    //

    M5DX::initialize();

    audio::resetSoundChip();

    ///

    //    target::setupBus();
    //    target::restoreBus();
    // return;

#if 0
    sys::initTimer(1000000);
    sys::setTimerPeriod(5376, true);
    sys::startTimer();
    sys::enableTimerInterrupt();
    return;
#endif

#if 0
    auto mdxFilename = "/mdx/Arsys/Knight Arms/KNA08.MDX";

    if (mdxPlayer.loadMDX(mdxFilename))
    {
        DBOUT(("load success.\n"));
    }

    //
    mdxPlayer.start();
    mdxPlayer.play(-1);
#endif
}

// The loop routine runs over and over again forever
void
loop()
{
    M5DX::tick();

    static int counter = 0;
    ++counter;

    if ((counter % 100) == 0 && 0)
    {
        static uint8_t note = 0x40;
        {
            io::MidiMessage m(0x80, note & 0x7f, 0);
            io::BLEMidiClient::instance().put(m);
        }
        ++note;
        {
            io::MidiMessage m(0x90, note & 0x7f, 0x4f);
            io::BLEMidiClient::instance().put(m);
        }
    }

    delay(1);
}

#if 0
// The arduino task
void
loopTask(void* pvParameters)
{
    setup();
    for (;;)
    {
        micros(); // update overflow
        loop();
    }
}
#endif

extern "C" void
app_main()
{
#if 0
    initArduino();
    xTaskCreatePinnedToCore(
        loopTask, "loopTask", 8192, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
#else
    setup();
    while (1)
    {
        loop();
    }

#endif
}
