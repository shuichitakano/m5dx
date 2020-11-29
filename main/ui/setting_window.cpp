/*
 * author : Shuichi TAKANO
 * since  : Sat Mar 09 2019 11:16:59
 */

#include "setting_window.h"
#include "bt_audio_window.h"
#include "context.h"
#include "dialog.h"
#include "strings.h"
#include "system_setting.h"
#include "ui_manager.h"
#include <array>
#include <assert.h>
#include <debug.h>
#include <functional>
#include <m5dx_module.h>
#include <string>

namespace ui
{

namespace
{

template <class T, int N>
class ListSelectWindow final : public SimpleListWindow
{
    std::array<SimpleList::TextItem, N> items_;

    using Func = std::function<void(UpdateContext&, T)>;

public:
    template <class ToString>
    ListSelectWindow(const std::string& title,
                     T current,
                     const Func& setFunc,
                     const ToString& toString,
                     const std::array<T, N>& values,
                     bool closeOnDecide = true,
                     bool useCancel     = false)
    {
        setTitle(title);

        int ofs = 0;
        if (useCancel)
        {
            getList().appendCancel();
            ofs = 1;
        }

        for (size_t i = 0; i < N; ++i)
        {
            items_[i].setText(toString(values[i]));
            append(&items_[i]);
            if (values[i] == current)
            {
                getList().setIndex(i + ofs);
            }
        }

        getList().setDecideFunc([=](UpdateContext& ctx, int i) {
            if (i == getList().getCancelIndex())
            {
                ctx.popManagedUI();
            }
            if (i >= ofs && i < N + ofs)
            {
                setFunc(ctx, values[i - ofs]);
                if (closeOnDecide)
                {
                    ctx.popManagedUI();
                }
            }
        });
    }
};

/*
auto createLanguageWindow = [] {
    return new ListSelectWindow<Language, 2>(
        get(strings::language),
        SystemSettings::instance().getLanguage(),
        [](auto l) { SystemSettings::instance().setLanguage(l); },
        [](auto l) { return toString(l); },
        {Language::JAPANESE, Language::ENGLISH});
};

static constexpr std::array<Language, 2> table{Language::JAPANESE,
                                               Language::ENGLISH};

class LanguageWindow final : public SimpleListWindow
{
    SimpleList::TextItem japanese_{get(strings::japanese)};
    SimpleList::TextItem english_{get(strings::english)};

public:
    LanguageWindow()
    {
        setTitle(get(strings::settings));
        append(&japanese_);
        append(&english_);

        auto p = std::find(std::begin(table),
                           std::end(table),
                           SystemSettings::instance().getLanguage());
        if (p != std::end(table))
        {
            getList().setIndex(p - std::begin(table));
        }

        getList().setDecideFunc([](UpdateContext& ctx, int i) {
            if (auto uiManager = ctx.getUIManager())
            {
                assert(i >= 0 && i < 2);
                SystemSettings::instance().setLanguage(table[i]);
                uiManager->pop();
            }
        });
    }
};

class LanguageItem final : public SimpleList::ItemWithValue
{
public:
    std::string getTitle() const override { return get(strings::language); }
    std::string getValue() const override
    {
        return toString(SystemSettings::instance().getLanguage());
    }
    void decide(UpdateContext& ctx) override
    {
        //
        if (auto* uiManager = ctx.getUIManager())
        {
            //            uiManager->push(std::make_shared<LanguageWindow>());
            uiManager->push(WidgetPtr(createLanguageWindow()));
        }
    }
};
*/

class LabelItem final : public SimpleList::ItemWithTitle
{
    using GetTitleFunc = std::function<std::string()>;
    using DecideFunc   = std::function<void(UpdateContext& ctx)>;

    GetTitleFunc getTitleFunc_;
    DecideFunc decideFunc_;

public:
    LabelItem(const GetTitleFunc& getTitleFunc, const DecideFunc& decideFunc)
        : getTitleFunc_(getTitleFunc)
        , decideFunc_(decideFunc)
    {
    }
    std::string getTitle() const override { return getTitleFunc_(); }
    void decide(UpdateContext& ctx) override { decideFunc_(ctx); }
};

template <class T, size_t N>
class ListItem final : public SimpleList::ItemWithValue
{
    using GetTitleFunc = std::function<std::string()>;
    using GetValueFunc = std::function<T()>;
    using SetValueFunc = std::function<void(UpdateContext&, T)>;
    using ToStringFunc = std::function<std::string(T)>;

    GetTitleFunc getTitleFunc_;
    GetValueFunc getValueFunc_;
    SetValueFunc setValueFunc_;
    ToStringFunc toStringFunc_;
    std::array<T, N> values_;

public:
    ListItem(GetTitleFunc getTitleFunc,
             GetValueFunc getValueFunc,
             SetValueFunc setValueFunc,
             ToStringFunc toStringFunc,
             const std::array<T, N>& values)
        : getTitleFunc_(getTitleFunc)
        , getValueFunc_(getValueFunc)
        , setValueFunc_(setValueFunc)
        , toStringFunc_(toStringFunc)
        , values_(values)
    {
    }

    std::string getTitle() const override { return getTitleFunc_(); }
    std::string getValue() const override
    {
        return toStringFunc_(getValueFunc_());
    }
    void decide(UpdateContext& ctx) override
    {
        if (auto* uiManager = ctx.getUIManager())
        {
            uiManager->push(
                std::make_shared<ListSelectWindow<T, N>>(getTitle(),
                                                         getValueFunc_(),
                                                         setValueFunc_,
                                                         toStringFunc_,
                                                         values_));
        }
    }
};

const char*
getEnableDisableString(bool f)
{
    return get(f ? strings::enable : strings::disable);
}

std::string
toString(int v)
{
    //    return std::to_string(v);
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", v);
    return buf;
}

std::string
toString(float v)
{
    //    return std::to_string(v);
    char buf[16];
    snprintf(buf, sizeof(buf), "%.2f", v);
    return buf;
}

ListItem<Language, 2> languageItem(
    [] { return get(strings::language); },
    [] { return SystemSettings::instance().getLanguage(); },
    [](UpdateContext&, auto l) { SystemSettings::instance().setLanguage(l); },
    [](auto l) { return toString(l); },
    {Language::ENGLISH, Language::JAPANESE});

ListItem<SoundModule, 4>
    soundModuleItem([] { return get(strings::soundModule); },
                    [] { return SystemSettings::instance().getSoundModule(); },
                    [](UpdateContext&, auto v) {
                        SystemSettings::instance().setSoundModule(v);
                    },
                    [](auto v) { return toString(v); },
                    {SoundModule::AUTO,
                     SoundModule::NOTHING,
                     SoundModule::YM2151,
                     SoundModule::YMF288});

void
writeModuleID(UpdateContext& ctx, SoundModule id)
{
    if (auto* um = ctx.getUIManager())
    {
        auto p = std::make_shared<Dialog>(get(strings::writeModuleID),
                                          Dim2{240, 120});
        char buf[512];
        snprintf(buf, sizeof(buf), get(strings::writeModuleMes), toString(id));
        p->setMessage(buf);

        p->appendButton(get(strings::no));
        p->appendButton(get(strings::yes), [id](UpdateContext& ctx) {
            switch (id)
            {
            case SoundModule::YM2151:
                DBOUT(("writeID 2151\n"));
                M5DX::writeModuleID(M5DX::ModuleID::YM2151);
                break;

            case SoundModule::YMF288:
                DBOUT(("writeID 288\n"));
                M5DX::writeModuleID(M5DX::ModuleID::YMF288);
                break;

            default:
                break;
            }
            ctx.popManagedUI(); // もう一つ閉じる
        });

        um->push(p);
    }
}

auto makeWriteModuleListWindow = [] {
    return new ListSelectWindow<SoundModule, 2>(
        get(strings::writeModuleID),
        {},
        [](UpdateContext& ctx, SoundModule v) { writeModuleID(ctx, v); },
        [](SoundModule v) { return toString(v); },
        {SoundModule::YM2151, SoundModule::YMF288},
        false /* close */,
        true /* cancel */);
};

std::string
make3DotString(const std::string& str)
{
    return str + "...";
}

LabelItem writeModuleMenuItem(
    [] { return make3DotString(get(strings::writeModuleID)); },
    [](UpdateContext& ctx) {
        if (auto* uiManager = ctx.getUIManager())
        {
            uiManager->push(WidgetPtr{makeWriteModuleListWindow()});
        }
    });

ListItem<PlayerDialMode, 2> playerDialModeItem(
    [] { return get(strings::playerDiadMode); },
    [] { return SystemSettings::instance().getPlayerDialMode(); },
    [](UpdateContext&, auto v) {
        SystemSettings::instance().setPlayerDialMode(v);
    },
    [](auto v) { return toString(v); },
    {PlayerDialMode::VOLUME, PlayerDialMode::TRACK_SELECT});

ListItem<bool, 2>
    shuffleModeItem([] { return get(strings::shuffleMode); },
                    [] { return SystemSettings::instance().isShuffleMode(); },
                    [](UpdateContext&, auto m) {
                        SystemSettings::instance().setShuffleMode(m);
                    },
                    [](auto m) { return getEnableDisableString(m); },
                    {false, true});

ListItem<bool, 2> internalSpeakerItem(
    [] { return get(strings::internalSpeaker); },
    [] { return SystemSettings::instance().isEnabledInternalSpeaker(); },
    [](UpdateContext&, auto m) {
        SystemSettings::instance().enableInternalSpeaker(m);
        SystemSettings::instance().applyInternalSpeakerMode();
    },
    [](auto m) { return getEnableDisableString(m); },
    {false, true});

ListItem<DeltaSigmaMode, 2> internalSpeaker3rdDeltaSigmaModeItem(
    [] { return get(strings::deltaSigmaMode); },
    [] { return SystemSettings::instance().getDeltaSigmaMode(); },
    [](UpdateContext&, auto m) {
        SystemSettings::instance().setDeltaSigmaMode(m);
        SystemSettings::instance().applyDeltaSigmaMode();
    },
    [](auto m) { return toString(m); },
    {DeltaSigmaMode::ORDER_1ST, DeltaSigmaMode::ORDER_3RD});

ListItem<bool, 2> dispOffReverseItem(
    [] { return get(strings::dispOffReverse); },
    [] { return SystemSettings::instance().isEnabledDisplayOffIfReverse(); },
    [](UpdateContext&, auto m) {
        SystemSettings::instance().enableDisplayOffIfReverse(m);
    },
    [](auto m) { return getEnableDisableString(m); },
    {false, true});

ListItem<int, 5> loopCountItem(
    [] { return get(strings::loopCount); },
    [] { return SystemSettings::instance().getLoopCount(); },
    [](UpdateContext&, auto v) { SystemSettings::instance().setLoopCount(v); },
    [](auto v) { return toString(v); },
    {1, 2, 3, 4, 5});

ListItem<int, 10> backLightItem(
    [] { return get(strings::backLightIntensity); },
    [] { return SystemSettings::instance().getBackLightIntensity(); },
    [](UpdateContext&, auto v) {
        SystemSettings::instance().setBackLightIntensity(v);
        SystemSettings::instance().applyBackLightIntensity();
    },
    [](auto v) { return toString(v); },
    {10, 20, 30, 40, 50, 60, 70, 80, 90, 100});

ListItem<NeoPixelMode, 4> neoPixelModeItem(
    [] { return get(strings::NeoPixelMode); },
    [] { return SystemSettings::instance().getNeoPixelMode(); },
    [](UpdateContext&, auto m) {
        SystemSettings::instance().setNeoPixelMode(m);
    },
    [](auto m) { return toString(m); },
    {NeoPixelMode::OFF,
     NeoPixelMode::SIMPLE_LV,
     NeoPixelMode::GAMING_LV,
     NeoPixelMode::SPECTRUM});

ListItem<int, 10> neoPixelBrightnessItem(
    [] { return get(strings::NeoPixelBrightness); },
    [] { return SystemSettings::instance().getNeoPixelBrightness(); },
    [](UpdateContext&, auto v) {
        SystemSettings::instance().setNeoPixelBrightness(v);
    },
    [](auto v) { return toString(v); },
    {10, 20, 30, 40, 50, 60, 70, 80, 90, 100});

ListItem<int, 13> ymf288FMVolumeItem(
    [] { return get(strings::YMF288FMVolume); },
    [] { return SystemSettings::instance().getYMF288FMVolume(); },
    [](UpdateContext&, auto v) {
        SystemSettings::instance().setYMF288FMVolume(v);
        SystemSettings::instance().applyYMF288Volume();
    },
    [](auto v) { return toString(v * 0.75f) + "dB"; },
    {0, -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12});

ListItem<int, 13> ymf288RhythmVolumeItem(
    [] { return get(strings::YMF288RhythmVolume); },
    [] { return SystemSettings::instance().getYMF288RhythmVolume(); },
    [](UpdateContext&, auto v) {
        SystemSettings::instance().setYMF288RhythmVolume(v);
        SystemSettings::instance().applyYMF288Volume();
    },
    [](auto v) { return toString(v * 0.75f) + "dB"; },
    {0, -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12});

ListItem<RepeatMode, 3> repeatModeItem(
    [] { return get(strings::repeatMode); },
    [] { return SystemSettings::instance().getRepeatMode(); },
    [](UpdateContext&, auto v) { SystemSettings::instance().setRepeatMode(v); },
    [](auto v) { return toString(v); },
    {RepeatMode::NONE, RepeatMode::ALL, RepeatMode::SINGLE});

ListItem<InitialBTMode, 4> bootBTMIDIItem(
    [] { return get(strings::bootBTMIDI); },
    [] { return SystemSettings::instance().getBluetoothMIDI().initialMode; },
    [](UpdateContext&, auto m) {
        SystemSettings::instance().getBluetoothMIDI().initialMode = m;
    },
    [](auto m) { return toString(m); },
    {InitialBTMode::DISABLE,
     InitialBTMode::_30SEC,
     InitialBTMode::_60SEC,
     InitialBTMode::ALWAYS});

LabelItem btAudioMenuItem([] { return make3DotString(get(strings::BTAudio)); },
                          [](UpdateContext& ctx) {
                              if (auto* uiManager = ctx.getUIManager())
                              {
                                  uiManager->push(
                                      std::make_shared<BTAudioWindow>());
                              }
                          });

ListItem<InitialBTMode, 4> bootBTAudioItem(
    [] { return get(strings::bootBTAudio); },
    [] { return SystemSettings::instance().getBluetoothAudio().initialMode; },
    [](UpdateContext&, auto m) {
        SystemSettings::instance().getBluetoothAudio().initialMode = m;
    },
    [](auto m) { return toString(m); },
    {InitialBTMode::DISABLE,
     InitialBTMode::_30SEC,
     InitialBTMode::_60SEC,
     InitialBTMode::ALWAYS});

} // namespace

SettingWindow::SettingWindow()
{
    appendCancel();
    append(&loopCountItem);
    append(&shuffleModeItem);
    append(&repeatModeItem);
    append(&ymf288FMVolumeItem);
    append(&ymf288RhythmVolumeItem);
    append(&backLightItem);
    append(&internalSpeakerItem);
    append(&internalSpeaker3rdDeltaSigmaModeItem);
    append(&playerDialModeItem);
    append(&dispOffReverseItem);
    append(&neoPixelModeItem);
    append(&neoPixelBrightnessItem);
    append(&btAudioMenuItem);
    append(&bootBTAudioItem);
    append(&bootBTMIDIItem);
    append(&soundModuleItem);
    append(&writeModuleMenuItem);
    append(&languageItem);
}

SettingWindow::~SettingWindow()
{
    SystemSettings::instance().storeToNVS();
}

void
SettingWindow::onUpdate(UpdateContext& ctx)
{
    setTitle(get(strings::settings));
    getList().setDecideText(get(strings::select));
    super::onUpdate(ctx);
}

} // namespace ui
