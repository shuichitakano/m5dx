/*
 * author : Shuichi TAKANO
 * since  : Sun Mar 03 2019 2:26:6
 */

#include "player_window.h"
#include "button_tip.h"
#include "context.h"
#include "file_window.h"
#include "key.h"
#include "setting_window.h"
#include "strings.h"
#include "system_setting.h"
#include "ui_manager.h"
#include <algorithm>
#include <audio/audio_out.h>
#include <music_player/music_player_manager.h>
#include <mutex>
#include <system/mutex.h>

namespace ui
{

namespace
{
constexpr Dim2 widgetSize_ = {WindowSettings::SCREEN_WIDTH,
                              WindowSettings::SCREEN_HEIGHT};
}

void
PlayerWindow::onUpdate(UpdateContext& ctx)
{
    auto* bt        = ctx.getButtonTip();
    auto* uiManager = ctx.getUIManager();
    auto* key       = ctx.getKeyState();
    if (key && bt && uiManager && ctx.isEnableInput())
    {
        ctx.disableInput();
        std::lock_guard<sys::Mutex> lock(music_player::getMutex());

        auto* mp       = music_player::getActiveMusicPlayer();
        bool playing   = mp && !mp->isFinished() && !mp->isPaused();
        bool longLeft  = key->isLongPress(0);
        bool longRight = key->isLongPress(2);

        bool prevCaptured_ = longLeftCaptured_ || longRightCaptured_;

        longLeftCaptured_ =
            longLeft && (longLeftCaptured_ || !longRightCaptured_);
        longRightCaptured_ =
            longRight && (longRightCaptured_ || !longLeftCaptured_);

        static constexpr int minVol = -30;
        static constexpr int maxVol = 6;

        auto& ss = SystemSettings::instance();
        int dial = key->getDial();
        if (dial && ss.getPlayerDialMode() == PlayerDialMode::VOLUME)
        {
            auto v = ss.getVolume();
            v      = std::min(maxVol, std::max(minVol, v + dial));
            ss.setVolume(v);
        }

        if (longLeftCaptured_)
        {
            bt->set(0, get(strings::volumeAdj));
            bt->set(1, get(strings::volDown));
            bt->set(2, get(strings::volUp));

            auto v = ss.getVolume();
            if (key->isTrigger(1))
            {
                v = std::max(v - 1, minVol);
                ss.setVolume(v);
            }
            if (key->isTrigger(2))
            {
                v = std::min(v + 1, maxVol);
                ss.setVolume(v);
            }
        }
        else if (longRightCaptured_)
        {
            bt->set(0, get(strings::prev));
            bt->set(1, get(strings::next));
            bt->set(2, get(strings::selectSong));

            if (key->isReleaseEdge(0))
            {
                music_player::prevOrRewindPlayList();
            }
            if (key->isReleaseEdge(1))
            {
                music_player::nextPlayList(true);
            }
        }
        else if (!prevCaptured_)
        {
            if (playing)
            {
                bt->set(0, get(strings::settings));
                bt->set(1, get(strings::stop));
                bt->set(2, get(strings::next));

                if (key->isReleaseEdge(0))
                {
                    uiManager->push(std::make_shared<SettingWindow>());
                }
                if (key->isReleaseEdge(1))
                {
                    if (mp)
                    {
                        mp->pause();
                    }
                }
                if (key->isReleaseEdge(2))
                {
                    music_player::nextPlayList(true);
                }
            }
            else
            {
                bt->set(0, get(strings::settings));
                bt->set(1, get(strings::play));
                bt->set(2, get(strings::selectFile));

                if (key->isReleaseEdge(0))
                {
                    uiManager->push(std::make_shared<SettingWindow>());
                }
                if (key->isReleaseEdge(1))
                {
                    if (mp)
                    {
                        mp->play();
                    }
                }
                if (key->isReleaseEdge(2))
                {
                    auto curFile = music_player::getCurrentPlayFile();
                    uiManager->push(std::make_shared<FileWindow>(curFile));
                }
            }
        }
    }
}

void
PlayerWindow::onRender(RenderContext& ctx)
{
    bool redraw = needRefresh_ || ctx.isInvalidated(widgetSize_);
    if (redraw)
    {
        ctx.applyClipRegion();
        ctx.updateInvalidatedRegion(getSize());

        ctx.fill({0, 0}, widgetSize_, 0x400000);
        needRefresh_ = false;
    }

    auto* player = music_player::getActiveMusicPlayer();
    auto& ss     = SystemSettings::instance();
    auto& fm     = ctx.getFontManager();
    fm.setEdgedMode(false);
    fm.setTransparentMode(true);

    {
        auto title = player ? player->getTitle() : std::string();
        if (redraw || title_ != title)
        {
            title_ = title;

            static constexpr Vec2 pos       = {0, 200};
            static constexpr Dim2 size      = {320, 8};
            static constexpr uint32_t col   = 0xffc080;
            static constexpr uint32_t bgcol = 0x200000;
            ctx.fill(pos, size, bgcol);
            ctx.setFontColor(col);
            ctx.putText(title.c_str(), pos, size);
        }
    }

    fm.setTransparentMode(false);
    char buf[100];
    snprintf(buf, sizeof(buf), "vol %d db", ss.getVolume());
    ctx.setFontColor(0xffffff);
    ctx.putText(buf, {128, 0}, {100, 8});

    auto& tmpFB = ctx.getTemporaryFrameBuffer({128, 128});
    {
        auto recoverFB = ctx.updateFrameBuffer(&tmpFB);
        ctx.applyClipRegion();
        tmpFB.fill(tmpFB.makeColor(0x0, 0, 128));
        auto red   = tmpFB.makeColor(255, 0, 0);
        auto green = tmpFB.makeColor(0, 255, 0);

        audio::AudioOutDriverManager::instance().lockHistoryBuffer();
        static std::array<int16_t, 2> tmpWave[128];
        audio::AudioOutDriverManager::instance().getHistoryBuffer().copyLatest(
            tmpWave, 128);
        audio::AudioOutDriverManager::instance().unlockHistoryBuffer();

        auto* wave = tmpWave;
        for (int i = 0; i < 128; ++i)
        {
            int y0 = std::min(127, std::max(0, 64 + (int)((*wave)[0] >> 9)));
            int y1 = std::min(127, std::max(0, 64 + (int)((*wave)[1] >> 9)));
            tmpFB.setPixel(i, y0, red);
            tmpFB.setPixel(i, y1, green);
            ++wave;
        }
    }
    ctx.applyClipRegion();
    ctx.put({0, 0}, tmpFB);
}

Dim2
PlayerWindow::getSize() const
{
    return widgetSize_;
}

} // namespace ui
