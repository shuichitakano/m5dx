/*
 * author : Shuichi TAKANO
 * since  : Fri Nov 09 2018 2:1:5
 */
#ifndef D34D3B79_3133_F071_1EAC_9BF1385D2C0C
#define D34D3B79_3133_F071_1EAC_9BF1385D2C0C

namespace audio
{

class SoundChipBase
{
public:
    virtual ~SoundChipBase() noexcept = default;

    virtual void setValue(int addr, int v) = 0;
    virtual int getValue(int addr)         = 0;
    virtual int setClock(int clock)        = 0;
};

} // namespace audio

#endif /* D34D3B79_3133_F071_1EAC_9BF1385D2C0C */
