/*
 * author : Shuichi TAKANO
 * since  : Mon Feb 11 2019 19:3:50
 */

#include "system_setting.h"

namespace ui
{



SystemSettings&
SystemSettings::instance()
{
    static SystemSettings inst;
    return inst;
}

} // namespace ui
