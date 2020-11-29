/*
 * author : Shuichi TAKANO
 * since  : Sat Mar 09 2019 11:13:57
 */
#ifndef B6D9C022_F134_1462_A9EB_D64C1A6564FB
#define B6D9C022_F134_1462_A9EB_D64C1A6564FB

#include "simple_list_window.h"

namespace ui
{

class SettingWindow final : public SimpleListWindow
{
    using super = SimpleListWindow;

public:
    SettingWindow();
    ~SettingWindow();

    void onUpdate(UpdateContext& ctx) override;
};

} // namespace ui

#endif /* B6D9C022_F134_1462_A9EB_D64C1A6564FB */
