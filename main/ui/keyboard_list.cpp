/*
 * author : Shuichi TAKANO
 * since  : Sat Sep 12 2020 23:06:18
 */

#include "keyboard_list.h"
#include "context.h"
#include "key.h"
#include "system_setting.h"
#include <algorithm>
#include <debug.h>
#include <music_player/music_player_manager.h>
#include <sound_sys/sound_system.h>
#include <utility>

namespace ui
{

namespace
{
constexpr Dim2 widgetSize_ = {320, 180};
constexpr Dim2 listSize_   = {320, 20};
} // namespace

KeyboardList::KeyboardList()
{
    items_.reserve(64);

    setDirectionIsVertical(true);
    setNeedScrollBar(false);
    enableButton(false);
    enableNoSelect(true);
    setIndex(-1);

    // ch mute
    setDecideFunc([this](UpdateContext& ctx, int idx) {
        auto& item = items_[idx];
        if (auto* sys = item.soundSys_)
        {
            bool enabled = sys->getEnabledChannels() & (1 << item.ch_);
            sys->mute(item.ch_, enabled);
        }
        touchSelectWidget();
    });

    // 全mute解除
    setLongPressFunc([this](UpdateContext&, int) {
        for (auto& item : items_)
        {
            if (auto* sys = item.soundSys_)
            {
                sys->mute(item.ch_, false);
            }
        }
        refresh();
    });
}

Dim2
KeyboardList::getSize() const
{
    return widgetSize_;
}

uint16_t
KeyboardList::getBaseItemSize() const
{
    return listSize_.h;
}

size_t
KeyboardList::getWidgetCount() const
{
    return items_.size();
}

const Widget*
KeyboardList::getWidget(size_t i) const
{
    return i < items_.size() ? &items_[i] : nullptr;
}

Widget*
KeyboardList::getWidget(size_t i)
{
    return i < items_.size() ? &items_[i] : nullptr;
}

void
KeyboardList::onUpdate(UpdateContext& ctx)
{
    super::onUpdate(ctx);

    auto* player = music_player::getActiveMusicPlayer();
    if (player)
    {
        int itemIdx   = 0;
        int systemIdx = 0;
        while (auto* soundSys = player->getSystem(systemIdx++))
        {
            soundSys->fetch();

            auto& info = soundSys->getSystemInfo();
            int nCh    = info.channelCount;

            for (int ch = 0; ch < nCh; ++ch)
            {
                if (itemIdx >= static_cast<int>(items_.size()))
                {
                    items_.resize(itemIdx + 1);
                }
                auto& item = items_[itemIdx];

                if (item.soundSys_ != soundSys || item.ch_ != ch)
                {
                    item.soundSys_ = soundSys;
                    item.ch_       = ch;
                    item.updated_  = true;
                }

                ++itemIdx;
            }
        }
    }

#if 0
    auto* key = ctx._getKeyState();
    if (key)
    {
        auto& ss = SystemSettings::instance();
        int dial = key->getDial();

        if (ss.getPlayerDialMode() == PlayerDialMode::TRACK_SELECT)
        {
            if (dial)
            {
                auto n  = static_cast<int>(items_.size());
                cursor_ = std::min(n - 1, std::max(-1, cursor_ - dial));
                setIndex(cursor_);
            }
            if (key->isPressEdge(3) && cursor_ >= 0)
            {
                auto& item = items_[cursor_];
                auto* sys  = item.soundSys_;
                if (sys)
                {
                    bool enabled = sys->getEnabledChannels() & (1 << item.ch_);
                    sys->mute(item.ch_, enabled);
                }
            }
        }
    }
#endif
}

//////////////
KeyboardList::Item::Item()
{
    curOnKeys_.reserve(8);
}

Dim2
KeyboardList::Item::getSize() const
{
    return listSize_;
}

void
KeyboardList::Item::onUpdate(UpdateContext& ctx)
{
    if (!soundSys_)
    {
        return;
    }

    float dt                  = ctx.getDeltaT();
    constexpr float decPerSec = 2.0f;
    for (auto& lv : curLevels_)
    {
        lv = std::max(0.0f, lv - decPerSec * dt);
    }

    bool trigger = soundSys_->getKeyOnTriggerCache() & (1 << ch_);
    if (trigger)
    {
        auto lvs      = soundSys_->getChLevel(ch_);
        curLevels_[0] = std::max(curLevels_[0], lvs[0]);
        curLevels_[1] = std::max(curLevels_[1], lvs[1]);
    }
}

namespace col
{
uint32_t bg         = 0x0;
uint32_t bgSelected = 0x404040;
uint32_t ch         = 0xffffff;
uint32_t chMuted    = 0x808080;
uint32_t iconCH     = 0xc0c0c0;
uint32_t status     = 0xa0a0a0;
} // namespace col

namespace tex
{
constexpr Rect iconCH   = {{0, 21}, {9, 3}};
constexpr Rect keyboard = {{60, 27}, {260, 20}};
constexpr Rect LVMeter  = {{27, 28}, {2, 19}};
constexpr Rect font8x8  = {{0, 0}, {8, 8}};
constexpr Rect font5x6  = {{0, 8}, {5, 6}};  // 0123456789+-
constexpr Rect font6x7  = {{0, 14}, {6, 7}}; // @#
constexpr Rect chip     = {{0, 49}, {25, 7}};
} // namespace tex

namespace pos
{
constexpr Vec2 keyboard = {60, 0};
constexpr Vec2 LVMeter  = {54, 0};
constexpr Vec2 chip     = {0, 3};

constexpr Vec2 iconCH = {27, 7};
constexpr Vec2 chText = {37, 3};

constexpr Vec2 iconTone = {2, 12};
constexpr Vec2 toneText = {8, 13};

constexpr Vec2 iconKey = {23, 12};
constexpr Vec2 keyText = {29, 13};

} // namespace pos

namespace rect
{
constexpr Rect tone = {{2, 12}, {16, 7}};
constexpr Rect key  = {{23, 12}, {31, 7}};
constexpr Rect bg   = {{0, 0}, {60, 20}};
} // namespace rect

void
KeyboardList::Item::onRender(RenderContext& ctx)
{
    if (!soundSys_)
    {
        return;
    }

    auto* player  = music_player::getActiveMusicPlayer();
    bool isActive = player && !player->isFinished() && !player->isPaused();

    bool selected = ctx.getCurrentIndex() == ctx.getSelectIndex();
    bool enabled  = soundSys_->getEnabledChannels() & (1 << ch_);
    ctx.applyClipRegion();

    auto& sysInfo = soundSys_->getSystemInfo();

    auto colBG = selected ? col::bgSelected : col::bg;

    bool refresh = false;
    if (updated_ || ctx.isInvalidated(listSize_))
    {
        refresh = true;

        ctx.updateInvalidatedRegion(getSize());
        int idx = ctx.getCurrentIndex();

        ctx.fill(rect::bg, colBG);

        ctx.putTextureTrans(pos::iconCH, tex::iconCH);
        char tmp[8];
        snprintf(tmp, sizeof(tmp), "%02d", std::min(99, idx + 1));
        ctx.putTextureText(tmp,
                           '0',
                           pos::chText,
                           tex::font8x8,
                           enabled ? col::ch : col::chMuted,
                           colBG);

        auto sysID = sysInfo.getSystemID(ch_);

        if (sysID >= 0 || sysID < 24)
        {
            int row      = sysID / 12;
            int column   = sysID % 12;
            auto chipTex = tex::chip;
            // 1ピクセル開けて並んでいる
            chipTex.pos.x += column * (chipTex.size.w + 1);
            chipTex.pos.y += row * (chipTex.size.h + 1);
            ctx.putTextureTrans(pos::chip, chipTex);
        }

        updated_ = false;
    }

    int noteIdx = -1;
    int noteSub = 0;
    auto& tmpFB = ctx.getTemporaryFrameBuffer(tex::keyboard.size);
    BBox kbBB, erasedBB;
    kbBB.invalidate();
    erasedBB.invalidate();

    if (refresh)
    {
        kbBB.update(BBox(Vec2{0, 0}, tex::keyboard.size));
    }
    {
        auto recoverFB = ctx.updateFrameBuffer(&tmpFB);
        ctx.applyClipRegion();
        ctx.putTexture({0, 0}, tex::keyboard);

        bool keyOn = soundSys_->getKeyOnChannels() & (1 << ch_);
        keyOn &= enabled;
        bool oneShot = sysInfo.oneShotChannelMask & (1 << ch_);

        static std::vector<int8_t> prevOnKeys;
        prevOnKeys.swap(curOnKeys_);
        curOnKeys_.clear();

        static constexpr Rect masks[] = {{{7, 30}, {4, 17}},   // F
                                         {{12, 30}, {4, 17}},  // G
                                         {{17, 30}, {4, 17}},  // B
                                         {{0, 30}, {4, 11}},   // Black
                                         {{22, 30}, {4, 17}}}; // 88

        static constexpr std::pair<int, int> maskTbl[] = {
            // ofs, mask
            {26 - 25, 1}, // A
            {29 - 25, 3}, // A#
            {31 - 25, 2}, // B
            {1 + 10, 0},  // C
            {4 + 10, 3},  // C#
            {6 + 10, 1},  // D
            {9 + 10, 3},  // D#
            {11 + 10, 2}, // E
            {16 + 10, 0}, // F
            {19 + 10, 3}, // F#
            {21 + 10, 1}, // G
            {24 + 10, 3}, // G#
        };

        auto getMask = [&](int noteIdx) -> auto
        {
            auto& m     = maskTbl[noteIdx % 12];
            int ofs     = (noteIdx / 12) * 35 + m.first;
            int maskIdx = m.second;
            if (noteIdx == 0)
            {
                maskIdx = 0;
            }
            else if (noteIdx == 87)
            {
                maskIdx = 4;
            }
            auto& mask = masks[maskIdx];
            return std::make_tuple(ofs, mask);
        };

        int voiceCount =
            isActive
                ? (sysInfo.multiVoice ? soundSys_->getNoteCount(ch_) : keyOn)
                : 0;
        for (int voice = 0; voice < voiceCount; ++voice)
        {
            float note = soundSys_->getNote(ch_, voice) + 48;
            noteIdx    = static_cast<int>(note + 0.5f);
            noteSub    = static_cast<int>((note - noteIdx) * 100 + 0.5f);

            if (noteIdx >= 0 && noteIdx < 88)
            {
                int ofs;
                Rect mask;
                std::tie(ofs, mask) = getMask(noteIdx);
                if (ofs < tex::keyboard.size.w)
                {
                    Vec2 pos{ofs, 3};
                    ctx.putTexture(pos, mask);
                    kbBB.update(BBox(pos, mask.size));
                }
                curOnKeys_.push_back(noteIdx);
            }
        }
        std::sort(curOnKeys_.begin(), curOnKeys_.end());
        static std::vector<int8_t> diff;
        diff.reserve(16);
        diff.resize(curOnKeys_.size() + prevOnKeys.size());
        auto end = std::set_difference(prevOnKeys.begin(),
                                       prevOnKeys.end(),
                                       curOnKeys_.begin(),
                                       curOnKeys_.end(),
                                       diff.begin());
        for (auto p = diff.begin(); p != end; ++p)
        {
            int ofs;
            Rect mask;
            std::tie(ofs, mask) = getMask(*p);
            if (ofs < tex::keyboard.size.w)
            {
                Vec2 pos{ofs, 3};
                erasedBB.update(BBox(pos, mask.size));
            }
        }
    }
    ctx.applyClipRegion();
    ctx.put(pos::keyboard, tmpFB, erasedBB);
    ctx.put(pos::keyboard, tmpFB, kbBB);
    // todo: 重なってたら1回にしたい

    for (int i = 0; i < 2; ++i)
    {
        int il = std::min(10, std::max(0, int(curLevels_[i] * 10.0f + 0.5f)));
        if (refresh || curIntLevels_[i] != il)
        {
            curIntLevels_[i] = il;

            auto t = tex::LVMeter;
            t.pos.x += 3 * il;

            auto p = pos::LVMeter;
            p.x += i * 3;
            ctx.putTexture(p, t);
        }
    }

    int tone = soundSys_->getInstrument(ch_);
    if (refresh || tone != curTone_)
    {
        curTone_ = tone;

        if (tone < 0)
        {
            ctx.fill(rect::tone, colBG);
        }
        else
        {
            char tmp[10];
            snprintf(tmp, sizeof(tmp), "%02d", std::min(99, tone));
            ctx.putTextureText(
                "@", '@', pos::iconTone, tex::font6x7, col::status, colBG);
            ctx.putTextureText(
                tmp, '0', pos::toneText, tex::font5x6, col::status, colBG);
        }
    }

    if (refresh || noteIdx != curNote_ || noteSub != curNoteSub_)
    {
        curNote_    = noteIdx;
        curNoteSub_ = noteSub;

        if (noteIdx < 0)
        {
            ctx.fill(rect::key, colBG);
        }
        else
        {
            char tmp[10];
            snprintf(tmp,
                     sizeof(tmp),
                     "%02d%c%02d",
                     noteIdx,
                     noteSub < 0 ? ';' : ':', // -+
                     noteSub < 0 ? -noteSub : noteSub);
            ctx.putTextureText(
                "1", '0', pos::iconKey, tex::font6x7, col::status, colBG);
            ctx.putTextureText(
                tmp, '0', pos::keyText, tex::font5x6, col::status, colBG);
        }
    }
}

} // namespace ui
