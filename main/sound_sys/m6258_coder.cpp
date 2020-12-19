/*
 * author : Shuichi TAKANO
 * since  : Sun Feb 17 2019 2:40:31
 */

#include "m6258_coder.h"

namespace sound_sys
{

#define M6258_DEFINE_TBL()                                                     \
    M6258_DEFINE_TBLx(0), M6258_DEFINE_TBLx(1), M6258_DEFINE_TBLx(2),          \
        M6258_DEFINE_TBLx(3), M6258_DEFINE_TBLx(4), M6258_DEFINE_TBLx(5),      \
        M6258_DEFINE_TBLx(6), M6258_DEFINE_TBLx(7), M6258_DEFINE_TBLx(8),      \
        M6258_DEFINE_TBLx(9), M6258_DEFINE_TBLx(10), M6258_DEFINE_TBLx(11),    \
        M6258_DEFINE_TBLx(12), M6258_DEFINE_TBLx(13), M6258_DEFINE_TBLx(14),   \
        M6258_DEFINE_TBLx(15), M6258_DEFINE_TBLx(16), M6258_DEFINE_TBLx(17),   \
        M6258_DEFINE_TBLx(18), M6258_DEFINE_TBLx(19), M6258_DEFINE_TBLx(20),   \
        M6258_DEFINE_TBLx(21), M6258_DEFINE_TBLx(22), M6258_DEFINE_TBLx(23),   \
        M6258_DEFINE_TBLx(24), M6258_DEFINE_TBLx(25), M6258_DEFINE_TBLx(26),   \
        M6258_DEFINE_TBLx(27), M6258_DEFINE_TBLx(28), M6258_DEFINE_TBLx(29),   \
        M6258_DEFINE_TBLx(30), M6258_DEFINE_TBLx(31), M6258_DEFINE_TBLx(32),   \
        M6258_DEFINE_TBLx(33), M6258_DEFINE_TBLx(34), M6258_DEFINE_TBLx(35),   \
        M6258_DEFINE_TBLx(36), M6258_DEFINE_TBLx(37), M6258_DEFINE_TBLx(38),   \
        M6258_DEFINE_TBLx(39), M6258_DEFINE_TBLx(40), M6258_DEFINE_TBLx(41),   \
        M6258_DEFINE_TBLx(42), M6258_DEFINE_TBLx(43), M6258_DEFINE_TBLx(44),   \
        M6258_DEFINE_TBLx(45), M6258_DEFINE_TBLx(46), M6258_DEFINE_TBLx(47),   \
        M6258_DEFINE_TBLx(48),

#define M6258_DEFINE_TBLx(i)                                                   \
    M6258_TBL_VAL(0, i), M6258_TBL_VAL(1, i), M6258_TBL_VAL(2, i),             \
        M6258_TBL_VAL(3, i), M6258_TBL_VAL(4, i), M6258_TBL_VAL(5, i),         \
        M6258_TBL_VAL(6, i), M6258_TBL_VAL(7, i), M6258_TBL_VAL(8, i),         \
        M6258_TBL_VAL(9, i), M6258_TBL_VAL(10, i), M6258_TBL_VAL(11, i),       \
        M6258_TBL_VAL(12, i), M6258_TBL_VAL(13, i), M6258_TBL_VAL(14, i),      \
        M6258_TBL_VAL(15, i)

#define M6258_TBL_VAL(x, i)                                                    \
    (((uint16_t)(M6258_GET_DELTA(x, i)) << 16) | (M6258_GET_NEXTI(x, i)))

#define M6258_GET_DELTA(x, i)                                                  \
    M6258_ADJ_SIGN(x, M6258_GET_PRED_VEC(i) * M6258_GET_SCALE(x) >> 3)

#define M6258_GET_NEXTI(x, i) M6258_CLAMPI(M6258_GET_PREDADJ(x) + i)

#define M6258_GET_PREDADJ(x) M6258_GET_PREDADJ_(x)
#define M6258_GET_SCALE(x) M6258_GET_SCALE_(x)
#define M6258_ADJ_SIGN(x, v) M6258_ADJ_SIGN_(x, v)
#define M6258_GET_PRED_VEC(i) M6258_GET_PRED_VEC_(i)
#define M6258_CLAMPI(i) ((i) < 0 ? 0 : ((i) > 48 ? 48 : (i)))

#define M6258_GET_PREDADJ_(x) M6258_PREDADJ_TBL_##x
#define M6258_PREDADJ_TBL_0 -1
#define M6258_PREDADJ_TBL_1 -1
#define M6258_PREDADJ_TBL_2 -1
#define M6258_PREDADJ_TBL_3 -1
#define M6258_PREDADJ_TBL_4 2
#define M6258_PREDADJ_TBL_5 4
#define M6258_PREDADJ_TBL_6 6
#define M6258_PREDADJ_TBL_7 8
#define M6258_PREDADJ_TBL_8 -1
#define M6258_PREDADJ_TBL_9 -1
#define M6258_PREDADJ_TBL_10 -1
#define M6258_PREDADJ_TBL_11 -1
#define M6258_PREDADJ_TBL_12 2
#define M6258_PREDADJ_TBL_13 4
#define M6258_PREDADJ_TBL_14 6
#define M6258_PREDADJ_TBL_15 8

#define M6258_GET_SCALE_(x) M6258_SCALE_TBL_##x
#define M6258_SCALE_TBL_0 1
#define M6258_SCALE_TBL_1 3
#define M6258_SCALE_TBL_2 5
#define M6258_SCALE_TBL_3 7
#define M6258_SCALE_TBL_4 9
#define M6258_SCALE_TBL_5 11
#define M6258_SCALE_TBL_6 13
#define M6258_SCALE_TBL_7 15
#define M6258_SCALE_TBL_8 1
#define M6258_SCALE_TBL_9 3
#define M6258_SCALE_TBL_10 5
#define M6258_SCALE_TBL_11 7
#define M6258_SCALE_TBL_12 9
#define M6258_SCALE_TBL_13 11
#define M6258_SCALE_TBL_14 13
#define M6258_SCALE_TBL_15 15

#define M6258_ADJ_SIGN_(x, v) (M6258_SIGN_TBL_##x(v))
#define M6258_SIGN_TBL_0
#define M6258_SIGN_TBL_1
#define M6258_SIGN_TBL_2
#define M6258_SIGN_TBL_3
#define M6258_SIGN_TBL_4
#define M6258_SIGN_TBL_5
#define M6258_SIGN_TBL_6
#define M6258_SIGN_TBL_7
#define M6258_SIGN_TBL_8 -
#define M6258_SIGN_TBL_9 -
#define M6258_SIGN_TBL_10 -
#define M6258_SIGN_TBL_11 -
#define M6258_SIGN_TBL_12 -
#define M6258_SIGN_TBL_13 -
#define M6258_SIGN_TBL_14 -
#define M6258_SIGN_TBL_15 -

#define M6258_GET_PRED_VEC_(i) M6258_PRED_VEC_TBL_##i
#define M6258_PRED_VEC_TBL_0 16
#define M6258_PRED_VEC_TBL_1 17
#define M6258_PRED_VEC_TBL_2 19
#define M6258_PRED_VEC_TBL_3 21
#define M6258_PRED_VEC_TBL_4 23
#define M6258_PRED_VEC_TBL_5 25
#define M6258_PRED_VEC_TBL_6 28
#define M6258_PRED_VEC_TBL_7 31
#define M6258_PRED_VEC_TBL_8 34
#define M6258_PRED_VEC_TBL_9 37
#define M6258_PRED_VEC_TBL_10 41
#define M6258_PRED_VEC_TBL_11 45
#define M6258_PRED_VEC_TBL_12 50
#define M6258_PRED_VEC_TBL_13 55
#define M6258_PRED_VEC_TBL_14 60
#define M6258_PRED_VEC_TBL_15 66
#define M6258_PRED_VEC_TBL_16 73
#define M6258_PRED_VEC_TBL_17 80
#define M6258_PRED_VEC_TBL_18 88
#define M6258_PRED_VEC_TBL_19 97
#define M6258_PRED_VEC_TBL_20 107
#define M6258_PRED_VEC_TBL_21 118
#define M6258_PRED_VEC_TBL_22 130
#define M6258_PRED_VEC_TBL_23 143
#define M6258_PRED_VEC_TBL_24 157
#define M6258_PRED_VEC_TBL_25 173
#define M6258_PRED_VEC_TBL_26 190
#define M6258_PRED_VEC_TBL_27 209
#define M6258_PRED_VEC_TBL_28 230
#define M6258_PRED_VEC_TBL_29 253
#define M6258_PRED_VEC_TBL_30 279
#define M6258_PRED_VEC_TBL_31 307
#define M6258_PRED_VEC_TBL_32 337
#define M6258_PRED_VEC_TBL_33 371
#define M6258_PRED_VEC_TBL_34 408
#define M6258_PRED_VEC_TBL_35 449
#define M6258_PRED_VEC_TBL_36 494
#define M6258_PRED_VEC_TBL_37 544
#define M6258_PRED_VEC_TBL_38 598
#define M6258_PRED_VEC_TBL_39 658
#define M6258_PRED_VEC_TBL_40 724
#define M6258_PRED_VEC_TBL_41 796
#define M6258_PRED_VEC_TBL_42 875
//#define M6258_PRED_VEC_TBL_42	876	// X68sound
#define M6258_PRED_VEC_TBL_43 963
#define M6258_PRED_VEC_TBL_44 1060
#define M6258_PRED_VEC_TBL_45 1166
#define M6258_PRED_VEC_TBL_46 1282
#define M6258_PRED_VEC_TBL_47 1411
#define M6258_PRED_VEC_TBL_48 1552

namespace
{
constexpr int32_t updateTable_[] = {M6258_DEFINE_TBL()};

constexpr int predictTable_[] = {
    16,  17,  19,           21,  23,   25,   28,   31,   34,   37,
    41,  45,  50,           55,  60,   66,   73,   80,   88,   97,
    107, 118, 130,          143, 157,  173,  190,  209,  230,  253,
    279, 307, 337,          371, 408,  449,  494,  544,  598,  658,
    724, 796, 875 /*876?*/, 963, 1060, 1166, 1282, 1411, 1552,
};

inline constexpr int
getPredictVector(int i)
{
    return predictTable_[i];
}

} /* namespace */

int
M6258Coder::update(int x)
{
#if 0
  int predict = getPredictVector (predictIdx_);
  int scale8 = ((x & 7) << 1) + 1;
  int d = predict * scale8 >> 3;
  currentValue_ += x & 8 ? -d : d;

  static const int predAdjTable[] =
    {
      -1, -1, -1, -1, 2, 4, 6, 8,
      -1, -1, -1, -1, 2, 4, 6, 8,
    };
  int adj = predAdjTable[x];
  predictIdx_ = std::max (0, std::min (48, predictIdx_ + adj));
#else
    int32_t v = updateTable_[x | (predictIdx_ << 4)];
    currentValue_ += v >> 16;
    predictIdx_ = v & 65535;
#endif

    return currentValue_;
}

int
M6258Coder::encodeSample(int v)
{
    int predict = getPredictVector(predictIdx_);
    int diff    = v - currentValue_;
    bool sign   = diff < 0;
    int d       = sign ? -diff : diff;

    int sp = d << 2;
    int p4 = predict << 2;
    int p2 = predict << 1;

    int x = sign ? 8 : 0;
    if (sp > p4)
    {
        x += 4;
        sp -= p4;
    }
    if (sp > p2)
    {
        x += 2;
        sp -= p2;
    }
    if (sp > predict)
    {
        x += 1;
    }

    update(x);
    return x;
}

} // namespace sound_sys
