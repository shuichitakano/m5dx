/*
 * author : Shuichi TAKANO
 * since  : Mon Feb 11 2019 16:38:19
 */
#ifndef _1E8A3899_1134_13F9_FA91_77E1D8065E95
#define _1E8A3899_1134_13F9_FA91_77E1D8065E95

#include "widget.h"
#include <memory>
#include <stdint.h>
#include <sys/mutex.h>
#include <vector>

namespace ui
{

class UIManager
{
    struct Layer
    {
        Vec2 pos_;
        WidgetPtr widget_;
    };

    std::vector<Layer> layer_;
    std::vector<Layer> next_;
    size_t renderStartLV_ = 0;
    sys::Mutex mutex_;

    bool popReq_  = false;
    bool refresh_ = true;

public:
    void push(const WidgetPtr& p, Vec2 pos);
    void push(const WidgetPtr& p);
    void pop();

    void update(UpdateContext& ctx);
    void render(RenderContext& ctx);

protected:
    void checkRenderStartLV();
};

} // namespace ui

#endif /* _1E8A3899_1134_13F9_FA91_77E1D8065E95 */
