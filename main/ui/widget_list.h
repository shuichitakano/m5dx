/*
 * author : Shuichi TAKANO
 * since  : Sat Feb 09 2019 17:43:39
 */
#ifndef _7B419DEE_C134_13FE_10A0_613DEAC9542F
#define _7B419DEE_C134_13FE_10A0_613DEAC9542F

#include <stdlib.h>

namespace ui
{

class Widget;

class WidgetList
{
public:
    virtual ~WidgetList() noexcept = default;

    virtual size_t getWidgetCount() const           = 0;
    virtual const Widget* getWidget(size_t i) const = 0;
    virtual Widget* getWidget(size_t i)             = 0;
};

} // namespace ui

#endif /* _7B419DEE_C134_13FE_10A0_613DEAC9542F */
