/* -*- mode:C++; -*-
 *
 * author(s) : Shuichi TAKANO
 * since 2014/06/30(Mon) 03:18:46
 *
 */

#include "mdxplayer.h"
#include "../debug.h"
#include <algorithm>
#include <audio/sample_generator.h>
#include <audio/sound_chip_manager.h>
#include <io/file_util.h>
#include <mxdrv/mxdrv.h>
#include <mxdrv/sys.h>
#include <string.h>

#include <audio/audio.h>

namespace music_player
{

namespace
{
struct IsNotText
{
    bool operator()(uint8_t c)
    {
        return (c < 32) && (c != 0xa) && (c != 0xd) && (c != 0x1b);
    }
};

void
removeEscapeSequence(std::string& str)
{
    int pos = 0;
    while (1)
    {
        pos = str.find(0x1b, pos);
        if (pos == (int)std::string::npos)
            break;
        str.erase(pos, 2);
    }
}

} // namespace

bool
MDXPlayer::loadMDX(const char* filename)
{
    ByteArray().swap(mdx_);

    int size = io::getFileSize(filename);
    if (size < 0)
    {
        DBOUT(("file stat error. %s\n", filename));
        return false;
    }

    mdx_.resize(size + 8);
    if (io::readFile(mdx_.data() + 8, filename, size) < 0)
    {
        DBOUT(("file read error. %s\n", filename));
        return false;
    }

    if (!analyzeTitle())
    {
        DBOUT(("title analyze error.\n"));
        return false;
    }

    std::string pdxName;
    if (!analyzePDXFilename(pdxName))
        return false;

    if (!pdxName.empty() && !loadPDX(filename, pdxName))
    {
        DBOUT(("PDX load error %s %s.\n", filename, pdxName.c_str()));
    }

    //
    int mdxBodyOfs = title_.size() + 3 + pdxName.size() + 1 + 8 /*header*/;
    mdx_[0]        = 0;
    mdx_[1]        = 0;
    mdx_[2]        = pdx_.size() ? 0 : 0xff;
    mdx_[3]        = mdx_[2];
    mdx_[4]        = mdxBodyOfs >> 8;
    mdx_[5]        = mdxBodyOfs;
    mdx_[6]        = 0;
    mdx_[7]        = 8;

    mdx_[8 + title_.size()] = 0;
    removeEscapeSequence(title_);

    initialized_     = false;
    playTimeByClock_ = 0;

    DBOUT(("MDX loaded.\n"));
    return true;
}

bool
MDXPlayer::isSupported(const char* filename)
{
    auto p = strrchr(filename, '.');
    if (p)
        return strcasecmp(p, ".MDX") == 0;
    return false;
}

void
MDXPlayer::freeAudioChips()
{
    pcm8_.setCallback(nullptr);
    audio::getSampleGeneratorManager().remove(&pcm8_);
    freeYM2151(ym2151_.detachChip());
}

bool
MDXPlayer::start()
{
    DBOUT(("start MDX!\n"));
    freeAudioChips();

    ym2151_.setChip(audio::allocateYM2151());
    pcm8_.initialize();
    audio::getSampleGeneratorManager().add(&pcm8_);

    auto& sys  = getMXDRVSoundSystemSet();
    sys.ym2151 = &ym2151_;
    sys.m6258  = &pcm8_;

    audio::setFMVolume(0.5f);
    pcm8_.setVolume(0.5f);

    auto pcm8 = (UBYTE*)MXDRV_GetWork(MXDRV_WORK_PCM8);
    *pcm8     = 1;

    return true;
}

bool
MDXPlayer::terminate()
{
    DBOUT(("terminate MDX!\n"));
    stop();
    MXDRV_End();
    ByteArray().swap(mdx_);
    ByteArray().swap(pdx_);
    std::string().swap(currentPDXPath_);
    std::string().swap(pdxPath_);
    std::string().swap(title_);

    freeAudioChips();
    return true;
}

bool
MDXPlayer::load(const char* filename)
{
    return loadMDX(filename);
}

bool
MDXPlayer::play(int)
{
    MXDRV_Stop();

    void* pdx = pdx_.size() ? &pdx_[0] : 0;

    //  if (!initialized_)
    {
        int res = MXDRV_Start(&mdx_[0], mdx_.size(), pdx, pdx_.size());
        if (res)
        {
            DBOUT(("mxdrv start error. %d\n", res));
            return false;
        }
        initialized_ = true;
    }
    fadeout_ = false;
    paused_  = false;

    MXDRV_Play(&mdx_[0], mdx_.size(), pdx, pdx_.size());

    return true;
}

bool
MDXPlayer::stop()
{
    MXDRV_Stop();
    return true;
}

bool
MDXPlayer::pause()
{
    MXDRV_Pause();
    paused_ = true;
    return true;
}

bool
MDXPlayer::cont()
{
    paused_ = false;
    if (isFinished())
    {
        MXDRV_Replay();
        fadeout_ = false;
    }
    else
    {
        MXDRV_Cont();
    }
    return true;
}

bool
MDXPlayer::fadeout()
{
    if (!fadeout_)
    {
        MXDRV_Fadeout();
        fadeout_ = true;
    }
    return true;
}

bool
MDXPlayer::isFinished() const
{
    auto g = (MXWORK_GLOBAL*)MXDRV_GetWork(MXDRV_WORK_GLOBAL);
    return g->L001e13 != 0;
}

bool
MDXPlayer::isPaused() const
{
    return paused_;
}

int
MDXPlayer::getCurrentLoop() const
{
    auto g = (MXWORK_GLOBAL*)MXDRV_GetWork(MXDRV_WORK_GLOBAL);
    return isFinished() ? 0 : g->L002246;
}

float
MDXPlayer::getPlayTime() const
{
    auto g = (MXWORK_GLOBAL*)MXDRV_GetWork(MXDRV_WORK_GLOBAL);
    return g->PLAYTIME * (1024 / 4000000.0f); // Timer B
}

FileFormat
MDXPlayer::getFormat() const
{
    return FileFormat::MDX;
}

sound_sys::SoundSystem*
MDXPlayer::getSystem(int idx)
{
    switch (idx)
    {
    case 0:
        return &ym2151_;

    case 1:
        return &pcm8_;
    }
    return 0;
}

bool
MDXPlayer::analyzeTitle()
{
    std::string().swap(title_);

    auto top = mdx_.begin() + 8;
    auto end = mdx_.end();

    auto tail = std::find(top, end, 0x1a);
    if ((tail == mdx_.end()) || (tail - mdx_.begin() < 2) ||
        (*(tail - 1) != 0xa) || (*(tail - 2) != 0xd))
        return false;

    tail -= 2;

    auto ntp = std::find_if(top, tail, IsNotText());
    if (ntp != tail)
    {
        return false;
    }

    title_.assign((const char*)&*top, (const char*)&*tail);
    DBOUT(("title: %s\n", title_.c_str()));
    return true;
}

bool
MDXPlayer::analyzePDXFilename(std::string& name) const
{
    auto top  = mdx_.begin() + title_.size() + 3 + 8 /*header*/;
    auto tail = std::find(top, mdx_.end(), 0);
    if (tail == mdx_.end())
        return false;

    name.assign((const char*)&*top, (const char*)&*tail);
    DBOUT(("pdx: %s\n", name.c_str()));
    return true;
}

bool
MDXPlayer::loadPDX(const char* mdxFilename, const std::string& pdxName)
{
    const char* mdxPathTail = strrchr(mdxFilename, '/');
    std::string mdxPath(mdxFilename,
                        mdxPathTail ? mdxPathTail + 1 : mdxFilename);

    DBOUT(("loadPDX:%s, path %s, PDX path %s\n",
           pdxName.c_str(),
           mdxPath.c_str(),
           pdxPath_.c_str()));

    auto path_size = [&]() -> std::pair<std::string, int> {
        auto path = mdxPath + pdxName;
        int size  = io::getFileSize(path.c_str());
        if (size >= 0)
        {
            return {path, size};
        }

        path += ".pdx";
        size = io::getFileSize(path.c_str());
        if (size >= 0)
        {
            return {path, size};
        }

        path = pdxPath_ + pdxName;
        size = io::getFileSize(path.c_str());
        if (size >= 0)
        {
            return {path, size};
        }

        path += ".pdx";
        size = io::getFileSize(path.c_str());
        if (size >= 0)
        {
            return {path, size};
        }

        return {};
    }();

    if (path_size.first.empty())
    {
        DBOUT(("pdx not found.\n"));
        path_size.first.swap(currentPDXPath_);
        return false;
    }

    if (path_size.first == currentPDXPath_)
    {
        DBOUT(("pdx %s already loaded.\n", currentPDXPath_.c_str()));
        return false;
    }

    currentPDXPath_ = path_size.first;
    DBOUT(("PDX : %s\n", currentPDXPath_.c_str()));

    //
    ByteArray().swap(pdx_);

    int fileSize   = path_size.second;
    int pdxbodyptr = (8 + currentPDXPath_.size() + 1) & (-2);
    pdx_.resize(fileSize + pdxbodyptr);
    if (io::readFile(
            pdx_.data() + pdxbodyptr, currentPDXPath_.c_str(), fileSize) < 0)
    {
        DBOUT(("load PDX error"));
        return false;
    }

    strcpy((char*)&pdx_[8], currentPDXPath_.c_str());

    pdx_[0] = 0x00;
    pdx_[1] = 0x00;
    pdx_[2] = 0x00;
    pdx_[3] = 0x00;
    pdx_[4] = pdxbodyptr >> 8;
    pdx_[5] = pdxbodyptr;
    pdx_[6] = currentPDXPath_.size() >> 8;
    pdx_[7] = currentPDXPath_.size();

    return true;
}

void
MDXPlayer::setPDXPath(const char* s)
{
    pdxPath_ = s;
}

const char*
MDXPlayer::getTitle() const
{
    return title_.c_str();
}

int
MDXPlayer::getTrackCount() const
{
    return 0;
}

int
MDXPlayer::getCurrentTrack() const
{
    return 0;
}

std::experimental::optional<std::string>
MDXPlayer::loadTitle(const char* filename)
{
    std::vector<char> buf(81 + 3 /* 0xd, 0xa, 0x1a */);
    int readBytes = io::readFile(buf.data(), filename, buf.size());
    if (readBytes < 0)
    {
        DBOUT(("get title error.\n"));
        return {};
    }

    buf.resize(readBytes);

    auto tail = std::find(buf.begin(), buf.end(), 0xd);
    auto ntp  = std::find_if(buf.begin(), tail, IsNotText());
    if (ntp != tail)
    {
        return {};
    }

    std::string s(buf.begin(), tail);
    removeEscapeSequence(s);

    return s;
}

} // namespace music_player