/*
 * author : Shuichi TAKANO
 * since  : Mon Feb 11 2019 17:39:32
 */

#include "application.h"

#include "debug.h"
#include "target.h"
#include <audio/audio.h>
#include <audio/audio_out.h>
#include <graphics/bmp.h>
#include <graphics/display.h>
#include <graphics/font_data.h>
#include <graphics/font_manager.h>
#include <graphics/texture.h>
#include <io/bt_a2dp_source_manager.h>
#include <io/file_util.h>
#include <memory>
#include <music_player/music_player_manager.h>
#include <system/job_manager.h>
#include <system/util.h>
#include <ui/context.h>
#include <ui/key.h>
#include <ui/system_setting.h>
#include <ui/ui_manager.h>
#include <util/binary.h>
#include <wire.h>

#include <ui/control_bar.h>
#include <ui/player_window.h>

DEF_LINKED_BINARY(m5dx_material_bmp);
DEF_LINKED_BINARY(_4x8_font_bin);
DEF_LINKED_BINARY(misaki_font_bin);

namespace M5DX
{

namespace
{

struct App
{
    sys::JobManager jobManagerHighPrio_;

    graphics::FontData fontAscii_;
    graphics::FontData fontKanji_;
    graphics::Texture texture_;

    ui::UIManager uiManager_;
    ui::KeyState keyState_;

    std::shared_ptr<ui::ControlBar> controlBar_;

    io::BTA2DPSourceManager a2dpManager_;

    uint32_t currentTimer_ = 0;

public:
    App()
    {
#if 0
        // todo: フォントはコード埋め込みに変更する
        static std::vector<uint8_t> fontAsciiBin;
        static std::vector<uint8_t> fontKanjiBin;
        io::readFile(fontAsciiBin, "/data/4x8_font.bin");
        io::readFile(fontKanjiBin, "/data/misaki_font.bin");

        fontAscii_.setData(fontAsciiBin.data());
        fontKanji_.setData(fontKanjiBin.data());
#else
        fontAscii_.setData(GET_LINKED_BINARY(_4x8_font_bin));
        fontKanji_.setData(GET_LINKED_BINARY(misaki_font_bin));
#endif

        texture_.initialize(
            GET_LINKED_BINARY_T(graphics::BMP, m5dx_material_bmp));

        //
        jobManagerHighPrio_.start(10, 2048, "JobManagerHP");

        a2dpManager_.initialize(&jobManagerHighPrio_);
        //        a2dpManager_.startDiscovery();

        //
        controlBar_ = std::make_shared<ui::ControlBar>();
        uiManager_.push(controlBar_, {0, 232});
        std::make_shared<ui::PlayerWindow>()->show(uiManager_);

        ui::SystemSettings::instance().apply();

        currentTimer_ = sys::millis();
    }

    void tick()
    {
        uint32_t time = millis();
        float dt      = (time - currentTimer_) * 0.001f;
        currentTimer_ = time;

        music_player::tickMusicPlayerManager();

        {
            int dial     = 0;
            bool trigger = false;

            target::startI2C();
            Wire.requestFrom(0x62, 2);
            while (Wire.available())
            {
                dial += (int8_t)Wire.read();
                trigger |= Wire.read() != 255;
            }
            target::endI2C();

            keyState_.update(!target::getButtonA(),
                             !target::getButtonB(),
                             !target::getButtonC(),
                             trigger,
                             dial);

            ui::UpdateContext ctx(
                dt, &uiManager_, &keyState_, controlBar_.get());
            uiManager_.update(ctx);
        }
        {
            auto& ss       = ui::SystemSettings::instance();
            auto& audioOut = audio::AudioOutDriverManager::instance();

            float vol = expf(ss.getVolume() * 0.11512925464970229f);
            audioOut.setVolume(vol);
        }
        {
            ui::RenderContext ctx;
            ctx.setFrameBuffer(&graphics::getDisplay());
            ctx.setTexture(&texture_);
            auto& fm = ctx.getFontManager();
            fm.setAsciiFontData(&fontAscii_);
            fm.setKanjiFontData(&fontKanji_);

            uiManager_.render(ctx);

            drawDebug(ctx);

            audio::dumpFMDataDebug();
        }
    }

    void drawDebug(ui::RenderContext& ctx)
    {
#ifndef NDEBUG
        static int counter = 0;
        ++counter;

        ctx.setFontColor(0x40ffa0);
        ctx.setFontBGColor(0x000000);
        auto& fm = ctx.getFontManager();
        fm.setEdgedMode(false);
        fm.setTransparentMode(false);

        // char str[20];
        // sprintf(str, "F %d", counter);
        // ctx.putText(str, {0, 0}, {320, 240}, ui::TextAlignH::RIGHT);
#endif
    }

    static App& instance()
    {
        static App inst;
        return inst;
    }
};

} // namespace

void
initialize()
{
    App::instance();
}

void
tick()
{
    App::instance().tick();
}

}; // namespace M5DX
