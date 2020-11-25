/*
 * author : Shuichi TAKANO
 * since  : Wed Sep 30 2020 01:54:58
 */
#ifndef F3A5D6F1_1134_3DC7_1795_4EE85DE64647
#define F3A5D6F1_1134_3DC7_1795_4EE85DE64647

#include "music_player.h"
#include <io/memory_stream.h>
#include <map>
#include <memory>
#include <sound_sys/ymf288.h>
#include <string>
#include <vector>

namespace music_player
{

class S98Player final : public MusicPlayer
{
    std::string title_;

    bool started_  = false;
    bool playing_  = false;
    bool paused_   = false;
    int loopCount_ = 0;
    float wait_    = 0;

    uint32_t totalTimeUs_ = 0; // 71分でループしちゃう
    uint32_t prevTimeUs_  = 0;

    float samplesPerUs_ = 0;

    std::vector<uint8_t> data_;
    io::MemoryBinaryStream stream_;

    struct Header
    {
        uint32_t timerNumerator_{};
        uint32_t timerDenominator_{};
        uint32_t startOffset_{};
        uint32_t loopOffset_{};

        std::map<std::string, std::string> tags_;

        enum class DeviceType
        {
            NONE = 0,
            OPN,
            OPN2,
            OPNA,
            OPM,
            OPLL,
            OPL,
            OPL2,
            OPL3,
            PSG  = 15,
            DCSG = 16,
        };

        struct DeviceInfo
        {
            DeviceType type_;
            uint32_t clock_;
            uint32_t pan_; // port mute
        };

        std::vector<DeviceInfo> deviceInfos_;

    public:
        Header() { deviceInfos_.reserve(2); }
        bool load(io::BinaryStream* stream);
        bool loadTags(io::BinaryStream* stream);

        std::string findTitle() const;
        float getSamplesPerUs() const;
    };

    Header header_;

    class DeviceInterface
    {
    public:
        virtual ~DeviceInterface()                            = default;
        virtual void initialize(uint32_t clock, uint32_t pan) = 0;
        virtual void write(bool extra, uint_fast8_t r, uint_fast8_t v) = 0;
        virtual void mute()                                            = 0;
        virtual sound_sys::SoundSystem* getSoundSystem()               = 0;
    };
    class YM2608;

    std::vector<std::unique_ptr<DeviceInterface>> deviceInterfaces_;

public:
    S98Player();

    bool isSupported(const char* filename) override;
    std::experimental::optional<std::string>
    loadTitle(const char* filename) override;
    bool start() override;
    bool terminate() override;
    bool load(const char* filename) override;
    bool play(int track) override;
    bool stop() override;
    bool pause() override;
    bool cont() override;
    bool fadeout() override;
    bool isFinished() const override;
    bool isPaused() const override;
    int getCurrentLoop() const override;
    int getTrackCount() const override;
    int getCurrentTrack() const override;
    float getPlayTime() const override;
    const char* getTitle() const override;
    FileFormat getFormat() const override;
    sound_sys::SoundSystem* getSystem(int idx) override;

protected:
    void freeAudioChips();
    void finalizeDeviceInterfaces();
    void createDeviceInterfaces();
    void processCommand(io::BinaryStream* stream);

    void tick();
};

} // namespace music_player

#endif /* F3A5D6F1_1134_3DC7_1795_4EE85DE64647 */
