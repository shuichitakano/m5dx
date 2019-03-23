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
}

void
SimpleListWindow::append(SimpleList::Item* item)
{
    list_.append(item);
}

void
SimpleListWindow::appendCancel(const char* str)
{
    list_.appendCancel(str);
}

void
SimpleListWindow::clear()
{
    list_.clear();
}

} // namespace ui
