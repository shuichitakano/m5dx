/*
 * author : Shuichi TAKANO
 * since  : Tue Jan 29 2019 3:43:41
 */
#ifndef ECDD31DF_D134_1396_3467_99EE5D027492
#define ECDD31DF_D134_1396_3467_99EE5D027492

#include <algorithm>
#include <limits>
#include <stdint.h>

namespace ui
{

struct Vec2
{
    int x, y;

public:
    void operator+=(const Vec2& v)
    {
        x += v.x;
        y += v.y;
    }
    void operator-=(const Vec2& v)
    {
        x -= v.x;
        y -= v.y;
    }
};

inline Vec2
operator+(const Vec2& a, const Vec2& b)
{
    return {a.x + b.x, a.y + b.y};
}

inline Vec2
operator-(const Vec2& a, const Vec2& b)
{
    return {a.x - b.x, a.y - b.y};
}

inline Vec2
min(const Vec2& a, const Vec2& b)
{
    return {std::min(a.x, b.x), std::min(a.y, b.y)};
}

inline Vec2
max(const Vec2& a, const Vec2& b)
{
    return {std::max(a.x, b.x), std::max(a.y, b.y)};
}

struct Dim2
{
    uint32_t w, h;
};

struct Rect
{
    Vec2 pos;
    Dim2 size;
};

struct BBox
{
    Vec2 p[2];

public:
    BBox() = default;
    BBox(const Vec2& p0, const Vec2& p1)
        : p{p0, p1}
    {
    }

    BBox(const Vec2& pos, const Dim2& size)
    {
        p[0]   = pos;
        p[1].x = pos.x + size.w;
        p[1].y = pos.y + size.h;
    }

    void invalidate()
    {
        p[0] = {std::numeric_limits<int>::max(),
                std::numeric_limits<int>::max()};
        p[1] = {std::numeric_limits<int>::min(),
                std::numeric_limits<int>::min()};
    }

    int getWidth() const { return p[1].x - p[0].x; }
    int getHeight() const { return p[1].y - p[0].y; }

    bool isValid() const { return p[0].x <= p[1].x && p[0].y <= p[1].y; }

    void update(const Vec2& v)
    {
        p[0].x = std::min(p[0].x, v.x);
        p[0].y = std::min(p[0].y, v.y);
        p[1].x = std::max(p[1].x, v.x);
        p[1].y = std::max(p[1].y, v.y);
    }

    void update(const BBox& b)
    {
        p[0].x = std::min(p[0].x, b.p[0].x);
        p[0].y = std::min(p[0].y, b.p[0].y);
        p[1].x = std::max(p[1].x, b.p[1].x);
        p[1].y = std::max(p[1].y, b.p[1].y);
    }

    void update(const Rect& r)
    {
        p[0].x = std::min(p[0].x, r.pos.x);
        p[0].y = std::min(p[0].y, r.pos.y);
        p[1].x = std::max<int>(p[1].x, r.pos.x + r.size.w);
        p[1].y = std::max<int>(p[1].y, r.pos.y + r.size.h);
    }

    bool isIntersect(const BBox& t) const
    {
        return ((p[0].x < t.p[1].x && p[0].y < t.p[1].y) &&
                (p[1].x > t.p[0].x && p[1].y > t.p[0].y));
    }

    void intersect(const BBox& t)
    {
        p[0].x = std::max(p[0].x, t.p[0].x);
        p[0].y = std::max(p[0].y, t.p[0].y);
        p[1].x = std::min(p[1].x, t.p[1].x);
        p[1].y = std::min(p[1].y, t.p[1].y);
    }
};

} // namespace ui

#endif /* ECDD31DF_D134_1396_3467_99EE5D027492 */
