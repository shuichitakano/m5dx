/*
 * author : Shuichi TAKANO
 * since  : Mon Dec 10 2018 1:53:15
 */

#include "bluetooth.h"

#include "../debug.h"
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <sys/nvs.h>

namespace io
{

bool
initializeBluetooth()
{
    static bool initialized = false;
    if (initialized)
    {
        return true;
    }

    if (!sys::initializeNVS())
    {
        return false;
    }

    //    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    {
        esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
        if (auto ret = esp_bt_controller_init(&bt_cfg))
        {
            DBOUT(("%s: initialize controller failed: %s\n",
                   __func__,
                   esp_err_to_name(ret)));
            return false;
        }
    }

    if (auto ret = esp_bt_controller_enable(ESP_BT_MODE_BTDM))
    {
        DBOUT(("%s: enable controller failed: %s\n",
               __func__,
               esp_err_to_name(ret)));
        return false;
    }

    if (auto ret = esp_bluedroid_init())
    {
        DBOUT((
            "%s: init bluetooth failed: %s\n", __func__, esp_err_to_name(ret)));
        return false;
    }

    if (auto ret = esp_bluedroid_enable())
    {
        DBOUT(("%s: enable bluetooth failed: %s\n",
               __func__,
               esp_err_to_name(ret)));
        return false;
    }

    initialized = true;
    return true;
}

} // namespace io
