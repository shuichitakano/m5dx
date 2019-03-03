/*
 * author : Shuichi TAKANO
 * since  : Mon Feb 11 2019 18:24:9
 */
#ifndef _03737EF3_8134_13F9_1165_6453B9CC56E3
#define _03737EF3_8134_13F9_1165_6453B9CC56E3

#include "locale.h"
#include <array>

namespace ui
{
using Strings = std::array<const char*, (int)Language::NUM>;
const char* get(const Strings& strs);

namespace strings
{

extern const Strings loadFile;
extern const Strings selectFile;
extern const Strings selectSong;
extern const Strings volumeAdj;
extern const Strings settings;
extern const Strings select;
extern const Strings play;
extern const Strings stop;
extern const Strings up;
extern const Strings down;
extern const Strings back;
extern const Strings prev;
extern const Strings next;
extern const Strings volUp;
extern const Strings volDown;

} // namespace strings

} // namespace ui

#endif /* _03737EF3_8134_13F9_1165_6453B9CC56E3 */
