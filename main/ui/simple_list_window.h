/*
 * author : Shuichi TAKANO
 * since  : Sat Mar 09 2019 3:12:39
 */
#ifndef _190A31FF_9134_1462_2FAB_D4931B65026F
#define _190A31FF_9134_1462_2FAB_D4931B65026F

#include "simple_list.h"
#include "window.h"

namespace ui
{

class SimpleListWindow : public Window
{
    SimpleList list_;

public:
    SimpleListWindow();

    void append(SimpleList::Item* item);
    void appendCancel(const char* str = nullptr);
    void clear();

protected:
    SimpleList& getList() { return list_; }
};

} // namespace ui

#endif /* _190A31FF_9134_1462_2FAB_D4931B65026F */
