/*
 * author : Shuichi TAKANO
 * since  : Sat Feb 09 2019 22:50:40
 */
#ifndef _07D526DB_9134_13FE_1576_3EA1586E6814
#define _07D526DB_9134_13FE_1576_3EA1586E6814

#include "scroll_list.h"
#include "widget.h"
#include <music_player/file_format.h>
#include <string>
#include <vector>

namespace ui
{

class FileList : public ScrollList
{
    struct Item : public Widget
    {
        bool updated_  = true;
        bool selected_ = false;

    public:
        Dim2 getSize() const override;
        void onRender(RenderContext& ctx) override;

        virtual void _render(RenderContext& ctx) = 0;
    };

    struct File : public Item
    {
        std::string filename_;
        std::string title_;
        size_t size_ = 0;
        music_player::FileFormat format_{};

    public:
        File()
        {
            filename_ = "filename.mdx";
            title_    = "曲名というかタイトルというか";
            size_     = 123456;
        }

        void onUpdate(UpdateContext& ctx) override {}
        void _render(RenderContext& ctx) override;
    };

    struct Directory : public Item
    {
        std::string name_;

    public:
        Directory() { name_ = "directory name"; }
        void onUpdate(UpdateContext& ctx) override {}
        void _render(RenderContext& ctx) override;
    };

    std::vector<Directory> directories_;
    std::vector<File> files_;

public:
    FileList();

    Dim2 getSize() const override;

    uint16_t getBaseItemSize() const override;

    size_t getWidgetCount() const override;
    const Widget* getWidget(size_t i) const override;
    Widget* getWidget(size_t i) override;

protected:
    Widget* _getWidget(size_t i);
}; // namespace ui

} // namespace ui

#endif /* _07D526DB_9134_13FE_1576_3EA1586E6814 */
