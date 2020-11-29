/*
 * author : Shuichi TAKANO
 * since  : Mon Feb 11 2019 19:3:50
 */

#include "system_setting.h"
#include "strings.h"
#include <algorithm>
#include <system/nvs.h>

#include "../debug.h"
#include "../m5dx_module.h"
#include <audio/audio.h>
#include <audio/audio_out.h>
#include <audio/sound_chip_manager.h>
#include <graphics/display.h>

namespace ui
{

namespace
{

template <class T>
size_t
updateList(const T& v, size_t n, size_t capacity, T* p)
{
    // LRU で、後ろが最近
    auto end = p + n;
    auto it  = std::find(p, end, v);
    if (it == end)
    {
        if (n == capacity)
        {
            std::copy(p + 1, end, p);
            p[n - 1] = v;
        }
        else
        {
            p[n] = v;
            ++n;
        }
    }
    else
    {
        std::copy(it + 1, end, it);
        p[n - 1] = v;
    }
    return n;
}

} // namespace

int
getSecond(InitialBTMode m)
{
    switch (m)
    {
    case InitialBTMode::_30SEC:
        return 30;

    case InitialBTMode::_60SEC:
        return 60;

    case InitialBTMode::ALWAYS:
        return -1;

    default:
        break;
    }
    return 0;
}

void
BluetoothMIDI::update(const BluetoothADDR& v)
{
    nAddr = updateList(v, nAddr, N_PAIRED_ADDR, addr);
}

bool
BluetoothMIDI::find(const BluetoothADDR& v) const
{
    auto end = addr + nAddr;
    return std::find(addr, end, v) != end;
}

//
void
BluetoothAudio::update(const BluetoothADDR& v)
{
    nAddr = updateList(v, nAddr, N_PAIRED_ADDR, addr);
}

bool
BluetoothAudio::find(const BluetoothADDR& v) const
{
    auto end = addr + nAddr;
    return std::find(addr, end, v) != end;
}

//
SystemSettings&
SystemSettings::instance()
{
    static SystemSettings inst;
    return inst;
}

const char*
toString(SoundModule v)
{
    switch (v)
    {
    case SoundModule::AUTO:
        return get(strings::_auto);

    case SoundModule::NOTHING:
        return get(strings::nothing);

    case SoundModule::YM2151:
        return "YM2151";

    case SoundModule::YMF288:
        return "YMF288";

    default:
        return "Unknown";
    }
}

const char*
toString(Language l)
{
    switch (l)
    {
    case Language::JAPANESE:
        return get(strings::japanese);

    case Language::ENGLISH:
        return get(strings::english);

    default:
        return "Unknown";
    }
}

const char*
toString(RepeatMode m)
{
    switch (m)
    {
    case RepeatMode::ALL:
        return get(strings::all);

    case RepeatMode::SINGLE:
        return get(strings::single);

    case RepeatMode::NONE:
        return get(strings::none);

    default:
        return "Unknown";
    }
}

const char*
toString(PlayerDialMode m)
{
    switch (m)
    {
    case PlayerDialMode::VOLUME:
        return get(strings::volumeAdj);

    case PlayerDialMode::TRACK_SELECT:
        return get(strings::trackSelect);

    default:
        return "Unknown";
    }
}

const char*
toString(DeltaSigmaMode m)
{
    switch (m)
    {
    case DeltaSigmaMode::ORDER_1ST:
        return get(strings::_1stOrder);

    case DeltaSigmaMode::ORDER_3RD:
        return get(strings::_3rdOrder);

    default:
        return "Unknown";
    }
}

const char*
toString(InitialBTMode m)
{
    switch (m)
    {
    case InitialBTMode::DISABLE:
        return get(strings::disable);

    case InitialBTMode::_30SEC:
        return get(strings::_30sec);

    case InitialBTMode::_60SEC:
        return get(strings::_60sec);

    case InitialBTMode::ALWAYS:
        return get(strings::always);

    default:
        return "Unknown";
    }
}

const char*
toString(NeoPixelMode m)
{
    switch (m)
    {
    default:
        return get(strings::disable);

    case NeoPixelMode::SIMPLE_LV:
        return get(strings::simpleLvMeter);

    case NeoPixelMode::GAMING_LV:
        return get(strings::gamingLvMeter);

    case NeoPixelMode::SPECTRUM:
        return get(strings::spectrumMeter);
    }
}

////
void
SystemSettings::applyBackLightIntensity() const
{
    graphics::getDisplay().setBackLightIntensity(backLightIntensity_ * 255 /
                                                 100);
}

void
SystemSettings::applyDeltaSigmaMode() const
{
    audio::setInternalSpeaker3rdDeltaSigmaMode(deltaSigmaMode_ ==
                                               DeltaSigmaMode::ORDER_3RD);
}

void
SystemSettings::applyInternalSpeakerMode() const
{
    auto& m = audio::AudioOutDriverManager::instance();
    if (isEnabledInternalSpeaker())
    {
        if (!m.getDriver())
        {
            // 何もない時だけ attach する
            audio::attachInternalSpeaker();
        }
    }
    else
    {
        audio::detachInternalSpeaker();
    }
}

void
SystemSettings::applyYMF288Volume() const
{
    audio::setYMF288FMVolume(ymf288FMVol_);
    audio::setYMF288RhythmVolume(ymf288RhythmVol_);
}

namespace
{

void
applySoundModule(M5DX::ModuleID id)
{
    switch (id)
    {
    case M5DX::ModuleID::YM2151:
        audio::attachYM2151();
        break;

    case M5DX::ModuleID::YMF288:
        audio::atatchYMF288();
        break;

    default:
        audio::detachAllChip();
        break;
    }
}

} // namespace

void
SystemSettings::applySoundModuleType() const
{
    switch (soundModule_)
    {
    case SoundModule::AUTO:
        applySoundModule(M5DX::readModuleID());
        break;

    case SoundModule::YM2151:
        applySoundModule(M5DX::ModuleID::YM2151);
        break;

    case SoundModule::YMF288:
        applySoundModule(M5DX::ModuleID::YMF288);
        break;

    default:
        audio::detachAllChip();
        break;
    }
}

void
SystemSettings::apply() const
{
    applyBackLightIntensity();
    applyDeltaSigmaMode();
    applyYMF288Volume();
    applySoundModuleType();
}

namespace
{
constexpr const char* NVS_NAME = "M5DX";
}

void
SystemSettings::storeToNVS() const
{
    DBOUT(("store settings to NVS\n"));
    sys::NVSHandler nvs(NVS_NAME, true);
    if (nvs)
    {
        nvs.setInt("lang", static_cast<int>(language_));
        nvs.setInt("mod", static_cast<int>(soundModule_));
        // nvs.setInt("vol", volume_);
        nvs.setInt("loop", loopCount_);
        nvs.setBool("shuf", shuffleMode_);
        nvs.setInt("rep", static_cast<int>(repeatMode_));
        nvs.setInt("blint", backLightIntensity_);
        nvs.setBool("intsp", internalSpeaker_);
        nvs.setBool("revdispoff", displayOffIfReverse_);
        nvs.setInt("dial", static_cast<int>(playerDial_));
        nvs.setInt("dsmode", static_cast<int>(deltaSigmaMode_));
        nvs.setInt("288fmvol", ymf288FMVol_);
        nvs.setInt("288ryvol", ymf288RhythmVol_);
        nvs.setInt("neopixmode", static_cast<int>(neoPixelMode_));
        nvs.setInt("neopixbl", neoPixelBrightness_);

        btMIDI_.storeTo(nvs);
        btAudio_.storeTo(nvs);
    }
}

void
SystemSettings::loadFromNVS()
{
    DBOUT(("load settings from NVS\n"));
    sys::NVSHandler nvs(NVS_NAME, false);
    if (nvs)
    {
        if (auto v = nvs.getInt("lang"))
        {
            language_ = static_cast<Language>(v.value());
        }
        if (auto v = nvs.getInt("mod"))
        {
            soundModule_ = static_cast<SoundModule>(v.value());
        }
        // if (auto v = nvs.getInt("vol"))
        // {
        //     volume_ = v.value();
        // }
        if (auto v = nvs.getInt("loop"))
        {
            loopCount_ = v.value();
        }
        if (auto v = nvs.getBool("shuf"))
        {
            shuffleMode_ = v.value();
        }
        if (auto v = nvs.getInt("rep"))
        {
            repeatMode_ = static_cast<RepeatMode>(v.value());
        }
        if (auto v = nvs.getInt("blint"))
        {
            backLightIntensity_ = v.value();
        }
        if (auto v = nvs.getBool("intsp"))
        {
            internalSpeaker_ = v.value();
        }
        if (auto v = nvs.getBool("revdispoff"))
        {
            displayOffIfReverse_ = v.value();
        }
        if (auto v = nvs.getInt("dial"))
        {
            playerDial_ = static_cast<PlayerDialMode>(v.value());
        }
        if (auto v = nvs.getInt("dsmode"))
        {
            deltaSigmaMode_ = static_cast<DeltaSigmaMode>(v.value());
        }
        if (auto v = nvs.getInt("288fmvol"))
        {
            ymf288FMVol_ = v.value();
        }
        if (auto v = nvs.getInt("288ryvol"))
        {
            ymf288RhythmVol_ = v.value();
        }
        if (auto v = nvs.getInt("neopixmode"))
        {
            neoPixelMode_ = static_cast<NeoPixelMode>(v.value());
        }
        if (auto v = nvs.getInt("neopixbl"))
        {
            neoPixelBrightness_ = v.value();
        }

        btMIDI_.loadFrom(nvs);
        btAudio_.loadFrom(nvs);
    }
}

////////////

void
BluetoothMIDI::storeTo(sys::NVSHandler& nvs) const
{
    nvs.setInt("btmidi_intmode", static_cast<int>(initialMode));
}

void
BluetoothMIDI::loadFrom(sys::NVSHandler& nvs)
{
    if (auto v = nvs.getInt("btmidi_intmode"))
    {
        initialMode = static_cast<InitialBTMode>(v.value());
    }
}

////////////

void
BluetoothAudio::storeTo(sys::NVSHandler& nvs) const
{
    nvs.setInt("bta_intmode", static_cast<int>(initialMode));
}

void
BluetoothAudio::loadFrom(sys::NVSHandler& nvs)
{
    if (auto v = nvs.getInt("bta_intmode"))
    {
        initialMode = static_cast<InitialBTMode>(v.value());
    }
}

} // namespace ui
