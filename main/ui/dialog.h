/*
 * author : Shuichi TAKANO
 * since  : Sun Mar 10 2019 3:28:45
 */
#ifndef _22295606_4134_145C_321F_73D858CDC6BB
#define _22295606_4134_145C_321F_73D858CDC6BB

#include "window.h"
#include <functional>
#include <vector>

namespace ui
{

class Dialog : public Window
{
    using Func = std::function<void(UpdateContext&)>;

    struct Client : public Widget
    {
        struct Button
        {
            std::string text_;
            Func action_;
        };

        Dim2 size_;
        std::string message_;
        std::vector<Button> buttons_;

        size_t currentIndex_ = 0;

        bool needRefresh_       = true;
        bool needRefreshButton_ = true;

        Func updateFunc_;

    public:
        Dim2 getSize() const override { return size_; }
        void onUpdate(UpdateContext& ctx) override;
        void onRender(RenderContext& ctx) override;
        void touch() override { needRefresh_ = true; }
    };

    Client client_;

public:
    Dialog(const std::string& title, Dim2 size);

    void setMessage(const std::string& str);
    void appendButton(const std::string& str, const Func& action = {});

    void setUpdateFunction(const Func& f) { client_.updateFunc_ = f; }
};

} // namespace ui

#endif /* _22295606_4134_145C_321F_73D858CDC6BB */
