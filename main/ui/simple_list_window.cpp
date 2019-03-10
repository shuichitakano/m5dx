/*
 * author : Shuichi TAKANO
 * since  : Sat Mar 09 2019 3:14:35
 */

#include "simple_list_window.h"
#include "context.h"

namespace ui
{

SimpleListWindow::SimpleListWindow()
{
    setChild(&list_);

    list_.setLongPressFunc([](UpdateContext& ctx, int) { ctx.popManagedUI(); });

    list_.setDecideFunc(
        [this](UpdateContext& ctx, int i) { list_.getItem(i)->decide(ctx); });
}

void
SimpleListWindow::append(SimpleList::Item* item)
{
    list_.append(item);
}

} // namespace ui
