/*
 * author : Shuichi TAKANO
 * since  : Fri Sep 11 2020 01:57:56
 */

#include "draw_util.h"
#include "types.h"
#include <graphics/framebuffer_base.h>

namespace ui
{

void
put(graphics::FrameBufferBase& fb,
    const graphics::Texture& tex,
    Vec2 dst,
    const Rect& src)
{
    fb.put(tex, dst.x, dst.y, src.pos.x, src.pos.y, src.size.w, src.size.h);
}

void
putTrans(graphics::FrameBufferBase& fb,
         const graphics::Texture& tex,
         Vec2 dst,
         const Rect& src)
{
    fb.putTrans(
        tex, dst.x, dst.y, src.pos.x, src.pos.y, src.size.w, src.size.h);
}

void
putReplaced(graphics::FrameBufferBase& fb,
            const graphics::Texture& tex,
            Vec2 dst,
            const Rect& src,
            uint16_t color,
            uint16_t bg)
{
    fb.putReplaced(tex,
                   dst.x,
                   dst.y,
                   src.pos.x,
                   src.pos.y,
                   src.size.w,
                   src.size.h,
                   color,
                   bg);
}

void
putText(graphics::FrameBufferBase& fb,
        const graphics::Texture& tex,
        const char* text,
        int charOfs,
        Vec2 dst,
        const Rect& font,
        uint16_t color,
        uint16_t bg)
{
    while (*text)
    {
        int ch    = *text++ - charOfs;
        auto rect = font;
        rect.pos.x += font.size.w * ch;
        putReplaced(fb, tex, dst, rect, color, bg);
        dst.x += font.size.w;
    }
}

} // namespace ui
