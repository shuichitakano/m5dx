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

constexpr Strings loadFile   = {"ファイル読み込み", "LOAD FILE"};
constexpr Strings selectFile = {"ファイル選択", "SELECT FILE"};
constexpr Strings selectSong = {"曲選択", "SELECT SONG"};
constexpr Strings volumeAdj  = {"音量調整", "VOLUME"};
constexpr Strings settings   = {"設定", "SETTINGS"};
constexpr Strings select     = {"選択", "SELECT"};
constexpr Strings play       = {"再生", "PLAY"};
constexpr Strings stop       = {"停止", "STOP"};
constexpr Strings up         = {"↑", "↑"};
constexpr Strings down       = {"↓", "↓"};
constexpr Strings back       = {"戻る", "BACK"};
constexpr Strings prev       = {"前へ", "PREV"};
constexpr Strings next       = {"次へ", "NEXT"};
constexpr Strings volUp      = {"大", "UP"};
constexpr Strings volDown    = {"小", "DOWN"};

} // namespace strings

} // namespace ui
