/*
 * author : Shuichi TAKANO
 * since  : Sat Sep 19 2020 03:39:30
 */

#include "fft.h"
#include <utility>

namespace util
{

/// @param sincosTable sin[0]*16384, cos[0]*16384, ...
void
realFFT(int16_t* a, const int16_t* sincosTable, int n)
{
    // http://www.kurims.kyoto-u.ac.jp/~ooura/fftman/ftmn2_12.html#sec2_1_2

    /* ---- scrambler ---- */
    int i = 0;
    for (int j = 1; j < n - 1; ++j)
    {
        for (int k = n >> 1; k > (i ^= k); k >>= 1)
        {
        }
        if (j < i)
        {
            std::swap(a[i], a[j]);
        }
    }

    int theta_step = n << 1; // 2pi

    int m, mh;
    for (mh = 1; (m = mh << 1) <= n; mh = m)
    {
        int mq = mh >> 1;
        theta_step >>= 1;
        /* ---- real to real butterflies (W == 1) ---- */
        for (int jr = 0; jr < n; jr += m)
        {
            int kr  = jr + mh;
            auto xr = a[kr];
            a[kr]   = a[jr] - xr;
            a[jr] += xr;
        }
        /* ---- complex to complex butterflies (W != 1) ---- */
        auto sincosp = sincosTable;
        for (int i = 1; i < mq; ++i)
        {
            sincosp += theta_step;
            int16_t wi = -sincosp[0]; // sin (theta * i);
            int16_t wr = sincosp[1];  // cos (theta * i);
            for (int j = 0; j < n; j += m)
            {
                int jr     = j + i;
                int ji     = j + mh - i;
                int kr     = j + mh + i;
                int ki     = j + m - i;
                int16_t xr = (wr * a[kr] + wi * a[ki]) >> 14;
                int16_t xi = (wr * a[ki] - wi * a[kr]) >> 14;
                a[kr]      = -a[ji] + xi;
                a[ki]      = a[ji] + xi;
                a[ji]      = a[jr] - xr;
                a[jr]      = a[jr] + xr;
            }
        }
        /* ---- real to complex butterflies are trivial ---- */
    }
}

} // namespace util
