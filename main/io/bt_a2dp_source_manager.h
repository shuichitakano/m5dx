/*
 * author : Shuichi TAKANO
 * since  : Sat Jan 12 2019 1:56:3
 */
#ifndef BC85C0A5_7134_139A_17D1_D501102E805C
#define BC85C0A5_7134_139A_17D1_D501102E805C

#include <string>

namespace sys
{
class JobManager;
}

namespace io
{

class BTA2DPSourceManager
{
public:
    void initialize(sys::JobManager* jm);
    void start();

    static BTA2DPSourceManager& instance();
};

} // namespace io

#endif /* BC85C0A5_7134_139A_17D1_D501102E805C */
