/*
 * author : Shuichi TAKANO
 * since  : Mon Oct 29 2018 12:43:57
 */

#include "file_util.h"
#include "../debug.h"
#include <SD.h>

namespace io
{

int
getFileSize(const char* filename)
{
    auto f = SD.open(filename);
    if (!f || !f.available())
    {
        DBOUT(("getFileSize error %s\n", filename));
        return -1;
    }
    return f.size();
}

int
readFile(void* buffer, const char* filename, size_t size, size_t pos)
{
    DBOUT(("readFile(%p,%s,%d,%x)\n", buffer, filename, (int)size, (int)pos));

    auto f = SD.open(filename);
    if (!f)
    {
        DBOUT(("file '%s' open error.\n", filename));
        return -1;
    }

    if (pos)
    {
        if (!f.seek(pos))
        {
            DBOUT(("seek error\n"));
            return -1;
        }
    }

    return f.read((uint8_t*)buffer, size);
}

bool
readFile(std::vector<uint8_t>& buffer, const char* filename)
{
    auto f = SD.open(filename);
    if (!f)
    {
        DBOUT(("file '%s' open error.\n", filename));
        return false;
    }
    buffer.resize(f.size());
    DBOUT(("readFile(%s, %zdbytes)\n", filename, buffer.size()));
    return f.read(buffer.data(), buffer.size());
}

bool
writeFile(const void* buffer, const char* filename, size_t size)
{
    DBOUT(("writeFile(%p,%s,%d)\n", buffer, filename, size));

    auto f = SD.open(filename, "w");
    if (!f)
    {
        DBOUT(("file '%s' open error.\n", filename));
        return false;
    }

    if (!f.write((const uint8_t*)buffer, size))
    {
        DBOUT(("file '%s' write error.\n", filename));
        return false;
    }
    return true;
}

bool
updateFile(const void* buffer, const char* filename, size_t size, size_t pos)
{
    DBOUT(("updateFile(%p,%s,%d,%x)\n", buffer, filename, (int)size, (int)pos));

    auto f = SD.open(filename, "rw"); //?
    if (!f)
    {
        DBOUT(("file '%s' open error.\n", filename));
        return false;
    }

    if (pos)
    {
        if (!f.seek(pos))
        {
            DBOUT(("seek error\n"));
            return false;
        }
    }

    if (!f.write((const uint8_t*)buffer, size))
    {
        DBOUT(("file '%s' write error.\n", filename));
        return false;
    }
    return true;
}

bool
writeFile(const std::vector<uint8_t>& buffer, const char* filename)
{
    return writeFile(buffer.data(), filename, buffer.size());
}

} // namespace io
