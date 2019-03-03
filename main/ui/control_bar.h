/*
 * author : Shuichi TAKANO
 * since  : Sun Mar 03 2019 2:28:26
 */
#ifndef BE5A0834_6134_145C_22D8_AAC02531F06B
#define BE5A0834_6134_145C_22D8_AAC02531F06B

#include "widget.h"
#include <string>

namespace ui
{

class ControlBar : public Widget
{
    struct State
    {
        std::string text_ = "Long Hoge";
    };

    State state_[3];
    bool needRefresh_ = true;

public:
    void onUpdate(UpdateContext& ctx) override;
    void onRender(RenderContext& ctx) override;
    Dim2 getSize() const override;
};

} // namespace ui

#endif /* BE5A0834_6134_145C_22D8_AAC02531F06B */
