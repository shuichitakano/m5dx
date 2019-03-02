/*
 * author : Shuichi TAKANO
 * since  : Sun Mar 03 2019 0:53:40
 */
#ifndef _1E096396_D134_145C_8275_761AD9D64AA6
#define _1E096396_D134_145C_8275_761AD9D64AA6

namespace M5DX
{

enum class ModuleID
{
    UNKNOWN,
    YM2151,
    YMF288,
};

ModuleID readModuleID();
bool writeModuleID(ModuleID id);

} // namespace M5DX

#endif /* _1E096396_D134_145C_8275_761AD9D64AA6 */
