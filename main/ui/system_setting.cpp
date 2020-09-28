/*
 * author : Shuichi TAKANO
 * since  : Mon Feb 11 2019 19:3:50
 */

#include "system_setting.h"
#include "strings.h"
#include <algorithm>

#include <audio/audio.h>
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

} // namespace ui
