/*
 * author : Shuichi TAKANO
 * since  : Sun Mar 03 2019 20:16:58
 */
#ifndef _761BDC01_4134_145C_133C_726DDA327E98
#define _761BDC01_4134_145C_133C_726DDA327E98

#include <string>

namespace ui
{

class ButtonTip
{
public:
    virtual ~ButtonTip() = default;

    virtual void set(int i, const std::string& text) = 0;
};

} // namespace ui

#endif /* _761BDC01_4134_145C_133C_726DDA327E98 */
