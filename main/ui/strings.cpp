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

constexpr Strings loadFile    = {"ファイル読み込み", "LOAD FILE"};
constexpr Strings selectFile  = {"ファイル選択", "SELECT FILE"};
constexpr Strings selectSong  = {"曲選択", "SELECT SONG"};
constexpr Strings volumeAdj   = {"音量調整", "VOLUME"};
constexpr Strings settings    = {"設定", "SETTINGS"};
constexpr Strings select      = {"選択", "SELECT"};
constexpr Strings close       = {"閉じる", "CLOSE"};
constexpr Strings play        = {"再生", "PLAY"};
constexpr Strings stop        = {"停止", "STOP"};
constexpr Strings up          = {"↑", "↑"};
constexpr Strings down        = {"↓", "↓"};
constexpr Strings cancel      = {"キャンセル", "CANCEL"};
constexpr Strings back        = {"戻る", "BACK"};
constexpr Strings prev        = {"前へ", "PREV"};
constexpr Strings next        = {"次へ", "NEXT"};
constexpr Strings volUp       = {"大", "UP"};
constexpr Strings volDown     = {"小", "DOWN"};
constexpr Strings nothing     = {"なし", "NOTHING"};
constexpr Strings enable      = {"有効", "ENABLE"};
constexpr Strings disable     = {"無効", "DISABLE"};
constexpr Strings always      = {"常時", "ALWAYS"};
constexpr Strings yes         = {"はい", "YES"};
constexpr Strings no          = {"いいえ", "NO"};
constexpr Strings _auto       = {"自動", "AUTO"};
constexpr Strings shuffleMode = {"シャッフル再生", "SHUFFLE MODE"};
constexpr Strings loopCount   = {"ループ回数", "LOOP COUNT"};
constexpr Strings soundModule = {"サウンドモジュール", "SOUND MODULE"};
constexpr Strings trackMask   = {"トラックマスク", "TRACK MASK"};
constexpr Strings trackSelect = {"トラック選択", "TRACK SELECT"};

constexpr Strings internalSpeaker = {"内蔵スピーカー", "INTERNAL SPEAKER"};
constexpr Strings backLightIntensity = {"バックライト輝度",
                                        "BACKLIGHT INTENSITY"};
constexpr Strings dispOffReverse     = {"裏返し画面オフ",
                                    "DISPLAY OFF When TURN DOWN"};
constexpr Strings trackOverride = {"MIDIオーバーライド", "MIDI OVERRIDE"};
constexpr Strings playerDiadMode = {"プレイヤーダイアルモード",
                                    "PLAYER DIAL MODE"};

constexpr Strings bootBTMIDI  = {"起動時 Bluetooth MIDI 待ち受け",
                                "STARTUP BT MIDI CONNECTION MODE"};
constexpr Strings bootBTAudio = {"起動時 Bluetooth Audio 待ち受け",
                                 "STARTUP BT AUDIO CONNECTION MODE"};
constexpr Strings _30sec      = {"30秒", "30 SEC"};
constexpr Strings _60sec      = {"60秒", "60 SEC"};

constexpr Strings language = {"Language", "LANGUAGE"};
constexpr Strings japanese = {"日本語", "JAPANESE"};
constexpr Strings english  = {"English", "ENGLISH"};

constexpr Strings repeatMode = {"リピート", "REPEAT MODE"};
constexpr Strings all        = {"全て", "ALL"};
constexpr Strings single     = {"１曲", "SINGLE"};
constexpr Strings none       = {"しない", "NONE"};

constexpr Strings writeModuleID = {"モジュールID書き込み", "WRITE MODULE ID"};
constexpr Strings writeModuleMes = {
    "EEPROMにモジュールID(%s)を書き込みます\n本当によろしいですか?",
    "Write module ID(%s) to EEPROM\nAre you sure?"};

constexpr Strings BTAudio           = {"Bluetooth Audio", "BT AUDIO"};
constexpr Strings BTAudioConnectMes = {"'%s' に接続しています...",
                                       "Connecting to '%s'..."};

constexpr Strings BTMIDI = {"Bluetooth MIDI", "BT MIDI"};

} // namespace strings

} // namespace ui
