/*
 * author : Shuichi TAKANO
 * since  : Mon Feb 11 2019 18:56:47
 */
#ifndef _06B5AA0F_2134_13F9_11B4_7DD6FD1CFFBB
#define _06B5AA0F_2134_13F9_11B4_7DD6FD1CFFBB

#include "locale.h"

namespace ui
{

class SystemSettings
{
    Language language_ = {Language::JAPANESE};

public:
    Language getLanguage() const { return language_; }
    void setLanguage(Language l) { language_ = l; }

    static SystemSettings& instance();
};

} // namespace ui

#endif /* _06B5AA0F_2134_13F9_11B4_7DD6FD1CFFBB */
