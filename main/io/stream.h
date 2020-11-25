/*
 * author : Shuichi TAKANO
 * since  : Sat Oct 03 2020 03:58:52
 */
#ifndef F970744A_A134_3E2E_36B1_BE1476DC8EF6
#define F970744A_A134_3E2E_36B1_BE1476DC8EF6

#include <memory>
#include <stddef.h>
#include <stdint.h>

namespace io
{

class BinaryStream
{
public:
    struct Deleter
    {
        bool allocated_;
        void operator()(const uint8_t* p) { delete[](allocated_ ? p : 0); }
    };
    using DataPtr = std::unique_ptr<const uint8_t[], Deleter>;

    static DataPtr makeDataPtr(size_t s) { return {new uint8_t[s], {true}}; }
    static DataPtr makeRefPtr(const uint8_t* p) { return {p, {false}}; }

public:
    virtual ~BinaryStream()           = default;
    BinaryStream()                    = default;
    BinaryStream(const BinaryStream&) = delete;
    BinaryStream& operator=(const BinaryStream&) = delete;

    virtual bool seek(int pos, bool tail = false) = 0;
    virtual uint32_t tell() const                 = 0;
    virtual const char* peek(size_t size)         = 0;
    virtual DataPtr get(size_t size);

    virtual bool advance(size_t size)  = 0;
    virtual bool isEndOfStream() const = 0;

    char getChar();
    virtual uint_fast8_t getU8();
    virtual uint_fast16_t getU16();
    virtual uint_fast32_t getU32();
    virtual uint_fast16_t getU16Aligned();
    virtual uint_fast32_t getU32Aligned();

    int8_t get8() { return (int8_t)getU8(); }
    int16_t get16() { return (int16_t)getU16(); }
    int32_t get32() { return (int32_t)getU32(); }
    int16_t get16Aligned() { return (int16_t)getU16Aligned(); }
    int32_t get32Aligned() { return (int32_t)getU32Aligned(); }
    float getFloat();
    float getFloatAligned();
};

} // namespace io

#endif /* F970744A_A134_3E2E_36B1_BE1476DC8EF6 */
