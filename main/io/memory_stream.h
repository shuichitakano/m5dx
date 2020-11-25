/*
 * author : Shuichi TAKANO
 * since  : Sat Oct 03 2020 04:48:19
 */
#ifndef _638A98E2_4134_3E2E_445E_7DA0A312AD24
#define _638A98E2_4134_3E2E_445E_7DA0A312AD24

#include "stream.h"

namespace io
{

class MemoryBinaryStream final : public BinaryStream
{
public:
    MemoryBinaryStream(const void* p = 0, size_t s = 0);
    void setMemory(const void* p, size_t s);

    bool seek(int pos, bool tail = false) override;
    uint32_t tell() const override;
    const char* peek(size_t size) override;
    DataPtr get(size_t size) override;
    bool advance(size_t size) override;
    bool isEndOfStream() const override;
    uint_fast8_t getU8() override;
    uint_fast16_t getU16() override;
    uint_fast32_t getU32() override;
    uint_fast16_t getU16Aligned() override;
    uint_fast32_t getU32Aligned() override;

private:
    const uint8_t* p_;
    const uint8_t* top_;
    size_t size_;
};

} // namespace io

#endif /* _638A98E2_4134_3E2E_445E_7DA0A312AD24 */
