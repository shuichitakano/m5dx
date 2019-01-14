/*
 * author : Shuichi TAKANO
 * since  : Mon Jan 14 2019 3:17:7
 */
#ifndef _4ED5A5A1_8134_1395_3063_9BC590170DC6
#define _4ED5A5A1_8134_1395_3063_9BC590170DC6

namespace util
{

template <class T>
class SimpleRingBuffer
{
public:
    typedef T value_type;

public:
    SimpleRingBuffer(T* p = 0, int size = 0) { set(p, size); }
    void set(T* p, int size)
    {
        buffer_ = p;
        cur_    = 0;
        mask_   = size - 1;
    }
    // sizeは2^n

    T* getBufferTop() { return buffer_; }
    const T* getBufferTop() const { return buffer_; }
    int getMask() const { return mask_; }
    int getCurrentPos() const { return cur_; }
    void setCurrentPos(int i) { cur_ = i; }
    T& getCurrent() { return buffer_[cur_]; }
    const T& getCurrent() const { return buffer_[cur_]; }
    void advancePointer(int n) { cur_ = (cur_ + n) & mask_; }

private:
    T* buffer_;
    int cur_;
    int mask_;
};
} // namespace util

#endif /* _4ED5A5A1_8134_1395_3063_9BC590170DC6 */
