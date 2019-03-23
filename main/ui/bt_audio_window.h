/*
 * author : Shuichi TAKANO
 * since  : Sat Mar 16 2019 23:27:52
 */
#ifndef _08C82C7F_2134_1462_163B_4D5588AE653B
#define _08C82C7F_2134_1462_163B_4D5588AE653B

#include "simple_list_window.h"
#include <array>
#include <vector>

namespace ui
{

class BTAudioWindow final : public SimpleListWindow
{
    using super = SimpleListWindow;

    struct Item final : public SimpleList::ItemWithTitle
    {
        using super = ItemWithTitle;

        std::string name_;
        std::array<uint8_t, 6> addr_;
        int rssi_;

        std::string getTitle() const override { return name_; }
        void _render(RenderContext& ctx) override;
        void decide(UpdateContext& ctx) override;
    };

    std::vector<Item> items_;

public:
    BTAudioWindow();
    ~BTAudioWindow() override;

    void onUpdate(UpdateContext& ctx) override;
};

} // namespace ui

#endif /* _08C82C7F_2134_1462_163B_4D5588AE653B */
