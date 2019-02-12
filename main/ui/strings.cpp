/*
 * author : Shuichi TAKANO
 * since  : Mon Feb 11 2019 18:24:52
 */

#include "strings.h"
#include "system_setting.h"

namespace ui
{

const char*
get(const Strings& strs)
{
    return strs[(int)SystemSettings::instance().getLanguage()];
}

namespace strings
{

constexpr Strings loadFile = {"ファイル読み込み", "Load File"};

} // namespace strings

} // namespace ui
