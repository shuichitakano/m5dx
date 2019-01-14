/*
 * author : Shuichi TAKANO
 * since  : Sun Jan 13 2019 22:56:19
 */
#ifndef BB3CD18D_9134_1394_1584_73E373F02F5E
#define BB3CD18D_9134_1394_1584_73E373F02F5E

#include <array>
#include <stdint.h>

namespace audio
{

class AudioStreamOut
{
public:
    virtual ~AudioStreamOut() = default;

    virtual void onUpdateAudioStream(std::array<int32_t, 2>* data,
                                     size_t nSamples,
                                     size_t sampleRate) = 0;

    // data は 16.8の固定小数
};

} // namespace audio

#endif /* BB3CD18D_9134_1394_1584_73E373F02F5E */
