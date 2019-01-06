/*
 * author : Shuichi TAKANO
 * since  : Mon Oct 29 2018 12:43:57
 */

#include "file_util.h"
#include <debug.h>
#include <stdio.h>

namespace io
{

int getFileSize(const char *filename)
{
    auto fp = fopen(filename, "rb");
    if (!fp)
    {
        DBOUT(("file open error %s\n", filename));
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    auto pos = ftell(fp);
    fclose(fp);

    return (int)pos;
}

int readFile(void *buffer, const char *filename, size_t size, size_t pos)
{
    DBOUT(("readFile: %s, %x-%d bytes.\n", filename, (int)pos, (int)size));
    auto fp = fopen(filename, "rb");
    if (!fp)
    {
        DBOUT(("file open error %s\n", filename));
        return -1;
    }

    if (pos)
        fseek(fp, pos, SEEK_SET);
    auto r = fread(buffer, 1, size, fp);
    fclose(fp);

    return (int)r;
}

bool readFile(std::vector<uint8_t> &buffer, const char *filename)
{
    DBOUT(("readFile: %s.\n", filename));
    auto fp = fopen(filename, "rb");
    if (!fp)
    {
        DBOUT(("file open error '%s'\n", filename));
        return false;
    }

    fseek(fp, 0, SEEK_END);
    auto size = (size_t)ftell(fp);

    fseek(fp, 0, SEEK_SET);

    buffer.resize(size);
    auto r = fread(buffer.data(), 1, size, fp);
    fclose(fp);

    return r == size;
}

bool writeFile(const void *buffer, const char *filename, size_t size)
{
    DBOUT(("writeFile: %s.\n", filename));
    auto fp = fopen(filename, "wb");
    if (!fp)
    {
        DBOUT(("file open error %s\n", filename));
        return false;
    }

    fwrite(buffer, 1, size, fp);
    fclose(fp);
    return true;
}

bool updateFile(const void *buffer, const char *filename, size_t size, size_t pos)
{
    DBOUT(("updateFile: %s, %x-%d bytes.\n", filename, (int)pos, (int)size));
    auto fp = fopen(filename, "rb+");
    if (!fp)
    {
        DBOUT(("file open error %s\n", filename));
        return false;
    }

    fseek(fp, pos, SEEK_SET);
    fwrite(buffer, 1, size, fp);
    fclose(fp);
    return true;
}

bool writeFile(const std::vector<uint8_t> &buffer, const char *filename)
{
    writeFile(buffer.data(), filename, buffer.size());
    return true;
}

} // namespace io
