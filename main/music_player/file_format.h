/*
 * author : Shuichi TAKANO
 * since  : Sat Feb 09 2019 23:30:24
 */
#ifndef _4016EC1B_5134_13FE_163D_DCA34472D18C
#define _4016EC1B_5134_13FE_163D_DCA34472D18C

namespace music_player
{

enum class FileFormat
{
    MDX,
    S98,
    VGM,
};

const char* getFileFormatString(FileFormat fmt);

} // namespace music_player

#endif /* _4016EC1B_5134_13FE_163D_DCA34472D18C */
