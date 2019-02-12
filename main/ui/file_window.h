/*
 * author : Shuichi TAKANO
 * since  : Mon Feb 11 2019 18:15:22
 */
#ifndef _35DE29D2_7134_13F9_114F_C0A0F70B9A0E
#define _35DE29D2_7134_13F9_114F_C0A0F70B9A0E

#include "file_list.h"
#include "window.h"

namespace ui
{

class FileWindow : public Window
{
    FileList list_;

public:
    FileWindow(const std::string& path);

    void setPath(const std::string& path);

    void onUpdate(UpdateContext& ctx) override;
    void onRender(RenderContext& ctx) override;
};

} // namespace ui

#endif /* _35DE29D2_7134_13F9_114F_C0A0F70B9A0E */
