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
extern const Strings close;
extern const Strings play;
extern const Strings stop;
extern const Strings up;
extern const Strings down;
extern const Strings cancel;
extern const Strings back;
extern const Strings prev;
extern const Strings next;
extern const Strings volUp;
extern const Strings volDown;
extern const Strings nothing;
extern const Strings enable;
extern const Strings disable;
extern const Strings always;
extern const Strings yes;
extern const Strings no;
extern const Strings _auto;
extern const Strings shuffleMode;
extern const Strings loopCount;
extern const Strings soundModule;
extern const Strings trackMask;
extern const Strings trackSelect;

extern const Strings internalSpeaker;
extern const Strings deltaSigmaMode;
extern const Strings backLightIntensity;
extern const Strings dispOffReverse;
extern const Strings trackOverride;
extern const Strings playerDiadMode;

extern const Strings bootBTMIDI;
extern const Strings bootBTAudio;
extern const Strings _30sec;
extern const Strings _60sec;
extern const Strings _1stOrder;
extern const Strings _3rdOrder;

extern const Strings language;
extern const Strings japanese;
extern const Strings english;

extern const Strings repeatMode;
extern const Strings all;
extern const Strings single;
extern const Strings none;

extern const Strings writeModuleID;
extern const Strings writeModuleMes;

extern const Strings BTAudio;
extern const Strings BTAudioConnectMes;
extern const Strings BTMIDI;

} // namespace strings

} // namespace ui

#endif /* _03737EF3_8134_13F9_1165_6453B9CC56E3 */
