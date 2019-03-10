/*
 * author : Shuichi TAKANO
 * since  : Sat Mar 09 2019 11:16:59
 */

#include "setting_window.h"
#include "context.h"
#include "dialog.h"
#include "strings.h"
#include "system_setting.h"
#include "ui_manager.h"
#include <array>
#include <assert.h>
#include <debug.h>
#include <functional>
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
                     const std::array<T, N>& values)
    {
        setTitle(title);

        for (size_t i = 0; i < N; ++i)
        {
            items_[i].setText(toString(values[i]));
            append(&items_[i]);
            if (values[i] == current)
            {
                getList().setIndex(i);
            }
        }

        getList().setDecideFunc([values, setFunc](UpdateContext& ctx, int i) {
            if (i >= 0 && i < N)
            {
                setFunc(ctx, values[i]);
                ctx.popManagedUI();
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

ListItem<Language, 2> languageItem(
    [] { return get(strings::language); },
    [] { return SystemSettings::instance().getLanguage(); },
    [](UpdateContext&, auto l) { SystemSettings::instance().setLanguage(l); },
    [](auto l) { return toString(l); },
    {Language::ENGLISH, Language::JAPANESE});

ListItem<SoundModule, 3> soundModuleItem(
    [] { return get(strings::soundModule); },
    [] { return SystemSettings::instance().getSoundModule(); },
    [](UpdateContext&, auto v) {
        SystemSettings::instance().setSoundModule(v);
    },
    [](auto v) { return toString(v); },
    {SoundModule::AUTO, SoundModule::YM2151, SoundModule::YMF288});

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
        p->appendButton(get(strings::yes),
                        [](UpdateContext&) { DBOUT(("write!!\n")); });

        um->push(p);
    }
}

ListItem<SoundModule, 2>
    writeModuleIDItem([] { return get(strings::writeModuleID); },
                      [] {
                          static SoundModule mod = SoundModule::YM2151;
                          return mod;
                      },
                      [](UpdateContext& ctx, auto v) { writeModuleID(ctx, v); },
                      [](auto v) { return toString(v); },
                      {SoundModule::YM2151, SoundModule::YMF288});

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
    },
    [](auto m) { return getEnableDisableString(m); },
    {false, true});

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

ListItem<int, 5> backLightItem(
    [] { return get(strings::backLightIntensity); },
    [] { return SystemSettings::instance().getBackLightIntensity(); },
    [](UpdateContext&, auto v) {
        SystemSettings::instance().setBackLightIntensity(v);
    },
    [](auto v) { return toString(v); },
    {20, 40, 60, 80, 100});

ListItem<RepeatMode, 3> repeatModeItem(
    [] { return get(strings::repeatMode); },
    [] { return SystemSettings::instance().getRepeatMode(); },
    [](UpdateContext&, auto v) { SystemSettings::instance().setRepeatMode(v); },
    [](auto v) { return toString(v); },
    {RepeatMode::NONE, RepeatMode::ALL, RepeatMode::SINGLE});

ListItem<bool, 2> autoBTMIDIItem(
    [] { return get(strings::autoBTMIDI); },
    [] { return SystemSettings::instance().getBluetoothMIDI().autoConnect; },
    [](UpdateContext&, auto m) {
        SystemSettings::instance().getBluetoothMIDI().autoConnect = m;
    },
    [](auto m) { return getEnableDisableString(m); },
    {false, true});

ListItem<bool, 2> autoBTAudioItem(
    [] { return get(strings::autoBTAudio); },
    [] { return SystemSettings::instance().getBluetoothAudio().autoConnect; },
    [](UpdateContext&, auto m) {
        SystemSettings::instance().getBluetoothAudio().autoConnect = m;
    },
    [](auto m) { return getEnableDisableString(m); },
    {false, true});

} // namespace

SettingWindow::SettingWindow()
{
    append(&loopCountItem);
    append(&shuffleModeItem);
    append(&repeatModeItem);
    append(&backLightItem);
    append(&internalSpeakerItem);
    append(&playerDialModeItem);
    append(&dispOffReverseItem);
    append(&autoBTAudioItem);
    append(&autoBTMIDIItem);
    append(&soundModuleItem);
    append(&writeModuleIDItem);
    append(&languageItem);
}

void
SettingWindow::onUpdate(UpdateContext& ctx)
{
    setTitle(get(strings::settings));
    getList().setDecideText(get(strings::select));
    super::onUpdate(ctx);
}

} // namespace ui
