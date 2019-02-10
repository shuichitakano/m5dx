/*
 * author : Shuichi TAKANO
 * since  : Fri Feb 01 2019 2:21:11
 */
#ifndef A7B568F3_5134_13FD_21BF_9F90204A2101
#define A7B568F3_5134_13FD_21BF_9F90204A2101

#include "widget.h"
#include <string>
#include <vector>

namespace ui
{

class Window : public Widget
{
    Vec2 pos_;
    std::string title_;


public:
    void onUpdate(UpdateContext& ctx) override;
    void onRender(RenderContext& ctx) override;

    Vec2& getPosition() { return pos_; }
    const Vec2& getPosition() const { return pos_; }
};

} // namespace ui

#endif /* A7B568F3_5134_13FD_21BF_9F90204A2101 */
