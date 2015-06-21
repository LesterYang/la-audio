#ifndef _SAF7741_UTIL_H
#define _SAF7741_UTIL_H

#include <stdio.h>

// debug level

#define DbgNone             (0x0)
#define DbgErr              (0x1)
#define DbgWarn             (0x1 << 1)
#define DbgInfo             (0x1 << 2)
#define DbgStartUpAudio     (0x1 << 3)
#define DbgStartUpRadio     (0x1 << 4)
#define DbgAudio            (0x1 << 5)
#define DbgRadio            (0x1 << 6)

#define DbgLvErr            (DbgErr)
#define DbgLvWarn           (DbgErr | DbgWarn)
#define DbgLvInfo           (DbgErr | DbgWarn | DbgInfo)
#define DbgStartUp          (DbgStartUpAudio | DbgStartUpRadio)

#define DbgAll              (~0)

#define MemXBits            (12)
#define MemYBits            (24)

#define MemXMask            (0xFFF)
#define MemYMask            (0xFFFFFF)
#define MemYMaskMSB         (0xFFF000)     
#define MemYMaskLSB         (0x000FFF)

#define DivMilli            (1000)
#define DivMicro            (1000000)
#define DivNano             (1000000000)


#ifdef __GNUC__
#define SAF7741_LIKELY(x) (__builtin_expect(!!(x),1))
#define SAF7741_UNLIKELY(x) (__builtin_expect(!!(x),0))
#else
#define SAF7741_LIKELY(x) (x)
#define SAF7741_UNLIKELY(x) (x)
#endif

#define void_saf7741_check_func(func)           \
    do{                                         \
        if(SAF7741_UNLIKELY((func) == NULL))   \
            return;                             \
    }while(0)

#define int_saf7741_check_func(func)            \
    do{                                         \
        if(SAF7741_UNLIKELY((func) == NULL))   \
            return -1;                          \
    }while(0)


extern int gDbgFlag;

#define saf7741_log(flag, expr, ...)                \
    do{                                             \
        if(flag & gDbgFlag)                         \
        {                                           \
            fprintf(stderr, "saf7741 : ");          \
            fprintf(stderr, expr, ##__VA_ARGS__);   \
            fprintf(stderr, "\n");                  \
        }                                           \
    }while(0)

void saf7741_utilSetDbgLv(int lv);

int saf7741_getHexFromFrac(long long frac, int bits, int div, int mask, int shift);

inline int saf7741_getMemXHexFromMilliFrac(long long mFrac)
{
    return saf7741_getHexFromFrac(mFrac, MemXBits, DivMilli, MemXMask, 0);
}

inline int saf7741_getMemYHexFromMilliFrac(long long mFrac)
{
    return saf7741_getHexFromFrac(mFrac, MemYBits, DivMilli, MemYMask, 0);
}

inline int saf7741_getMemYHexMSBFromMilliFrac(long long mFrac)
{
    return saf7741_getHexFromFrac(mFrac, MemYBits, DivMilli, MemYMaskMSB, 12);
}

inline int saf7741_getMemYHexLSBFromMilliFrac(long long mFrac)
{
    return saf7741_getHexFromFrac(mFrac, MemYBits, DivMilli, MemYMaskLSB, 1);
}

inline int saf7741_getMemXHexFromMicroFrac(long long uFrac)
{
    return saf7741_getHexFromFrac(uFrac, MemXBits, DivMicro, MemXMask, 0);
}

inline int saf7741_getMemYHexFromMicroFrac(long long uFrac)
{
    return saf7741_getHexFromFrac(uFrac, MemYBits, DivMicro, MemYMask, 0);
}

inline int saf7741_getMemYHexMSBFromMicroFrac(long long uFrac)
{
    return saf7741_getHexFromFrac(uFrac, MemYBits, DivMicro, MemYMaskMSB, 12);
}

inline int saf7741_getMemYHexLSBFromMicroFrac(long long uFrac)
{
    return saf7741_getHexFromFrac(uFrac, MemYBits, DivMicro, MemYMaskLSB, 1);
}

inline int saf7741_getMemXHexFromNanoFrac(long long nFrac)
{
    return saf7741_getHexFromFrac(nFrac, MemXBits, DivNano, MemXMask, 0);
}

inline int saf7741_getMemYHexFromNanoFrac(long long nFrac)
{
    return saf7741_getHexFromFrac(nFrac, MemYBits, DivNano, MemYMask, 0);
}

inline int saf7741_getMemYHexMSBFromNanoFrac(long long nFrac)
{
    return saf7741_getHexFromFrac(nFrac, MemYBits, DivNano, MemYMaskMSB, 12);
}

inline int saf7741_getMemYHexLSBFromNanoFrac(long long nFrac)
{
    return saf7741_getHexFromFrac(nFrac, MemYBits, DivNano, MemYMaskLSB, 1);
}


#endif /* _SAF7741_UTIL_H */

