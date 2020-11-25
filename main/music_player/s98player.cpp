/*
 * author : Shuichi TAKANO
 * since  : Wed Sep 30 2020 01:54:31
 */

#include "s98player.h"
#include "../debug.h"
#include <audio/audio.h>
#include <audio/sound_chip_manager.h>
#include <io/file_stream.h>
#include <io/file_util.h>
#include <string.h>
#include <system/timer.h>
#include <system/util.h>

namespace music_player
{

class S98Player::YM2608 : public S98Player::DeviceInterface
{
    sound_sys::YMF288 sys_;

public:
    YM2608()
    {
        sys_.setChip(audio::allocateYMF288());

        write(false, 0x11, 63); // set rhythm volume
    }
    ~YM2608() override { audio::freeYMF288(sys_.detachChip()); }

    void initialize(uint32_t clock, uint32_t pan) override
    {
        DBOUT(("initialize YM2608: clock %d\n", clock));
        (void)pan;
        sys_.setClock(clock);
    }

    void write(bool extra, uint_fast8_t r, uint_fast8_t v) override
    {
        //        DBOUT(("write YM2608: %03x:%02x\n", r, v));
        int h = extra ? 2 : 0;
        sys_.setValue(h | 0, r);
        sys_.setValue(h | 1, v);
    }

    void mute() override { sys_.allKeyOff(); }

    sound_sys::SoundSystem* getSoundSystem() override { return &sys_; }
};

S98Player::S98Player()
{
    deviceInterfaces_.reserve(2);
}

bool
S98Player::isSupported(const char* filename)
{
    auto p = strrchr(filename, '.');
    return p ? strcasecmp(p, ".S98") == 0 : false;
}

std::experimental::optional<std::string>
S98Player::loadTitle(const char* filename)
{
    FILE* fp = fopen(filename, "rb");
    if (!fp)
    {
        DBOUT(("'%s' open error.\n", filename));
        return {};
    }

    io::FileBinaryStream stream(fp);
    Header h;
    h.load(&stream);
    fclose(fp);

    return h.findTitle();
}

bool
S98Player::start()
{
    if (started_)
    {
        return true;
    }

    DBOUT(("start\n"));
    audio::setFMVolume(1.0f);
    // audio::setFMVolume(2.0f);
    sys::initTimer(1000000);
    sys::setTimerPeriod(1000, true);
    sys::setTimerCallback([&] { tick(); });
    sys::startTimer();
    sys::enableTimerInterrupt();

    started_ = true;

    return true;
}

bool
S98Player::terminate()
{
    DBOUT(("terminate\n"));
    started_ = false;

    sys::stopTimer();
    sys::resetTimerCallback();

    stop();

    finalizeDeviceInterfaces();
    decltype(data_)().swap(data_);
    header_ = Header();
    std::string().swap(title_);
    return true;
}

bool
S98Player::load(const char* filename)
{
    if (!io::readFile(data_, filename))
    {
        DBOUT(("'%s' load error.\n", filename));
        return false;
    }

    stream_.setMemory(data_.data(), data_.size());
    if (!header_.load(&stream_))
    {
        DBOUT(("'%s' parse error.\n", filename));
        decltype(data_)().swap(data_);
        return false;
    }

    title_ = header_.findTitle();
    createDeviceInterfaces();

    DBOUT(("time base: %d/%d\n",
           header_.timerNumerator_,
           header_.timerDenominator_));
    samplesPerUs_ = header_.getSamplesPerUs();
    DBOUT(("%f sample/us\n", samplesPerUs_));

    return true;
}

bool
S98Player::play(int track)
{
    if (data_.empty())
    {
        return false;
    }
    stream_.seek(header_.startOffset_);
    paused_      = false;
    loopCount_   = 0;
    wait_        = 0;
    playing_     = true;
    totalTimeUs_ = 0;
    prevTimeUs_  = sys::micros();

    return true;
}

bool
S98Player::stop()
{
    playing_ = false;
    for (auto& d : deviceInterfaces_)
    {
        d->mute();
    }
    return true;
}

bool
S98Player::pause()
{
    paused_ = true;
    for (auto& d : deviceInterfaces_)
    {
        d->mute();
    }
    return true;
}

bool
S98Player::cont()
{
    paused_ = false;
    return true;
}

bool
S98Player::fadeout()
{
    playing_ = false;
    return true;
}

bool
S98Player::isFinished() const
{
    return !playing_;
}

bool
S98Player::isPaused() const
{
    return paused_;
}

int
S98Player::getCurrentLoop() const
{
    return loopCount_;
}

int
S98Player::getTrackCount() const
{
    return -1;
}

int
S98Player::getCurrentTrack() const
{
    return -1;
}

float
S98Player::getPlayTime() const
{
    return totalTimeUs_ * 0.000001f;
}

const char*
S98Player::getTitle() const
{
    return title_.c_str();
}

FileFormat
S98Player::getFormat() const
{
    return FileFormat::S98;
}

sound_sys::SoundSystem*
S98Player::getSystem(int idx)
{
    if (idx < static_cast<int>(deviceInterfaces_.size()))
    {
        return deviceInterfaces_[idx]->getSoundSystem();
    }
    return nullptr;
}

void
S98Player::tick()
{
    auto curTime = sys::micros();
    auto dt      = curTime - prevTimeUs_;
    auto samples = dt * samplesPerUs_;
    prevTimeUs_  = curTime;

    if (!(playing_ && !paused_))
    {
        return;
    }

    // if (wait_)
    //     DBOUT(("dt %d, %d samples, wait %d\n", dt, samples, wait_));

    while (playing_ && !paused_ && wait_ <= samples)
    {

        samples -= wait_;
        wait_ = 0;
        processCommand(&stream_);
    }
    wait_ -= samples;
    totalTimeUs_ += dt;
}

void
S98Player::finalizeDeviceInterfaces()
{
    deviceInterfaces_.clear();
}

void
S98Player::createDeviceInterfaces()
{
    auto create = [](auto type) -> std::unique_ptr<DeviceInterface> {
        switch (type)
        {
        case Header::DeviceType::OPNA:
            DBOUT(("create OPNA\n"));
            return std::make_unique<YM2608>();

        case Header::DeviceType::OPN:
            DBOUT(("create OPN\n"));
            return std::make_unique<YM2608>();

        default:
            return {};
        }
    };

    finalizeDeviceInterfaces();
    for (auto& di : header_.deviceInfos_)
    {
        auto p = std::unique_ptr<DeviceInterface>(create(di.type_));
        if (p)
        {
            p->initialize(di.clock_, di.pan_);
        }
        deviceInterfaces_.push_back(std::move(p));
    }

    DBOUT(("%d device interfaces.\n", deviceInterfaces_.size()));
}

void
S98Player::processCommand(io::BinaryStream* stream)
{
    auto cmd = stream->getU8();
    if (cmd < header_.deviceInfos_.size() << 1)
    {
        int dev  = cmd >> 1;
        int addr = cmd & 1;
        auto reg = stream->getU8();
        auto val = stream->getU8();

        // DBOUT(("write device[%d]: addr %d, reg 0x%02x = 0x%02x\n",
        //        dev,
        //        addr,
        //        reg,
        //        val));
        auto& p = deviceInterfaces_[dev];
        if (p)
        {
            p->write(addr, reg, val);
        }
    }
    else
    {
        switch (cmd)
        {
        case 0xfd:
            DBOUT(("end of data.\n"));
            if (header_.loopOffset_)
            {
                stream->seek(header_.loopOffset_);
                ++loopCount_;
            }
            else
            {
                playing_ = false;
            }
            break;

        case 0xfe:
        {
            int shift = 0;
            int v     = 0;
            uint_fast8_t d;
            do
            {
                d = stream->getU8();
                v |= (d & 127) << shift;
                shift += 7;
            } while (d & 128);
            wait_ = v + 2;
        }
        break;

        case 0xff:
            wait_ = 1;
            break;
        }
    }
}

//
bool
S98Player::Header::load(io::BinaryStream* stream)
{
    deviceInfos_.clear();
    if (!stream)
    {
        return false;
    }

    uint8_t magic[3];
    magic[0] = stream->getU8();
    magic[1] = stream->getU8();
    magic[2] = stream->getU8();
    if (magic[0] != 'S' || magic[1] != '9' || magic[2] != '8')
    {
        DBOUT(("invalid magic\n"));
        return false;
    }

    uint8_t version = stream->getU8();
    if (version > '3')
    {
        DBOUT(("invalid version '%c'\n", version));
        return false;
    }

    timerNumerator_   = stream->getU32Aligned();
    timerDenominator_ = stream->getU32Aligned();
    if (timerNumerator_ == 0)
    {
        timerNumerator_ = 10;
    }
    if (timerDenominator_ == 0)
    {
        timerDenominator_ = 1000;
    }

    stream->getU32Aligned(); // skip compressing

    auto tagOfs  = stream->getU32Aligned();
    startOffset_ = stream->getU32Aligned();
    loopOffset_  = stream->getU32Aligned();

    int deviceCount = stream->getU32Aligned();
    if (deviceCount == 0)
    {
        DeviceInfo di;
        di.type_  = DeviceType::OPNA;
        di.clock_ = 7987200;
        di.pan_   = 0;
        deviceInfos_.push_back(di);
    }
    else
    {
        while (deviceCount--)
        {
            DeviceInfo di;
            di.type_  = static_cast<DeviceType>(stream->getU32Aligned());
            di.clock_ = stream->getU32Aligned();
            di.pan_   = stream->getU32Aligned();
            deviceInfos_.push_back(di);
        }
    }

    // DBOUT(("tick %d/%d = %f, start 0x%x, loop 0x%x\n",
    //        timerNumerator_,
    //        timerDenominator_,
    //        (float)timerNumerator_ / timerDenominator_,
    //        startOffset_,
    //        loopOffset_));

    // for (auto i = 0u; i < deviceInfos_.size(); ++i)
    // {
    //     auto& di = deviceInfos_[i];
    // DBOUT((" dev[%d] : type %d, clock %d, pan %d\n",
    //        i,
    //        (int)di.type_,
    //        di.clock_,
    //        di.pan_));
    // }

    if (tagOfs)
    {
        stream->seek(tagOfs);
        loadTags(stream);
    }

    return true;
}

bool
S98Player::Header::loadTags(io::BinaryStream* stream)
{
    if (stream->getU8() != '[' || stream->getU8() != 'S' ||
        stream->getU8() != '9' || stream->getU8() != '8' ||
        stream->getU8() != ']')
    {
        return false;
    }

    auto appendTag = [&](const std::string& str) {
        auto p = str.find_first_of('=');
        if (p != std::string::npos)
        {
            auto tag   = str.substr(0, p);
            auto val   = str.substr(p + 1);
            tags_[tag] = val;
            //            DBOUT(("tag %s = %s\n", tag.c_str(), val.c_str()));
        }
    };

    std::string str;
    while (!stream->isEndOfStream())
    {
        char c = (char)stream->getU8();
        if (c == 0xa || c == 0)
        {
            appendTag(str);
            str.clear();
        }
        else
        {
            str.push_back(c);
        }
    }
    appendTag(str);

    return true;
}

std::string
S98Player::Header::findTitle() const
{
    auto p = tags_.find("title");
    if (p != tags_.end())
    {
        return p->second;
    }
    return {};
}

float
S98Player::Header::getSamplesPerUs() const
{
    float samplePerSec = (float)timerDenominator_ / timerNumerator_;
    return samplePerSec / 1000000;
}

} // namespace music_player
