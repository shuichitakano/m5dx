/*
 * author : Shuichi TAKANO
 * since  : Mon Oct 29 2018 12:34:17
 */
#ifndef _509564CF_9133_F009_BC50_FDBBA8091096
#define _509564CF_9133_F009_BC50_FDBBA8091096

#include <cstddef>
#include <stdint.h>
#include <vector>

namespace io
{

int getFileSize(const char* filename);
int readFile(void* buffer, const char* filename, size_t size, size_t pos = 0);
bool writeFile(const void* buffer, const char* filename, size_t size);
bool
updateFile(const void* buffer, const char* filename, size_t size, size_t pos);

bool readFile(std::vector<uint8_t>& buffer, const char* filename);
bool writeFile(const std::vector<uint8_t>& buffer, const char* filename);

} // namespace io

#endif /* _509564CF_9133_F009_BC50_FDBBA8091096 */
