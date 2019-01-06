/*
 * author : Shuichi TAKANO
 * since  : Sun Jan 06 2019 22:29:11
 */

#include <driver/sdspi_host.h>
#include <driver/sdmmc_host.h>
#include <sdmmc_cmd.h>
#include <esp_vfs_fat.h>
#include <debug.h>

namespace io
{

bool initializeSD(const char *mountPoint)
{
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = VSPI_HOST;

    sdspi_slot_config_t slotConfig;
    slotConfig.gpio_miso = GPIO_NUM_19;
    slotConfig.gpio_mosi = GPIO_NUM_23;
    slotConfig.gpio_sck = GPIO_NUM_18;
    slotConfig.gpio_cs = GPIO_NUM_4;
    slotConfig.gpio_cd = ((gpio_num_t)-1);
    slotConfig.gpio_wp = ((gpio_num_t)-1);
    slotConfig.dma_channel = 1;

    esp_vfs_fat_sdmmc_mount_config_t mountConfig;
    mountConfig.format_if_mount_failed = false;
    mountConfig.max_files = 1;
    mountConfig.allocation_unit_size = 16 * 1024;

    sdmmc_card_t *card;
    if (auto r = esp_vfs_fat_sdmmc_mount(mountPoint, &host, &slotConfig, &mountConfig, &card))
    {
        DBOUT(("sdcard init error: %d\n", r));
        return false;
    }

#ifndef NDEBUG
    sdmmc_card_print_info(stdout, card);
#endif

    return true;
}

} // namespace io
