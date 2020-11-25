/*
 * author : Shuichi TAKANO
 * since  : Sun Mar 03 2019 2:24:37
 */
#ifndef ACAAAD1C_E134_145C_2286_21780926CC3C
#define ACAAAD1C_E134_145C_2286_21780926CC3C

#include "neo_pixel_disp.h"
#include "widget.h"
#include <memory>
#include <string>
#include <vector>

namespace ui
{
class UIManager;
class KeyboardList;

class PlayerWindow : public Widget,
                     public std::enable_shared_from_this<PlayerWindow>
{
    std::shared_ptr<KeyboardList> keyboardList_;

    bool needRefresh_ = true;

    bool longLeftCaptured_  = false;
    bool longRightCaptured_ = false;

    std::string title_;
    int currentLoop_ = -1;
    int currentSong_ = -1;
    int currentTime_ = -1;

    float volVisible_ = 0;

    std::vector<float> roudnessTable_;

    NeoPixelDisp neoPixelDisp_;
    float currentDt_ = 0;

public:
    PlayerWindow();
    void show(UIManager& m);

    void onUpdate(UpdateContext& ctx) override;
    void onRender(RenderContext& ctx) override;
    Dim2 getSize() const override;
};

} // namespace ui

#endif /* ACAAAD1C_E134_145C_2286_21780926CC3C */
