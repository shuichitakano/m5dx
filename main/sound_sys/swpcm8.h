/* -*- mode:C++; -*-
 * author(s) : Shuichi TAKANO
 * since 2015/12/17(Thu) 04:22:42
 */
#ifndef _VGM_SWPCM8_HF82945CEC14C4D56A63177C905481210
#define _VGM_SWPCM8_HF82945CEC14C4D56A63177C905481210

#include "m6258_coder.h"
#include "sound_system.h"
#include <audio/sample_generator.h>
#include <stdint.h>

namespace sound_sys
{

class SWPCM8 final : public SoundSystem, public audio::SampleGenerator
{
public:
    enum Type
    {
        TYPE_ADPCM,
        TYPE_PCM16,
        TYPE_PCM8,
    };

    enum Mode
    {
        MODE_ADPCM3_9K,  // 3906.25Hz
        MODE_ADPCM5_2K,  // 5208.3333Hz
        MODE_ADPCM7_8K,  // 7812.5Hz
        MODE_ADPCM10_4K, // 10416.6666Hz
        MODE_ADPCM15_6K, // 15625Hz
        MODE_PCM16BIT,   // 15625Hz
        MODE_PCM8BIT,    // 15625Hz
    };

    enum SampleRate
    {
        SAMPLERATE_1_1024,
        SAMPLERATE_1_768,
        SAMPLERATE_1_512,
        SAMPLERATE_INHIBIT
    }; // SAM2 SAM1

    using FinishTransferFunc = void (*)();

private:
    static constexpr int MAX_VOICE = 8;

    struct Voice
    {
        bool keyon;
        bool mute;
        bool pause;

        int note;

        uint8_t mode;
        uint8_t nextType;
        uint8_t type;
        uint8_t pan;
        uint8_t volume;

        uint32_t pos;
        uint32_t posEnd;
        uint32_t delta;
        const uint8_t* sample;

        int32_t val;
        int32_t frac;
        int32_t prev;

        uint32_t nextPosEnd;
        const uint8_t* nextSample;

        M6258Coder coder;
    };

    Voice voices_[MAX_VOICE];
    uint8_t nextKeyOff_;

    uint8_t keyOnCh_;
    uint8_t keyOnTrigger_;

    uint8_t pan_;
    uint8_t type_;
    bool panShare_;

    float clock_;
    float sampleRate_;
    float baseDelta_;
    uint32_t volume_;

    //    FinishTransferFunc finishTransferFunc_{};
    SystemInfo sysInfo_;

public:
    SWPCM8() { initialize(); }
    void initialize();

    void setNote(int ch, int n) { voices_[ch].note = n; }

    // SoundSystem
    float getNote(int ch, int) const override;
    const SystemInfo& getSystemInfo() const override;
    float getVolume(int ch) const override;
    float getPan(int ch) const override;
    int getInstrument(int ch) const override;
    bool mute(int ch, bool f) override;
    uint32_t getKeyOnChannels() const override;
    uint32_t getKeyOnTrigger() override;
    uint32_t getEnabledChannels() const override;
    const char* getStatusString(int ch, char* buf, int n) const override;

    // SampleGenerator
    void accumSamples(std::array<int32_t, 2>* buffer,
                      uint32_t samples) override;
    void setSampleRate(float rate) override;

    void setClock(int clock);
    void setVolume(float v);

    void setChPan(int ch, int pan);
    void setChVolume(int ch, uint8_t vol);
    void setChRate(int ch, float r);
    void play(int ch, const void* addr, int len, Type type = TYPE_ADPCM);
    void stop(int ch);

    bool isChKeyOn(int ch) const { return voices_[ch].keyon; }

    void setFreq(bool hi) { setClock(hi ? 8000000 : 4000000); }
    void setChMask(bool l, bool r);
    void resetPCM8();
    void pcm8(int ch, const void* addr, int mode, int len);
    void startTransfer(const uint8_t* p, int length);
    void stopTransfer();
    void pause() { voices_[0].pause = true; }
    void resume() { voices_[0].pause = false; }
    //    void setCallback(FinishTransferFunc f) { finishTransferFunc_ = f; }
    void setCallback(FinishTransferFunc) {}
    void play() {}
    void stop() { stop(0); }
    void setSampleRate6258(int r);

protected:
    void updateDelta();
};

} /* namespace sound_sys */

#endif /* _VGM_SWPCM8_HF82945CEC14C4D56A63177C905481210 */
