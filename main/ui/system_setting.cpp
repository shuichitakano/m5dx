/*
 * author : Shuichi TAKANO
 * since  : Mon Feb 11 2019 19:3:50
 */

#include "system_setting.h"
#include "strings.h"

namespace ui
{

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

} // namespace ui
