/*
 * author : Shuichi TAKANO
 * since  : Sun Mar 03 2019 2:24:37
 */
#ifndef ACAAAD1C_E134_145C_2286_21780926CC3C
#define ACAAAD1C_E134_145C_2286_21780926CC3C

#include "widget.h"
#include <string>

namespace ui
{

class PlayerWindow : public Widget
{
    bool needRefresh_ = true;

    bool longLeftCaptured_  = false;
    bool longRightCaptured_ = false;

    std::string title_;

public:
    void onUpdate(UpdateContext& ctx) override;
    void onRender(RenderContext& ctx) override;
    Dim2 getSize() const override;
};

} // namespace ui

#endif /* ACAAAD1C_E134_145C_2286_21780926CC3C */
