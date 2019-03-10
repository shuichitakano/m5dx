/*
 * author : Shuichi TAKANO
 * since  : Mon Feb 11 2019 6:34:4
 */
#ifndef _9A0E4147_5134_13F9_60E5_6EB0573131D2
#define _9A0E4147_5134_13F9_60E5_6EB0573131D2

#include "context.h"

namespace ui
{

// 領域の右側に縦スクロールバーを描画する
void drawVScrollBar(RenderContext& ctx,
                    Dim2 size,
                    int barPos,
                    int barSize,
                    int regionSize,
                    bool forceDraw);

// 指定領域のサイズで縦スクロールバーを描画する
void _drawVScrollBar(RenderContext& ctx,
                     Vec2 pos,
                     Dim2 size,
                     int barPos,
                     int barSize,
                     int regionSize);

} // namespace ui

#endif /* _9A0E4147_5134_13F9_60E5_6EB0573131D2 */
