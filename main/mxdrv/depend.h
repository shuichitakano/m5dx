// MXDRV.DLL X68000-depend header
// Copyright (C) 2000 GORRY.

#ifndef __DEPEND_H__
#define __DEPEND_H__

#include <stdint.h>

/*
typedef unsigned char UBYTE;
typedef unsigned short UWORD;
typedef unsigned long ULONG;
typedef signed char SBYTE;
typedef signed short SWORD;
typedef signed long SLONG;
*/

typedef uint8_t BOOL;
typedef uint8_t BYTE;
typedef uint32_t DWORD;
typedef uint64_t LONGLONG;

typedef uint8_t UBYTE;
typedef uint16_t UWORD;
typedef uint32_t ULONG;
typedef int8_t SBYTE;
typedef int16_t SWORD;
typedef int32_t SLONG;

typedef struct __X68REG {
    ULONG d0;
    ULONG d1;
    ULONG d2;
    ULONG d3;
    ULONG d4;
    ULONG d5;
    ULONG d6;
    ULONG d7;
    UBYTE *a0;
    UBYTE *a1;
    UBYTE *a2;
    UBYTE *a3;
    UBYTE *a4;
    UBYTE *a5;
    UBYTE *a6;
    UBYTE *a7;
} X68REG;

#endif //__DEPEND_H__
