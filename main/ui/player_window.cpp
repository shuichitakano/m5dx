/*
 * author : Shuichi TAKANO
 * since  : Sun Mar 03 2019 2:26:6
 */

#include "player_window.h"
#include "button_tip.h"
#include "context.h"
#include "draw_util.h"
#include "file_window.h"
#include "key.h"
#include "keyboard_list.h"
#include "setting_window.h"
#include "strings.h"
#include "system_setting.h"
#include "ui_manager.h"
#include <algorithm>
#include <audio/audio_out.h>
#include <audio/roudness.h>
#include <music_player/music_player_manager.h>
#include <mutex>
#include <system/mutex.h>
#include <util/fft.h>
#include <util/sin_table.h>

namespace ui
{

namespace
{

constexpr Dim2 widgetSize_ = {320, 52};
constexpr Vec2 panelPos_   = {0, 180};

namespace tex
{
constexpr Rect logo     = {{244, 0}, {76, 25}};
constexpr Rect iconLoop = {{11 * 8, 0}, {8, 8}};
constexpr Rect iconSong = {{96 + 11 * 7, 0}, {7, 8}};
constexpr Rect font8x8  = {{0, 0}, {8, 8}};
constexpr Rect font7x8  = {{96, 0}, {7, 8}};
} // namespace tex

namespace pos
{
constexpr Vec2 logo = {1, 184 - 180};
// constexpr Vec2 iconLoop = {1, 213 - 180};
// constexpr Vec2 iconSong = {17, 213 - 180};
// constexpr Vec2 loopText = {9, 213 - 180};
// constexpr Vec2 songText = {24, 213 - 180};
constexpr Vec2 iconLoop = {24, 213 - 180};
constexpr Vec2 iconSong = {0, 213 - 180};
constexpr Vec2 loopText = {32, 213 - 180};
constexpr Vec2 songText = {7, 213 - 180};
constexpr Vec2 time     = {43, 213 - 180};
constexpr Vec2 title    = {1, 224 - 180};
} // namespace pos

namespace rect
{
// constexpr Rect loop     = {{1, 213 - 180}, {24, 8}};
// constexpr Rect song     = {{17, 213 - 180}, {23, 8}};
constexpr Rect loop     = {{24, 213 - 180}, {24, 8}};
constexpr Rect song     = {{0, 213 - 180}, {23, 8}};
constexpr Rect loopText = {{9, 213 - 180}, {16, 8}};
constexpr Rect songText = {{24, 213 - 180}, {16, 8}};
constexpr Rect timeText = {{43, 213 - 180}, {35, 8}};
constexpr Rect title    = {{0, 224 - 180}, {320, 8}};

constexpr Rect waveFB   = {{78, 4}, {320 - 78, 36}};
constexpr Rect waveL    = {{0, 0}, {64, 36}};
constexpr Rect waveR    = {{68, 0}, {64, 36}};
constexpr Rect freqView = {{136, 0}, {106, 36}};

constexpr Rect vol = {{56, 28}, {20, 8}};
} // namespace rect

namespace col
{
constexpr uint32_t panel      = 0x202020;
constexpr uint32_t titleFont  = 0xffffff;
constexpr uint32_t icon       = 0xd0d0d0;
constexpr uint32_t statusFont = 0xc0c0c0;
constexpr uint32_t wave       = 0xd0d0d0;
constexpr uint32_t freqLine   = 0xb0b0b0;
constexpr uint32_t freqBar    = 0x404040;
} // namespace col

}; // namespace

PlayerWindow::PlayerWindow()
    : keyboardList_(std::make_shared<KeyboardList>())
{
    constexpr float baseSpectrumScale_ = 2.3e-10f;
    roudnessTable_.resize(rect::freqView.size.w);
    for (auto i = 0u; i < roudnessTable_.size(); ++i)
    {
        float freq =
            55.0f *
            powf(powf(7040.0f / 55.0f, 1.0f / roudnessTable_.size()), (float)i);
        roudnessTable_[i] = audio::computeRoudness(freq) * baseSpectrumScale_;
    }
}

void
PlayerWindow::show(UIManager& m)
{
    m.push(keyboardList_, {0, 0});
    m.push(shared_from_this(), panelPos_);
}

void
PlayerWindow::onUpdate(UpdateContext& ctx)
{
    float dt    = ctx.getDeltaT();
    currentDt_  = dt;
    volVisible_ = std::max(0.0f, volVisible_ - dt);

    auto* bt        = ctx.getButtonTip();
    auto* uiManager = ctx.getUIManager();
    auto* key       = ctx.getKeyState();
    if (key && bt && uiManager && ctx.isEnableInput())
    {
        //        ctx.disableInput(); // keyboardlist に託す
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

        static constexpr int minVol          = -30;
        static constexpr int maxVol          = 6;
        static constexpr float volVisibleSec = 1.0f;

        auto& ss = SystemSettings::instance();
        int dial = key->getDial();
        if (dial && ss.getPlayerDialMode() == PlayerDialMode::VOLUME)
        {
            auto v = ss.getVolume();
            v      = std::min(maxVol, std::max(minVol, v + dial));
            ss.setVolume(v);
            volVisible_ = volVisibleSec;
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
                volVisible_ = volVisibleSec;
            }
            if (key->isTrigger(2))
            {
                v = std::min(v + 1, maxVol);
                ss.setVolume(v);
                volVisible_ = volVisibleSec;
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

namespace
{

template <class F>
void
drawGraph(graphics::FrameBufferBase& fb,
          const Rect& r,
          F&& func,
          int xshift,
          bool bar,
          uint16_t lineColor,
          uint16_t barColor = 0)
{
    int prevY = 0;
    int n     = r.size.w << xshift;
    for (int i = 0; i < n; ++i)
    {
        int v = func(i);
        int x = (i >> xshift) + r.pos.x;
        int y = v + r.pos.y;

        if (i && prevY < y - 1)
        {
            fb.fill(x, prevY + 1, 1, y - prevY, lineColor);
        }
        else if (i && prevY > y + 1)
        {
            fb.fill(x, y, 1, prevY - y, lineColor);
        }
        else
        {
            fb.setPixel(x, y, lineColor);
        }

        if (bar)
        {
            int by0 = std::max(prevY, y + 1);
            int bh  = r.size.h - by0;
            if (bh > 0)
            {
                fb.fill(x, by0, 1, bh, barColor);
            }
        }

        prevY = y;
    }
}

} // namespace

void
PlayerWindow::onRender(RenderContext& ctx)
{
    ctx.applyClipRegion();
    bool redraw = needRefresh_ || ctx.isInvalidated(widgetSize_);
    ctx.updateInvalidatedRegion(getSize());

    if (redraw)
    {
        ctx.fill({0, 0}, widgetSize_, col::panel);
        ctx.putTextureTrans(pos::logo, tex::logo);
        needRefresh_ = false;
    }

    auto* player = music_player::getActiveMusicPlayer();
    auto& fm     = ctx.getFontManager();
    fm.setEdgedMode(false);
    fm.setTransparentMode(true);

    {
        auto title = player ? player->getTitle() : std::string();
        if (redraw || title_ != title)
        {
            title_ = title;
            ctx.fill(rect::title, col::panel);
            ctx.setFontColor(col::titleFont);
            ctx.putText(title.c_str(), rect::title);
        }
    }
    if (player)
    {
        int loop = std::min(player->getCurrentLoop() + 1, 9);
        if (redraw || currentLoop_ != loop)
        {
            ctx.applyClipRegion();
            ctx.updateInvalidatedRegion(getSize());

            currentLoop_ = loop;
            if (loop <= 0)
            {
                ctx.fill(rect::loop, col::panel);
            }
            else
            {
                char tmp[16];
                snprintf(tmp, sizeof(tmp), "%d", loop);
                ctx.putTextureText("@",
                                   '@',
                                   pos::iconLoop,
                                   tex::iconLoop,
                                   col::icon,
                                   col::panel);
                ctx.putTextureText(tmp,
                                   '0',
                                   pos::loopText,
                                   tex::font7x8,
                                   col::statusFont,
                                   col::panel);
            }
        }

        // あればtrack番号、なければplaylist indexを表示
        // この仕様でいいかは微妙
        // プレイリスト中の track 番号がある曲は？
        int song = player->getCurrentTrack();
        if (song < 0)
        {
            song = music_player::getCurrentListIndex();
        }
        song = std::min(song + 1, 99);
        if (redraw || currentSong_ != song)
        {
            ctx.applyClipRegion();
            ctx.updateInvalidatedRegion(getSize());

            currentSong_ = song;
            if (song <= 0)
            {
                ctx.fill(rect::song, col::panel);
            }
            else
            {
                char tmp[16];
                snprintf(tmp, sizeof(tmp), "%02d", song);
                ctx.putTextureText("@",
                                   '@',
                                   pos::iconSong,
                                   tex::iconSong,
                                   col::icon,
                                   col::panel);
                ctx.putTextureText(tmp,
                                   '0',
                                   pos::songText,
                                   tex::font7x8,
                                   col::statusFont,
                                   col::panel);
            }
        }

        int time =
            std::min(static_cast<int>(player->getPlayTime()), 99 * 60 + 59);
        if (redraw || currentTime_ != time)
        {
            currentTime_ = time;
            if (time < 0)
            {
                ctx.fill(rect::title, col::panel);
            }
            else
            {
                char tmp[16];
                snprintf(tmp, sizeof(tmp), "%02d:%02d", time / 60, time % 60);
                ctx.putTextureText(tmp,
                                   '0',
                                   pos::time,
                                   tex::font7x8,
                                   col::statusFont,
                                   col::panel);
            }
        }
    }

    fm.setTransparentMode(false);

    auto& tmpFB = ctx.getTemporaryFrameBuffer(rect::waveFB.size);
    {
        auto recoverFB = ctx.updateFrameBuffer(&tmpFB);
        ctx.applyClipRegion();
        ctx.fill({0, 0}, rect::waveFB.size, col::panel);

        std::array<float, 2> total{};

        static int16_t waveBuffer[256];

        auto& audioManager = audio::AudioOutDriverManager::instance();

        // wave
        auto* tmpWave = reinterpret_cast<std::array<int16_t, 2>*>(waveBuffer);
        audioManager.lockHistoryBuffer();
        audioManager.getHistoryBuffer().copyLatest(tmpWave, 128);
        audioManager.unlockHistoryBuffer();

        for (int ch = 0; ch < 2; ++ch)
        {
            constexpr int waveScale = 2;
            constexpr int h         = rect::waveL.size.h;
            constexpr int scale     = h * waveScale;
            constexpr int bias      = (h * 65536) >> 1;
            const auto& r           = ch == 0 ? rect::waveL : rect::waveR;
            const int16_t* wave     = &tmpWave[0][ch];

            drawGraph(
                tmpFB,
                r,
                [&](int) {
                    int v = (*wave * scale + bias) >> 16;
                    total[ch] += *wave * *wave;
                    wave += 2;
                    return v;
                },
                1,
                false,
                ctx.makeColor(col::wave));
        }

        // spectrum analyzer
        const auto* sinCos = util::sincos256Table;
        audioManager.lockHistoryBuffer();
        // 窓関数を掛けつつ1/3に周波数下げてコピー (44100 -> 14700Hz)
        audioManager.getHistoryBuffer().copyLatest(
            waveBuffer, 256, 3, [&](const std::array<int16_t, 2>& v) {
                int wf = 16384 - sinCos[1]; // (1 - cosθ)/2 * 2    [0:32768]
                sinCos += 2;
                return ((v[0] + v[1]) * wf) >> 16; // (15+1)+15-16=15
            });
        audioManager.unlockHistoryBuffer();

        util::realFFT(waveBuffer, util::sincos256Table, 256);
        waveBuffer[0]   = 0;
        waveBuffer[255] = 0;

        // サンプルは 44100/3=14700Hz
        // 14700/256 = 57.421875Hzステップで7350Hzまで
        // 55, 110, 220, 440, 880, 1760, 3520, 7040Hz に点
        // 15*7 + 1 = 106 pix
        // freq(x) = 55 * pow(pow(7040/55, 1/w), x)
        // i = freq(x) / (14700/256)

        constexpr float l7040_55 = 4.852030263919617f; // log(7040/55)
        const float es =
            l7040_55 / rect::freqView.size.w; // log(pow(7040/55, 1/w))
        const float ls = 55.0f / (44100.0f / 3 / 256);

        // n db を h pixel で
        // -20 * log10(v^1/2) / n * h
        // -10 * log(v) / log(10) / n * h
        constexpr float il10 = 0.43429448190325176f; // 1/log(10)
        constexpr float nDB  = 50.0f;
        const float vs       = (-10.0f * il10 / nDB) * rect::freqView.size.h;

        drawGraph(
            tmpFB,
            rect::freqView,
            [&](int i) {
                float spos = ls * expf(es * i);
                int idx    = static_cast<int>(spos);
                float t    = spos - idx;

                const auto* pr = &waveBuffer[idx];
                const auto* pi = &waveBuffer[256 - 1 - idx];
                int32_t v0     = pr[0] * pr[0] + pi[0] * pi[0];
                int32_t v1     = pr[1] * pr[1] + pi[-1] * pi[-1];
                float v        = ((v1 - v0) * t + v0) * roudnessTable_[i];

                float lv = logf(std::max(0.000001f, std::min(1.0f, v))) * vs;
                return static_cast<int>(lv);
            },
            0,
            true,
            ctx.makeColor(col::freqLine),
            ctx.makeColor(col::freqBar));

        // neo pix
        float lvs = 1.0f / (32768.0f * 32768.0f * 128.0f);
        std::array<float, 2> lv{total[0] * lvs, total[1] * lvs};
        neoPixelDisp_.update(lv, waveBuffer, 256, currentDt_);

        // vol
        if (volVisible_ > 0)
        {
            auto& ss = SystemSettings::instance();
            int vol  = ss.getVolume();
            char buf[16];
            snprintf(buf, sizeof(buf), "%ddB", vol);
            ctx.setFontColor(0xffffff);
            ctx.putText(buf, rect::vol.pos, rect::vol.size);
        }
    }
    ctx.applyClipRegion();
    ctx.put(rect::waveFB.pos, tmpFB);
}

Dim2
PlayerWindow::getSize() const
{
    return widgetSize_;
}

} // namespace ui
