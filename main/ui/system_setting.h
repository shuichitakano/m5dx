/*
 * author : Shuichi TAKANO
 * since  : Mon Feb 11 2019 18:56:47
 */
#ifndef _06B5AA0F_2134_13F9_11B4_7DD6FD1CFFBB
#define _06B5AA0F_2134_13F9_11B4_7DD6FD1CFFBB

#include "locale.h"
#include <array>

namespace ui
{

enum class SoundModule
{
    AUTO,
    NOTHING,
    YM2151,
    YMF288,
};

enum class RepeatMode
{
    NONE,
    ALL,
    SINGLE,
};

enum class PlayerDialMode
{
    VOLUME,
    TRACK_SELECT,
};

enum class InitialBTMode
{
    DISABLE,
    _30SEC,
    _60SEC,
    ALWAYS,
};

struct TrackSetting
{
    bool mask_     = true;
    bool override_ = false;
};

using BluetoothADDR = std::array<uint8_t, 6>;

struct BluetoothMIDI
{
    static constexpr size_t N_PAIRED_ADDR = 4;
    InitialBTMode initialMode             = InitialBTMode::_60SEC;
    size_t nAddr                          = 0;
    BluetoothADDR addr[N_PAIRED_ADDR]{};

    void update(const BluetoothADDR& v);
    bool find(const BluetoothADDR& v) const;
};

struct BluetoothAudio
{
    static constexpr size_t N_PAIRED_ADDR = 4;
    InitialBTMode initialMode             = InitialBTMode::_60SEC;
    size_t nAddr                          = 0;
    BluetoothADDR addr[N_PAIRED_ADDR]{};

    void update(const BluetoothADDR& v);
    bool find(const BluetoothADDR& v) const;
};

class SystemSettings
{
    Language language_         = Language::JAPANESE;
    SoundModule soundModule_   = SoundModule::AUTO;
    int volume_                = -6;
    int loopCount_             = 1;
    bool shuffleMode_          = false;
    RepeatMode repeatMode_     = RepeatMode::ALL;
    int backLightIntensity_    = 30;
    bool internalSpeaker_      = true;
    bool displayOffIfReverse_  = true; // 反転で画面オフ
    PlayerDialMode playerDial_ = PlayerDialMode::VOLUME;

    TrackSetting trackSetting_[100];

    BluetoothMIDI btMIDI_;
    BluetoothAudio btAudio_;

public:
    int getVolume() const { return volume_; }
    void setVolume(int v) { volume_ = v; }

    Language getLanguage() const { return language_; }
    void setLanguage(Language l) { language_ = l; }

    SoundModule getSoundModule() const { return soundModule_; }
    void setSoundModule(SoundModule v) { soundModule_ = v; }

    int getLoopCount() const { return loopCount_; }
    void setLoopCount(int n) { loopCount_ = n; }

    bool isShuffleMode() const { return shuffleMode_; }
    void setShuffleMode(bool f) { shuffleMode_ = f; }

    RepeatMode getRepeatMode() const { return repeatMode_; }
    void setRepeatMode(RepeatMode m) { repeatMode_ = m; }

    int getBackLightIntensity() const { return backLightIntensity_; }
    void setBackLightIntensity(int n) { backLightIntensity_ = n; }

    bool isEnabledInternalSpeaker() const { return internalSpeaker_; }
    void enableInternalSpeaker(bool f) { internalSpeaker_ = f; }

    bool isEnabledDisplayOffIfReverse() const { return displayOffIfReverse_; }
    void enableDisplayOffIfReverse(bool f) { displayOffIfReverse_ = f; }

    bool getTrackMask(size_t i) const { return trackSetting_[i].mask_; }
    void setTrackMask(size_t i, bool f) { trackSetting_[i].mask_ = f; }

    bool getTrackOverride(size_t i) const { return trackSetting_[i].override_; }
    void setTrackOverride(size_t i, bool f) { trackSetting_[i].override_ = f; }

    PlayerDialMode getPlayerDialMode() const { return playerDial_; }
    void setPlayerDialMode(PlayerDialMode m) { playerDial_ = m; }

    BluetoothAudio& getBluetoothAudio() { return btAudio_; }
    BluetoothMIDI& getBluetoothMIDI() { return btMIDI_; }

    //
    void applyBackLightIntensity() const;

    static SystemSettings& instance();
};

const char* toString(SoundModule v);
const char* toString(Language l);
const char* toString(RepeatMode m);
const char* toString(PlayerDialMode m);
const char* toString(InitialBTMode m);

} // namespace ui

#endif /* _06B5AA0F_2134_13F9_11B4_7DD6FD1CFFBB */
