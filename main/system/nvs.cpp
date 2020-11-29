/*
 * author : Shuichi TAKANO
 * since  : Mon Dec 10 2018 2:55:18
 */

#include "nvs.h"
#include <nvs.h>
#include <nvs_flash.h>
#include <vector>

namespace sys
{

bool
initializeNVS()
{
    static bool initialized = false;
    if (initialized)
    {
        return true;
    }

    auto ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    if (ret)
    {
        return false;
    }
    initialized = true;
    return true;
}

////////////////

NVSHandler::NVSHandler(const char* name, bool writable)
{
    initializeNVS();
    if (nvs_open(name, writable ? NVS_READWRITE : NVS_READONLY, &handle_) ==
        ESP_OK)
    {
        enabled_ = true;
    }
}

NVSHandler::~NVSHandler()
{
    if (enabled_)
    {
        nvs_close(handle_);
    }
}

std::experimental::optional<bool>
NVSHandler::getBool(const char* key)
{
    uint8_t v;
    if (enabled_ && ESP_OK == nvs_get_u8(handle_, key, &v))
    {
        return v;
    }
    return {};
}

std::experimental::optional<int32_t>
NVSHandler::getInt(const char* key)
{
    int32_t v;
    if (enabled_ && ESP_OK == nvs_get_i32(handle_, key, &v))
    {
        return v;
    }
    return {};
}

std::experimental::optional<uint32_t>
NVSHandler::getUInt(const char* key)
{
    uint32_t v;
    if (enabled_ && ESP_OK == nvs_get_u32(handle_, key, &v))
    {
        return v;
    }
    return {};
}

std::experimental::optional<float>
NVSHandler::getFloat(const char* key)
{
    float v;
    if (enabled_ &&
        ESP_OK == nvs_get_u32(handle_, key, reinterpret_cast<uint32_t*>(&v)))
    {
        return v;
    }
    return {};
}

std::experimental::optional<std::string>
NVSHandler::getString(const char* key)
{
    size_t size;
    if (enabled_ && ESP_OK == nvs_get_str(handle_, key, nullptr, &size))
    {
        std::vector<char> buffer(size);
        if (ESP_OK == nvs_get_str(handle_, key, buffer.data(), &size))
        {
            return {buffer.data()};
        }
    }
    return {};
}

bool
NVSHandler::setBool(const char* key, bool v)
{
    return enabled_ && ESP_OK == nvs_set_u8(handle_, key, v ? 1 : 0);
}

bool
NVSHandler::setInt(const char* key, int32_t v)
{
    return enabled_ && ESP_OK == nvs_set_i32(handle_, key, v);
}

bool
NVSHandler::setUInt(const char* key, uint32_t v)
{
    return enabled_ && ESP_OK == nvs_set_u32(handle_, key, v);
}

bool
NVSHandler::setFloat(const char* key, float v)
{
    union
    {
        float f_;
        uint32_t i_;
    } u;
    u.f_ = v;
    return enabled_ && ESP_OK == nvs_set_u32(handle_, key, u.i_);
}

bool
NVSHandler::setString(const char* key, const char* str)
{
    return enabled_ && ESP_OK == nvs_set_str(handle_, key, str);
}

} // namespace sys
