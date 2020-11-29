/*
 * author : Shuichi TAKANO
 * since  : Mon Dec 10 2018 2:54:57
 */
#ifndef CEFCD123_2133_F0D1_26D7_187B66DB28EC
#define CEFCD123_2133_F0D1_26D7_187B66DB28EC

#include <experimental/optional>
#include <stdint.h>
#include <string>

namespace sys
{

bool initializeNVS();

class NVSHandler
{
    bool enabled_    = false;
    uint32_t handle_ = 0;

public:
    NVSHandler(const char* name, bool writable);
    ~NVSHandler();
    explicit operator bool() const { return enabled_; }

    std::experimental::optional<bool> getBool(const char* key);
    std::experimental::optional<int32_t> getInt(const char* key);
    std::experimental::optional<uint32_t> getUInt(const char* key);
    std::experimental::optional<float> getFloat(const char* key);
    std::experimental::optional<std::string> getString(const char* key);

    bool setBool(const char* key, bool v);
    bool setInt(const char* key, int32_t v);
    bool setUInt(const char* key, uint32_t v);
    bool setFloat(const char* key, float v);
    bool setString(const char* key, const char* str);

private:
    NVSHandler(const NVSHandler&) = delete;
    NVSHandler& operator=(const NVSHandler&) = delete;
};

} // namespace sys

#endif /* CEFCD123_2133_F0D1_26D7_187B66DB28EC */
