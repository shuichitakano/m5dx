/*
 * author : Shuichi TAKANO
 * since  : Fri Sep 11 2020 01:56:49
 */
#ifndef _803AD24F_1134_3DC9_17DF_AE5AC4FD0AE8
#define _803AD24F_1134_3DC9_17DF_AE5AC4FD0AE8

#include "types.h"

namespace graphics
{
class FrameBufferBase;
class Texture;
} // namespace graphics

namespace ui
{

void put(graphics::FrameBufferBase& fb,
         const graphics::Texture& tex,
         Vec2 dst,
         const Rect& src);

void putTrans(graphics::FrameBufferBase& fb,
              const graphics::Texture& tex,
              Vec2 dst,
              const Rect& src);

void putReplaced(graphics::FrameBufferBase& fb,
                 const graphics::Texture& tex,
                 Vec2 dst,
                 const Rect& src,
                 uint16_t color,
                 uint16_t bg);

void putText(graphics::FrameBufferBase& fb,
             const graphics::Texture& tex,
             const char* text,
             int charOfs,
             Vec2 dst,
             const Rect& font,
             uint16_t color,
             uint16_t bg);

} // namespace ui

#endif /* _803AD24F_1134_3DC9_17DF_AE5AC4FD0AE8 */
