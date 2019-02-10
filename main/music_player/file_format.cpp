/*
 * author : Shuichi TAKANO
 * since  : Sat Feb 09 2019 23:31:48
 */

#include "file_format.h"

namespace music_player
{

const char*
getFileFormatString(FileFormat fmt)
{
    switch (fmt)
    {
    case FileFormat::MDX:
        return "MDX";

    case FileFormat::S98:
        return "S98";

    case FileFormat::VGM:
        return "VGM";
    }
    return "---";
}

} // namespace music_player
