/*
 * author : Shuichi TAKANO
 * since  : Sat Sep 12 2020 21:06:38
 */
#ifndef A268D6D8_F134_3DCA_1416_70C2F4DACAB7
#define A268D6D8_F134_3DCA_1416_70C2F4DACAB7

#include "scroll_list.h"
#include <array>
#include <stdint.h>
#include <vector>

namespace sound_sys
{
class SoundSystem;
}

namespace ui
{

class KeyboardList final : public ScrollList
{
    using super = ScrollList;

    struct Item final : public Widget
    {
        bool updated_ = true;

        sound_sys::SoundSystem* soundSys_{};
        int ch_ = 0;

        std::array<float, 2> curLevels_ = {};
        int curIntLevels_[2]            = {-1, -1};
        int curTone_                    = -1;
        int curNote_                    = -1;
        int curNoteSub_                 = -1;

        std::vector<int8_t> curOnKeys_;

    public:
        Item();
        Dim2 getSize() const override;
        void onUpdate(UpdateContext& ctx) override;
        void onRender(RenderContext& ctx) override;
        void touch() override { updated_ = true; }
    };

public:
    KeyboardList();

    Dim2 getSize() const override;

    uint16_t getBaseItemSize() const override;

    size_t getWidgetCount() const override;
    const Widget* getWidget(size_t i) const override;
    Widget* getWidget(size_t i) override;
    void onUpdate(UpdateContext& ctx) override;

private:
    std::vector<Item> items_;
};

} // namespace ui

#endif /* A268D6D8_F134_3DCA_1416_70C2F4DACAB7 */
