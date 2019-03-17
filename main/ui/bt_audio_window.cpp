/*
 * author : Shuichi TAKANO
 * since  : Sat Mar 16 2019 23:30:41
 */

#include "bt_audio_window.h"

#include "context.h"
#include <debug.h>
#include <io/bt_a2dp_source_manager.h>
#include <mutex>
#include <sys/mutex.h>

namespace ui
{

BTAudioWindow::BTAudioWindow()
{
    io::BTA2DPSourceManager::instance().startDiscovery(60);
}

void
BTAudioWindow::onUpdate(UpdateContext& ctx)
{
    auto& a2dpman = io::BTA2DPSourceManager::instance();
    std::lock_guard<sys::Mutex> lock(a2dpman.getMutex());

    const auto& entries = a2dpman.getEntries();
    items_.resize(entries.size());
    clear();
    size_t i = 0;
    for (auto& v : entries)
    {
        auto& item = items_[i];
        if (item.name_ != v.name || item.rssi_ != v.rssi ||
            item.addr_ != v.addr)
        {
            item.name_ = v.name;
            item.rssi_ = v.rssi;
            item.addr_ = v.addr;
            item.touch();
        }
        append(&item);
        ++i;
    }
}

void
BTAudioWindow::Item::_render(RenderContext& ctx)
{
    super::_render(ctx);

    auto w = getSize().w;

    Dim2 rssiSize = {24, 8};
    Vec2 rssiPos  = {int(w - rssiSize.w - 8), 2};
    Dim2 addrSize = {18 * 4, 8};
    Vec2 addrPos  = {int(rssiPos.x - addrSize.w), 2};

    ctx.setFontColor(0xffffff);

    char buf[20];
    snprintf(buf, sizeof(buf), "%d", rssi_);
    ctx.putText(buf, rssiPos, rssiSize, TextAlignH::RIGHT);

    snprintf(buf,
             sizeof(buf),
             "%02X:%02X:%02X:%02X:%02X:%02X",
             addr_[0],
             addr_[1],
             addr_[2],
             addr_[3],
             addr_[4],
             addr_[5]);
    ctx.putText(buf, addrPos, addrSize, TextAlignH::LEFT);
}

} // namespace ui
