/*
 * author : Shuichi TAKANO
 * since  : Mon Feb 11 2019 18:51:14
 */

#include "file_window.h"
#include "strings.h"
#include <debug.h>
#include <mutex>

#include <music_player/music_player_manager.h>

namespace ui
{

FileWindow::FileWindow(const std::string& path)
{
    setChild(&list_);
    setPath(path);

    list_.setDecideFunc([this](int i) {
        DBOUT(("decide index %d\n", i));
        if (i < 0)
        {
            return;
        }
        auto item  = list_.getItem(i);
        auto& name = item.first;

        if (item.second)
        {
            // dir
            DBOUT(("select directory %s\n", name.c_str()));
            if (name == "..")
            {
                auto& path = list_.getPath();
                auto p     = path.rfind('/');
                if (p != std::string::npos)
                {
                    setPath(p == 0 ? "/" : path.substr(0, p));
                }
            }
            else
            {
                setPath(list_.makeAbsPath(name));
            }

            list_.reset();
            refresh();
        }
        else
        {
            // file
            DBOUT(("select file %s\n", name.c_str()));
            auto fn = list_.makeAbsPath(name);

            music_player::playMusicFile(fn.c_str(), 0, true);
        }
    });
}

void
FileWindow::setPath(const std::string& path)
{
    std::lock_guard<sys::Mutex> lock(list_.getMutex());

    if (path == "/" || path == "")
    {
        setTitle(get(strings::loadFile));
    }
    else
    {
        setTitle(path);
    }

    list_.setPath(path);
}

void
FileWindow::onUpdate(UpdateContext& ctx)
{
    std::lock_guard<sys::Mutex> lock(list_.getMutex());
    Window::onUpdate(ctx);
}

void
FileWindow::onRender(RenderContext& ctx)
{
    std::lock_guard<sys::Mutex> lock(list_.getMutex());
    Window::onRender(ctx);
}

} // namespace ui
