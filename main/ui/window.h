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
    std::string title_;
    bool hasFrame_    = false;
    bool needRefresh_ = true;

    Widget* child_{};
    Dim2 size_{};

public:
    Window() = default;
    Window(const std::string& title, Widget* w);

    void setTitle(const std::string& str);
    void setChild(Widget* w, bool fitToClient = true);
    void enableFrame(bool f = true);

    void onUpdate(UpdateContext& ctx) override;
    void onRender(RenderContext& ctx) override;
    Dim2 getSize() const override { return size_; };

    void refresh() { needRefresh_ = true; }
};

} // namespace ui

#endif /* A7B568F3_5134_13FD_21BF_9F90204A2101 */
