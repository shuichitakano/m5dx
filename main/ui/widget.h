/*
 * author : Shuichi TAKANO
 * since  : Mon Jan 28 2019 2:8:36
 */
#ifndef _494CE45E_6134_1395_1FCC_4256F7C79FB1
#define _494CE45E_6134_1395_1FCC_4256F7C79FB1

#include "types.h"
#include <memory>

namespace ui
{

class UpdateContext;
class RenderContext;

class Widget
{
public:
    virtual ~Widget() = default;

    virtual void onUpdate(UpdateContext& ctx) = 0;
    virtual void onRender(RenderContext& ctx) = 0;
    virtual Dim2 getSize() const              = 0;

    virtual void touch() {}
};

using WidgetPtr = std::shared_ptr<Widget>;

} // namespace ui

#endif /* _494CE45E_6134_1395_1FCC_4256F7C79FB1 */
