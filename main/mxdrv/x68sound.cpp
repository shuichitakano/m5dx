/* -*- mode:C++; -*-
 *
 * author(s) : Shuichi TAKANO
 * since 2014/06/24(Tue) 10:59:45
 */

#include "x68sound.h"
#include "mxdrv.h"
#include "sys.h"
#include <stdint.h>
#include <system/timer.h>

MXDRVSoundSystemSet mxdrvSoundSystemSet_;

extern "C"
{

    void X68Sound_OpmInt(void (*proc)())
    {
        sys::setTimerCallback([=] {
            if (!proc)
            {
                return;
            }

            proc();

            auto* _2151 = getMXDRVSoundSystemSet().ym2151;
            for (int i = 0; i < 8; ++i)
            {
                auto w = &((MXWORK_CH*)MXDRV_GetWork(MXDRV_WORK_FM))[i];
                if (w->S0004)
                {
                    _2151->setInstrumentNumber(i, w->S0004[-1]);
                }
            }
        });
    }

    void X68Sound_AdpcmPoke(unsigned char data)
    {
        auto* p = getMXDRVSoundSystemSet().m6258;

        if (data & 2)
        {
            p->play();

            auto ch  = &((MXWORK_CH*)MXDRV_GetWork(MXDRV_WORK_FM))[8];
            int note = (ch->S0012 + 27) >> 6;
            //      printf ("note %d %d\n", ch->S0012, note);
            p->setNote(0, note - 40);
        }
        if (data & 1)
            p->stop();
    }

    void X68Sound_PpiPoke(unsigned char data)
    {
        auto* p = getMXDRVSoundSystemSet().m6258;
        p->setChMask(data & 2 ? false : true, data & 1 ? false : true);
        p->setSampleRate6258((data >> 2) & 3);
    }

    int X68Sound_Pcm8_Out(int ch, void* adrs, int mode, int len)
    {
        //  printf ("pcm8 out %d: %p, %d(%d %d %d) %d\n", ch, adrs, mode,
        //  vol, rate, m, len);
        auto* p = getMXDRVSoundSystemSet().m6258;
        p->pcm8(ch, adrs, mode, len);

        MXWORK_CH* work;
        if (ch == 0)
            work = &((MXWORK_CH*)MXDRV_GetWork(MXDRV_WORK_FM))[8];
        else
            work = &((MXWORK_CH*)MXDRV_GetWork(MXDRV_WORK_PCM))[ch - 1];
        int note = (work->S0012 + 27) >> 6;
        p->setNote(ch, note - 40);

        return 0;
    }

    int X68Sound_Pcm8_Abort()
    {
        auto* p = getMXDRVSoundSystemSet().m6258;
        p->resetPCM8();
        return 0;
    }
}
