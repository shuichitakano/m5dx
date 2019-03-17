/*
 * author : Shuichi TAKANO
 * since  : Fri Mar 08 2019 2:15:45
 */
#ifndef D9264661_D134_1461_20E4_34F1991810E1
#define D9264661_D134_1461_20E4_34F1991810E1

#include "scroll_list.h"
#include <string>
#include <vector>

namespace ui
{

class SimpleList : public ScrollList
{
    using super = ScrollList;

public:
    class Item : public Widget
    {
        bool needRefresh_ = true;

    public:
        Dim2 getSize() const override;
        void onUpdate(UpdateContext& ctx) override {}
        void onRender(RenderContext& ctx) override;
        void touch() override { needRefresh_ = true; }

        virtual void _render(RenderContext& ctx) = 0;
        virtual void decide(UpdateContext& ctx) {}
    };

    class ItemWithTitle : public Item
    {
        using super = Item;

    public:
        virtual std::string getTitle() const = 0;

        void _render(RenderContext& ctx) override;
    };

    class ItemWithValue : public ItemWithTitle
    {
        using super = ItemWithTitle;

    public:
        virtual std::string getValue() const = 0;

        void _render(RenderContext& ctx) override;
    };

    class TextItem : public ItemWithTitle
    {
        std::string text_;

    public:
        TextItem() = default;
        TextItem(std::string&& text)
            : text_(text)
        {
        }
        void setText(const std::string& str) { text_ = str; }
        std::string getTitle() const override { return text_; }
    };

public:
    SimpleList();

    Dim2 getSize() const override;

    uint16_t getBaseItemSize() const override;

    size_t getWidgetCount() const override;
    const Widget* getWidget(size_t i) const override;
    Widget* getWidget(size_t i) override;

    void clear();
    void append(Item* item);
    Item* getItem(size_t i) { return items_[i]; }

private:
    std::vector<Item*> items_;
};

} // namespace ui

#endif /* D9264661_D134_1461_20E4_34F1991810E1 */
