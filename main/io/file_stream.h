/*
 * author : Shuichi TAKANO
 * since  : Sat Oct 03 2020 04:22:42
 */
#ifndef CB827CD8_2134_3E2E_4074_070A89EB0C90
#define CB827CD8_2134_3E2E_4074_070A89EB0C90

#include "stream.h"
#include <stdio.h>
#include <vector>

namespace io
{

class FileBinaryStream final : public BinaryStream
{
public:
    FileBinaryStream(FILE* fp = 0, int s = -1)
        : fp_(fp)
        , size_(s)
        , adj_(0)
        , cache_(16)
    {
    }

    void setFile(FILE* fp, int s = -1)
    {
        fp_   = fp;
        size_ = s;
        adj_  = 0;
    }

    bool seek(int pos, bool tail = false) override;
    uint32_t tell() const override;
    const char* peek(size_t size) override;
    DataPtr get(size_t size) override;
    bool advance(size_t size) override;
    bool isEndOfStream() const override;

    void flush();

protected:
    void adjustPointer() const;

private:
    FILE* fp_;
    int32_t size_ = -1;

    mutable int adj_;
    std::vector<char> cache_;
};

} // namespace io

#endif /* CB827CD8_2134_3E2E_4074_070A89EB0C90 */
