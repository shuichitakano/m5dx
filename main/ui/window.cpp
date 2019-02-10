/*
 * author : Shuichi TAKANO
 * since  : Fri Feb 01 2019 2:53:43
 */

#include "window.h"
#include "context.h"

namespace ui
{

void
Window::onUpdate(UpdateContext& ctx)
{
}

void
Window::onRender(RenderContext& ctx)
{
    auto restorePos  = ctx.updatePosition(pos_);
    auto restoreClip = ctx.updateClipRegion(getSize());
}

} // namespace ui
