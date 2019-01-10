#include "target.h"
#include <Adafruit_NeoPixel.h>
#include <SD.h>
#include <audio/audio.h>
#include <graphics/display.h>
#include <graphics/font_data.h>
#include <graphics/font_manager.h>
#include <graphics/framebuffer.h>
#include <io/ble_manager.h>
#include <io/ble_midi.h>
#include <io/bluetooth.h>
#include <io/file_util.h>
#include <utility/Config.h>
#include <utility>

#include <sys/timer.h>

#include "debug.h"
#include <music_player/mdxplayer.h>

#undef min
#include <sys/job_manager.h>

#define M5STACK_FIRE_NEO_NUM_LEDS 10
#define M5STACK_FIRE_NEO_DATA_PIN 15

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(
    M5STACK_FIRE_NEO_NUM_LEDS, M5STACK_FIRE_NEO_DATA_PIN, NEO_GRB + NEO_KHZ800);

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

constexpr uint16_t
makeColor(int r, int g, int b)
{
    return ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3);
}

static music_player::MDXPlayer mdxPlayer;

static sys::JobManager jobManager_;

// void
// spriteTest()
// {
//     using Sprite = TFT_eSprite;

//     Sprite img(&lcd);

//     img.setColorDepth(1);
//     img.createSprite(128, 64);
//     img.fillSprite(0);

//     img.fillTriangle(35, 0, 0, 59, 69, 59, 1);
//     img.fillTriangle(35, 79, 0, 20, 69, 20, 1);

//     img.setBitmapColor(makeColor(255, 0, 255), 0);
//     img.pushSprite(10, 20, 0);

//     img.deleteSprite();
// }

void
listDir(fs::FS& fs, const char* dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if (!root)
    {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels)
            {
                listDir(fs, file.name(), levels - 1);
            }
        }
        else
        {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

namespace
{
graphics::FrameBuffer waveViewBuffer_;
}

// The setup routine runs once when M5Stack starts up
void
setup()
{

    Serial.begin(115200);
    Serial.flush();
    Serial.print("M5Stack initializing...\n");

    jobManager_.start();

    graphics::getDisplay().initialize();
    //    graphics::getDisplay().setWindow(120, 30, 20, 20);
    //    graphics::getDisplay().fill(100, 50, 250, 150, 0xffff);

    //    waveViewBuffer_.initialize(128, 64, 16);
    waveViewBuffer_.initialize(128, 128, 16);

    if (!SD.begin(TFCARD_CS_PIN, SPI, 40000000))
    {
        Serial.println("Card Mount Failed");
    }

    pixels.begin();

    //    delay(500);

    target::initGPIO();
    //    delay(10);
    target::restoreBus();

    dacWrite(25, 0);
    audio::initialize();

    jobManager_.add([] {
        printf("job1\n");
        delay(1000);
    });
    jobManager_.add([] { printf("job2\n"); });

    if (!io::initializeBluetooth() || !io::BLEManager::instance().initialize())
    {
        DBOUT(("bluetooth initialize error."));
    }
    io::BLEManager::instance().setDeviceName("M5DX");

    static io::MidiMessageQueue midiIn;
    io::BLEMidiClient::instance().setMIDIIn(&midiIn);
    io::BLEManager::instance().registerClientProfile(
        io::BLEMidiClient::instance());

    //    io::BLEManager::instance().removeAllBondedDevices();
    io::BLEManager::instance().startScan();

    listDir(SD, "/", 0);

    static std::vector<uint8_t> fontAsciiBin;
    static std::vector<uint8_t> fontKanjiBin;
    io::readFile(fontAsciiBin, "/data/4x8_font.bin");
    io::readFile(fontKanjiBin, "/data/misaki_font.bin");

    static graphics::FontData fontAscii(fontAsciiBin.data());
    static graphics::FontData fontKanji(fontKanjiBin.data());

    auto& fm = graphics::getDefaultFontManager();
    fm.setAsciiFontData(&fontAscii);
    fm.setKanjiFontData(&fontKanji);
    fm.setFrameBuffer(graphics::getDisplay());
    fm.setPosition(0, 20);
    fm.setColor(graphics::getDisplay().makeColor(128, 200, 50));
    //    graphics::getDisplay().setWindow(10, 22, 36, 10);
    fm.putString("日本語表示テスト\n123ABCDEF漢字");
    //    graphics::getDisplay().setWindow(0, 0, 320, 240);

    //    graphics::getDisplay().fill(0x11223344);

    // uint32_t test = 0x12345678;
    // uint32_t testr = __builtin_bitreverse(test);
    // printf("rev %08x\n", testr);

    //    spriteTest();

    //

    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE)
    {
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC)
    {
        Serial.println("MMC");
    }
    else if (cardType == CARD_SD)
    {
        Serial.println("SDSC");
    }
    else if (cardType == CARD_SDHC)
    {
        Serial.println("SDHC");
    }
    else
    {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);

    //    listDir(SD, "/", 0);
    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));

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

    //
    auto mdxFilename = "/STC00.MDX";

    if (mdxPlayer.loadMDX(mdxFilename))
    {
        DBOUT(("load success.\n"));
    }

    fm.setPosition(0, 100);
    fm.setColor(graphics::getDisplay().makeColor(0xff, 0x80, 0x00));
    fm.putString(mdxPlayer.getTitle());

    return;
    delay(500);

    //
    mdxPlayer.start();
    mdxPlayer.play(-1);
}

// The loop routine runs over and over again forever
void
loop()
{
    static int counter = 0;
    ++counter;

#if 1
    if (counter == 50)
    {
        mdxPlayer.start();
        mdxPlayer.play(-1);
    }
#endif

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

    auto& fm = graphics::getDefaultFontManager();
    fm.setColor(graphics::getDisplay().makeColor(0x40, 0xff, 0xa0));
    fm.setBGColor(graphics::getDisplay().makeColor(0x20, 0x20, 0x20));
    fm.setPosition(0, 0);
    char str[20];
    sprintf(str, "F %d\n", counter);
    fm.setEdgedMode(false);
    fm.setTransparentMode(false);
    fm.putString(str);

    auto* wave = audio::getRecentSampleForTest();

    fm.setColor(0xffffffff);
    sprintf(str, "%04x:%04x", (uint16_t)wave[0], (uint16_t)wave[1]);
    fm.putString(str);

#if 1
    waveViewBuffer_.fill(waveViewBuffer_.makeColor(0, 0, 128));
    auto red   = waveViewBuffer_.makeColor(255, 0, 0);
    auto green = waveViewBuffer_.makeColor(0, 255, 0);
    //    sprintf(str, ": r %d g %d", red, green);
    // fm.putString(str);
    for (int i = 0; i < 128; ++i)
    {
        int y0 = std::min(127, std::max(0, 64 + (int)(wave[0] >> 9)));
        int y1 = std::min(127, std::max(0, 64 + (int)(wave[1] >> 9)));
        waveViewBuffer_.setPixel(i, y0, red);
        waveViewBuffer_.setPixel(i, y1, green);
        wave += 2;
    }

    graphics::getDisplay().blit(waveViewBuffer_, 320 - 128, 240 - 128);
#endif

    static int pixelNumber = 0; // = random(0, M5STACK_FIRE_NEO_NUM_LEDS - 1);
    pixelNumber++;
    if (pixelNumber > 9)
        pixelNumber = 0;
    int r = 1 << random(0, 7);
    int g = 1 << random(0, 7);
    int b = 1 << random(0, 7);

    pixels.setPixelColor(pixelNumber, pixels.Color(r, g, b));
    pixels.show();

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
