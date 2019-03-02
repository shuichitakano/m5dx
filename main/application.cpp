/*
 * author : Shuichi TAKANO
 * since  : Mon Feb 11 2019 17:39:32
 */

#include "application.h"

#include "debug.h"
#include "target.h"
#include <graphics/display.h>
#include <graphics/font_data.h>
#include <graphics/font_manager.h>
#include <io/file_util.h>
#include <memory>
#include <ui/context.h>
#include <ui/key.h>
#include <ui/ui_manager.h>
#include <wire.h>

#include <ui/file_window.h>

namespace M5DX
{

namespace
{

struct App
{
    graphics::FontData fontAscii_;
    graphics::FontData fontKanji_;

    ui::UIManager uiManager_;
    ui::KeyState keyState_;

public:
    App()
    {
        // todo: フォントはコード埋め込みに変更する
        static std::vector<uint8_t> fontAsciiBin;
        static std::vector<uint8_t> fontKanjiBin;
        io::readFile(fontAsciiBin, "/data/4x8_font.bin");
        io::readFile(fontKanjiBin, "/data/misaki_font.bin");

        fontAscii_.setData(fontAsciiBin.data());
        fontKanji_.setData(fontKanjiBin.data());

        //
        uiManager_.append(std::make_shared<ui::FileWindow>("/"));
    }

    void tick()
    {
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

            ui::UpdateContext ctx(&keyState_);
            uiManager_.update(ctx);
        }
        {
            ui::RenderContext ctx;
            ctx.setFrameBuffer(&graphics::getDisplay());
            auto& fm = ctx.getFontManager();
            fm.setAsciiFontData(&fontAscii_);
            fm.setKanjiFontData(&fontKanji_);

            uiManager_.render(ctx);

            drawDebug(ctx);
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

        char str[20];
        sprintf(str, "F %d", counter);
        ctx.putText(str, {0, 0}, {320, 240}, ui::TextAlignH::RIGHT);
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
