#ifndef SSE2NEON_H
#define SSE2NEON_H


#ifndef SSE2NEON_PRECISE_MINMAX
#define SSE2NEON_PRECISE_MINMAX (0)
#endif

#ifndef SSE2NEON_PRECISE_DIV
#define SSE2NEON_PRECISE_DIV (0)
#endif

#ifndef SSE2NEON_PRECISE_SQRT
#define SSE2NEON_PRECISE_SQRT (0)
#endif

#ifndef SSE2NEON_PRECISE_DP
#define SSE2NEON_PRECISE_DP (0)
#endif


#ifndef SSE2NEON_INCLUDE_WINDOWS_H
#define SSE2NEON_INCLUDE_WINDOWS_H (0)
#endif


#if defined(__GNUC__) || defined(__clang__)
#pragma push_macro("FORCE_INLINE")
#pragma push_macro("ALIGN_STRUCT")
#define FORCE_INLINE static inline __attribute__((always_inline))
#define ALIGN_STRUCT(x) __attribute__((aligned(x)))
#define _sse2neon_likely(x) __builtin_expect(!!(x), 1)
#define _sse2neon_unlikely(x) __builtin_expect(!!(x), 0)
#elif defined(_MSC_VER)
#if _MSVC_TRADITIONAL
#error Using the traditional MSVC preprocessor is not supported! Use /Zc:preprocessor instead.
#endif
#ifndef FORCE_INLINE
#define FORCE_INLINE static inline
#endif
#ifndef ALIGN_STRUCT
#define ALIGN_STRUCT(x) __declspec(align(x))
#endif
#define _sse2neon_likely(x) (x)
#define _sse2neon_unlikely(x) (x)
#else
#pragma message("Macro name collisions may happen with unsupported compilers.")
#endif

#if !defined(__clang__) && defined(__GNUC__) && __GNUC__ < 10
#warning "GCC versions earlier than 10 are not supported."
#endif

#if defined(__OPTIMIZE__) && !defined(SSE2NEON_SUPPRESS_WARNINGS)
#warning \
    "Report any potential compiler optimization issues when using SSE2NEON. See the 'Optimization' section at https://github.com/DLTcollab/sse2neon."
#endif


#ifdef __cplusplus
#define _sse2neon_const static const
#else
#define _sse2neon_const const
#endif

#include <fenv.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

FORCE_INLINE double sse2neon_recast_u64_f64(uint64_t val)
{
    double tmp;
    memcpy(&tmp, &val, sizeof(uint64_t));
    return tmp;
}
FORCE_INLINE int64_t sse2neon_recast_f64_s64(double val)
{
    int64_t tmp;
    memcpy(&tmp, &val, sizeof(uint64_t));
    return tmp;
}

#if defined(_WIN32) && !defined(__MINGW32__)

#define SSE2NEON_ALLOC_DEFINED
#endif


#ifdef _MSC_VER
#if defined(_M_ARM64EC)
#define _DISABLE_SOFTINTRIN_ 1
#endif
#include <intrin.h>
#if SSE2NEON_INCLUDE_WINDOWS_H
#include <processthreadsapi.h>
#include <windows.h>
#endif

#if !defined(__cplusplus)
#error SSE2NEON only supports C++ compilation with this compiler
#endif

#ifdef SSE2NEON_ALLOC_DEFINED
#include <malloc.h>
#endif

#if (defined(_M_AMD64) || defined(__x86_64__)) || \
    (defined(_M_ARM64) || defined(_M_ARM64EC) || defined(__arm64__))
#define SSE2NEON_HAS_BITSCAN64
#endif
#endif

#if defined(__GNUC__) || defined(__clang__)
#define _sse2neon_define0(type, s, body) \
    __extension__({                      \
        type _a = (s);                   \
        body                             \
    })
#define _sse2neon_define1(type, s, body) \
    __extension__({                      \
        type _a = (s);                   \
        body                             \
    })
#define _sse2neon_define2(type, a, b, body) \
    __extension__({                         \
        type _a = (a), _b = (b);            \
        body                                \
    })
#define _sse2neon_return(ret) (ret)
#else
#define _sse2neon_define0(type, a, body) [=](type _a) { body }(a)
#define _sse2neon_define1(type, a, body) [](type _a) { body }(a)
#define _sse2neon_define2(type, a, b, body) \
    [](type _a, type _b) { body }((a), (b))
#define _sse2neon_return(ret) return ret
#endif

#define _sse2neon_init(...) \
    {                       \
        __VA_ARGS__         \
    }


#if defined(_MSC_VER) && !defined(__clang__)
#define SSE2NEON_BARRIER() _ReadWriteBarrier()
#else
#define SSE2NEON_BARRIER()                     \
    do {                                       \
        __asm__ __volatile__("" ::: "memory"); \
        (void) 0;                              \
    } while (0)
#endif


#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#include <stdatomic.h>
#endif

FORCE_INLINE void _sse2neon_smp_mb(void)
{
    SSE2NEON_BARRIER();
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) && \
    !defined(__STDC_NO_ATOMICS__)
    atomic_thread_fence(memory_order_seq_cst);
#elif defined(__GNUC__) || defined(__clang__)
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
#else
    __dmb(_ARM64_BARRIER_ISH);
#endif
}


#if defined(__GNUC__)
#if defined(__arm__) && __ARM_ARCH == 7


#if !defined(__ARM_NEON) || !defined(__ARM_NEON__)
#error "You must enable NEON instructions (e.g. -mfpu=neon) to use SSE2NEON."
#endif
#if !defined(__clang__)
#pragma GCC push_options
#pragma GCC target("fpu=neon")
#endif
#elif defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
#if !defined(__clang__) && !defined(_MSC_VER)
#pragma GCC push_options
#pragma GCC target("+simd")
#endif
#elif __ARM_ARCH == 8
#if !defined(__ARM_NEON) || !defined(__ARM_NEON__)
#error \
    "You must enable NEON instructions (e.g. -mfpu=neon-fp-armv8) to use SSE2NEON."
#endif
#if !defined(__clang__) && !defined(_MSC_VER)
#pragma GCC push_options
#endif
#else
#error \
    "Unsupported target. Must be either ARMv7-A+NEON or ARMv8-A \
(you could try setting target explicitly with -march or -mcpu)"
#endif
#endif

#include <arm_neon.h>
#if (!defined(__aarch64__) && !defined(_M_ARM64) && !defined(_M_ARM64EC)) && \
    (__ARM_ARCH == 8)
#if defined __has_include && __has_include(<arm_acle.h>)
#include <arm_acle.h>
#endif
#endif


#if defined(__APPLE__) && (defined(__aarch64__) || defined(__arm64__))
#define SSE2NEON_CACHELINE_SIZE 128
#else
#define SSE2NEON_CACHELINE_SIZE 64
#endif


#if !defined(__aarch64__) && !defined(_M_ARM64) && !defined(_M_ARM64EC)
#include <math.h>
#endif


#if !defined(__aarch64__) && !defined(_M_ARM64) && !defined(_M_ARM64EC)
#include <sys/time.h>
#endif


#ifndef __has_builtin

#if defined(__GNUC__) && (__GNUC__ <= 9)
#define __has_builtin(x) HAS##x
#define HAS__builtin_popcount 1
#define HAS__builtin_popcountll 1


#if (__GNUC__ >= 5) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 7))
#define HAS__builtin_shuffle 1
#else
#define HAS__builtin_shuffle 0
#endif

#define HAS__builtin_shufflevector 0
#define HAS__builtin_nontemporal_store 0
#else
#define __has_builtin(x) 0
#endif
#endif


#define _MM_SHUFFLE(fp3, fp2, fp1, fp0) \
    (((fp3) << 6) | ((fp2) << 4) | ((fp1) << 2) | ((fp0)))


#define _MM_SHUFFLE2(fp1, fp0) (((fp1) << 1) | (fp0))

#if __has_builtin(__builtin_shufflevector)
#define _sse2neon_shuffle(type, a, b, ...) \
    __builtin_shufflevector(a, b, __VA_ARGS__)
#elif __has_builtin(__builtin_shuffle)
#define _sse2neon_shuffle(type, a, b, ...) \
    __extension__({                        \
        type tmp = {__VA_ARGS__};          \
        __builtin_shuffle(a, b, tmp);      \
    })
#endif

#ifdef _sse2neon_shuffle
#define vshuffle_s16(a, b, ...) _sse2neon_shuffle(int16x4_t, a, b, __VA_ARGS__)
#define vshuffleq_s16(a, b, ...) _sse2neon_shuffle(int16x8_t, a, b, __VA_ARGS__)
#define vshuffle_s32(a, b, ...) _sse2neon_shuffle(int32x2_t, a, b, __VA_ARGS__)
#define vshuffleq_s32(a, b, ...) _sse2neon_shuffle(int32x4_t, a, b, __VA_ARGS__)
#define vshuffle_s64(a, b, ...) _sse2neon_shuffle(int64x1_t, a, b, __VA_ARGS__)
#define vshuffleq_s64(a, b, ...) _sse2neon_shuffle(int64x2_t, a, b, __VA_ARGS__)
#endif


#define _MM_FROUND_TO_NEAREST_INT 0x00
#define _MM_FROUND_TO_NEG_INF 0x01
#define _MM_FROUND_TO_POS_INF 0x02
#define _MM_FROUND_TO_ZERO 0x03
#define _MM_FROUND_CUR_DIRECTION 0x04
#define _MM_FROUND_NO_EXC 0x08
#define _MM_FROUND_RAISE_EXC 0x00
#define _MM_FROUND_NINT (_MM_FROUND_TO_NEAREST_INT | _MM_FROUND_RAISE_EXC)
#define _MM_FROUND_FLOOR (_MM_FROUND_TO_NEG_INF | _MM_FROUND_RAISE_EXC)
#define _MM_FROUND_CEIL (_MM_FROUND_TO_POS_INF | _MM_FROUND_RAISE_EXC)
#define _MM_FROUND_TRUNC (_MM_FROUND_TO_ZERO | _MM_FROUND_RAISE_EXC)
#define _MM_FROUND_RINT (_MM_FROUND_CUR_DIRECTION | _MM_FROUND_RAISE_EXC)
#define _MM_FROUND_NEARBYINT (_MM_FROUND_CUR_DIRECTION | _MM_FROUND_NO_EXC)
#define _MM_ROUND_NEAREST 0x0000
#define _MM_ROUND_DOWN 0x2000
#define _MM_ROUND_UP 0x4000
#define _MM_ROUND_TOWARD_ZERO 0x6000

#define _MM_FLUSH_ZERO_MASK 0x8000
#define _MM_FLUSH_ZERO_ON 0x8000
#define _MM_FLUSH_ZERO_OFF 0x0000

#define _MM_DENORMALS_ZERO_MASK 0x0040
#define _MM_DENORMALS_ZERO_ON 0x0040
#define _MM_DENORMALS_ZERO_OFF 0x0000


#define __constrange(a, b) const


typedef int64x1_t __m64;
typedef float32x4_t __m128;


#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
typedef float64x2_t __m128d;
#else
typedef float32x4_t __m128d;
#endif
typedef int64x2_t __m128i;


typedef int16_t ALIGN_STRUCT(1) unaligned_int16_t;
typedef int32_t ALIGN_STRUCT(1) unaligned_int32_t;
typedef int64_t ALIGN_STRUCT(1) unaligned_int64_t;


#if !(defined(_WIN32) || defined(_WIN64) || defined(__int64))
#if (defined(__x86_64__) || defined(__i386__))
#define __int64 long long
#else
#define __int64 int64_t
#endif
#endif


#define vreinterpretq_m128_f16(x) vreinterpretq_f32_f16(x)
#define vreinterpretq_m128_f32(x) (x)
#define vreinterpretq_m128_f64(x) vreinterpretq_f32_f64(x)

#define vreinterpretq_m128_u8(x) vreinterpretq_f32_u8(x)
#define vreinterpretq_m128_u16(x) vreinterpretq_f32_u16(x)
#define vreinterpretq_m128_u32(x) vreinterpretq_f32_u32(x)
#define vreinterpretq_m128_u64(x) vreinterpretq_f32_u64(x)

#define vreinterpretq_m128_s8(x) vreinterpretq_f32_s8(x)
#define vreinterpretq_m128_s16(x) vreinterpretq_f32_s16(x)
#define vreinterpretq_m128_s32(x) vreinterpretq_f32_s32(x)
#define vreinterpretq_m128_s64(x) vreinterpretq_f32_s64(x)

#define vreinterpretq_f16_m128(x) vreinterpretq_f16_f32(x)
#define vreinterpretq_f32_m128(x) (x)
#define vreinterpretq_f64_m128(x) vreinterpretq_f64_f32(x)

#define vreinterpretq_u8_m128(x) vreinterpretq_u8_f32(x)
#define vreinterpretq_u16_m128(x) vreinterpretq_u16_f32(x)
#define vreinterpretq_u32_m128(x) vreinterpretq_u32_f32(x)
#define vreinterpretq_u64_m128(x) vreinterpretq_u64_f32(x)

#define vreinterpretq_s8_m128(x) vreinterpretq_s8_f32(x)
#define vreinterpretq_s16_m128(x) vreinterpretq_s16_f32(x)
#define vreinterpretq_s32_m128(x) vreinterpretq_s32_f32(x)
#define vreinterpretq_s64_m128(x) vreinterpretq_s64_f32(x)

#define vreinterpretq_m128i_s8(x) vreinterpretq_s64_s8(x)
#define vreinterpretq_m128i_s16(x) vreinterpretq_s64_s16(x)
#define vreinterpretq_m128i_s32(x) vreinterpretq_s64_s32(x)
#define vreinterpretq_m128i_s64(x) (x)

#define vreinterpretq_m128i_u8(x) vreinterpretq_s64_u8(x)
#define vreinterpretq_m128i_u16(x) vreinterpretq_s64_u16(x)
#define vreinterpretq_m128i_u32(x) vreinterpretq_s64_u32(x)
#define vreinterpretq_m128i_u64(x) vreinterpretq_s64_u64(x)

#define vreinterpretq_f32_m128i(x) vreinterpretq_f32_s64(x)
#define vreinterpretq_f64_m128i(x) vreinterpretq_f64_s64(x)

#define vreinterpretq_s8_m128i(x) vreinterpretq_s8_s64(x)
#define vreinterpretq_s16_m128i(x) vreinterpretq_s16_s64(x)
#define vreinterpretq_s32_m128i(x) vreinterpretq_s32_s64(x)
#define vreinterpretq_s64_m128i(x) (x)

#define vreinterpretq_u8_m128i(x) vreinterpretq_u8_s64(x)
#define vreinterpretq_u16_m128i(x) vreinterpretq_u16_s64(x)
#define vreinterpretq_u32_m128i(x) vreinterpretq_u32_s64(x)
#define vreinterpretq_u64_m128i(x) vreinterpretq_u64_s64(x)

#define vreinterpret_m64_s8(x) vreinterpret_s64_s8(x)
#define vreinterpret_m64_s16(x) vreinterpret_s64_s16(x)
#define vreinterpret_m64_s32(x) vreinterpret_s64_s32(x)
#define vreinterpret_m64_s64(x) (x)

#define vreinterpret_m64_u8(x) vreinterpret_s64_u8(x)
#define vreinterpret_m64_u16(x) vreinterpret_s64_u16(x)
#define vreinterpret_m64_u32(x) vreinterpret_s64_u32(x)
#define vreinterpret_m64_u64(x) vreinterpret_s64_u64(x)

#define vreinterpret_m64_f16(x) vreinterpret_s64_f16(x)
#define vreinterpret_m64_f32(x) vreinterpret_s64_f32(x)
#define vreinterpret_m64_f64(x) vreinterpret_s64_f64(x)

#define vreinterpret_u8_m64(x) vreinterpret_u8_s64(x)
#define vreinterpret_u16_m64(x) vreinterpret_u16_s64(x)
#define vreinterpret_u32_m64(x) vreinterpret_u32_s64(x)
#define vreinterpret_u64_m64(x) vreinterpret_u64_s64(x)

#define vreinterpret_s8_m64(x) vreinterpret_s8_s64(x)
#define vreinterpret_s16_m64(x) vreinterpret_s16_s64(x)
#define vreinterpret_s32_m64(x) vreinterpret_s32_s64(x)
#define vreinterpret_s64_m64(x) (x)

#define vreinterpret_f32_m64(x) vreinterpret_f32_s64(x)

#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
#define vreinterpretq_m128d_s32(x) vreinterpretq_f64_s32(x)
#define vreinterpretq_m128d_s64(x) vreinterpretq_f64_s64(x)

#define vreinterpretq_m128d_u64(x) vreinterpretq_f64_u64(x)

#define vreinterpretq_m128d_f32(x) vreinterpretq_f64_f32(x)
#define vreinterpretq_m128d_f64(x) (x)

#define vreinterpretq_s64_m128d(x) vreinterpretq_s64_f64(x)

#define vreinterpretq_u32_m128d(x) vreinterpretq_u32_f64(x)
#define vreinterpretq_u64_m128d(x) vreinterpretq_u64_f64(x)

#define vreinterpretq_f64_m128d(x) (x)
#define vreinterpretq_f32_m128d(x) vreinterpretq_f32_f64(x)
#else
#define vreinterpretq_m128d_s32(x) vreinterpretq_f32_s32(x)
#define vreinterpretq_m128d_s64(x) vreinterpretq_f32_s64(x)

#define vreinterpretq_m128d_u32(x) vreinterpretq_f32_u32(x)
#define vreinterpretq_m128d_u64(x) vreinterpretq_f32_u64(x)

#define vreinterpretq_m128d_f32(x) (x)

#define vreinterpretq_s64_m128d(x) vreinterpretq_s64_f32(x)

#define vreinterpretq_u32_m128d(x) vreinterpretq_u32_f32(x)
#define vreinterpretq_u64_m128d(x) vreinterpretq_u64_f32(x)

#define vreinterpretq_f32_m128d(x) (x)
#endif


typedef union ALIGN_STRUCT(16) SIMDVec {
    float m128_f32[4];
    int8_t m128_i8[16];
    int16_t m128_i16[8];
    int32_t m128_i32[4];
    int64_t m128_i64[2];
    uint8_t m128_u8[16];
    uint16_t m128_u16[8];
    uint32_t m128_u32[4];
    uint64_t m128_u64[2];
} SIMDVec;


#define vreinterpretq_nth_u64_m128i(x, n) (((SIMDVec *) &x)->m128_u64[n])
#define vreinterpretq_nth_u32_m128i(x, n) (((SIMDVec *) &x)->m128_u32[n])
#define vreinterpretq_nth_u8_m128i(x, n) (((SIMDVec *) &x)->m128_u8[n])


#define _MM_GET_FLUSH_ZERO_MODE _sse2neon_mm_get_flush_zero_mode
#define _MM_SET_FLUSH_ZERO_MODE _sse2neon_mm_set_flush_zero_mode
#define _MM_GET_DENORMALS_ZERO_MODE _sse2neon_mm_get_denormals_zero_mode
#define _MM_SET_DENORMALS_ZERO_MODE _sse2neon_mm_set_denormals_zero_mode


FORCE_INLINE unsigned int _MM_GET_ROUNDING_MODE(void);
FORCE_INLINE __m128 _mm_move_ss(__m128, __m128);
FORCE_INLINE __m128 _mm_or_ps(__m128, __m128);
FORCE_INLINE __m128 _mm_set_ps1(float);
FORCE_INLINE __m128 _mm_setzero_ps(void);

FORCE_INLINE __m128i _mm_and_si128(__m128i, __m128i);
FORCE_INLINE __m128i _mm_castps_si128(__m128);
FORCE_INLINE __m128i _mm_cmpeq_epi32(__m128i, __m128i);
FORCE_INLINE __m128i _mm_cvtps_epi32(__m128);
FORCE_INLINE __m128d _mm_move_sd(__m128d, __m128d);
FORCE_INLINE __m128i _mm_or_si128(__m128i, __m128i);
FORCE_INLINE __m128i _mm_set_epi32(int, int, int, int);
FORCE_INLINE __m128i _mm_set_epi64x(int64_t, int64_t);
FORCE_INLINE __m128d _mm_set_pd(double, double);
FORCE_INLINE __m128i _mm_set1_epi32(int);
FORCE_INLINE __m128i _mm_setzero_si128(void);

FORCE_INLINE __m128d _mm_ceil_pd(__m128d);
FORCE_INLINE __m128 _mm_ceil_ps(__m128);
FORCE_INLINE __m128d _mm_floor_pd(__m128d);
FORCE_INLINE __m128 _mm_floor_ps(__m128);
FORCE_INLINE __m128d _mm_round_pd(__m128d, int);
FORCE_INLINE __m128 _mm_round_ps(__m128, int);

FORCE_INLINE uint32_t _mm_crc32_u8(uint32_t, uint8_t);


#if defined(__GNUC__) && !defined(__clang__) &&                        \
    ((__GNUC__ <= 13 && defined(__arm__)) ||                           \
     (__GNUC__ == 10 && __GNUC_MINOR__ < 3 && defined(__aarch64__)) || \
     (__GNUC__ <= 9 && defined(__aarch64__)))
FORCE_INLINE uint8x16x4_t _sse2neon_vld1q_u8_x4(const uint8_t *p)
{
    uint8x16x4_t ret;
    ret.val[0] = vld1q_u8(p + 0);
    ret.val[1] = vld1q_u8(p + 16);
    ret.val[2] = vld1q_u8(p + 32);
    ret.val[3] = vld1q_u8(p + 48);
    return ret;
}
#else

FORCE_INLINE uint8x16x4_t _sse2neon_vld1q_u8_x4(const uint8_t *p)
{
    return vld1q_u8_x4(p);
}
#endif

#if !defined(__aarch64__) && !defined(_M_ARM64) && !defined(_M_ARM64EC)

FORCE_INLINE uint8_t _sse2neon_vaddv_u8(uint8x8_t v8)
{
    const uint64x1_t v1 = vpaddl_u32(vpaddl_u16(vpaddl_u8(v8)));
    return vget_lane_u8(vreinterpret_u8_u64(v1), 0);
}
#else

FORCE_INLINE uint8_t _sse2neon_vaddv_u8(uint8x8_t v8)
{
    return vaddv_u8(v8);
}
#endif

#if !defined(__aarch64__) && !defined(_M_ARM64) && !defined(_M_ARM64EC)

FORCE_INLINE uint8_t _sse2neon_vaddvq_u8(uint8x16_t a)
{
    uint8x8_t tmp = vpadd_u8(vget_low_u8(a), vget_high_u8(a));
    uint8_t res = 0;
    for (int i = 0; i < 8; ++i)
        res += tmp[i];
    return res;
}
#else

FORCE_INLINE uint8_t _sse2neon_vaddvq_u8(uint8x16_t a)
{
    return vaddvq_u8(a);
}
#endif

#if !defined(__aarch64__) && !defined(_M_ARM64) && !defined(_M_ARM64EC)

FORCE_INLINE uint16_t _sse2neon_vaddvq_u16(uint16x8_t a)
{
    uint32x4_t m = vpaddlq_u16(a);
    uint64x2_t n = vpaddlq_u32(m);
    uint64x1_t o = vget_low_u64(n) + vget_high_u64(n);

    return vget_lane_u32((uint32x2_t) o, 0);
}
#else

FORCE_INLINE uint16_t _sse2neon_vaddvq_u16(uint16x8_t a)
{
    return vaddvq_u16(a);
}
#endif


#if defined(_M_ARM64EC)

#undef _MM_HINT_NTA
#undef _MM_HINT_T0
#undef _MM_HINT_T1
#undef _MM_HINT_T2
#endif
enum _mm_hint {
    _MM_HINT_NTA = 0,
    _MM_HINT_T0 = 1,
    _MM_HINT_T1 = 2,
    _MM_HINT_T2 = 3,
};


typedef struct {
    uint16_t res0;
    uint8_t res1 : 6;
    uint8_t bit22 : 1;
    uint8_t bit23 : 1;
    uint8_t bit24 : 1;
    uint8_t res2 : 7;
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    uint32_t res3;
#endif
} fpcr_bitfield;


FORCE_INLINE __m128 _mm_shuffle_ps_1032(__m128 a, __m128 b)
{
    float32x2_t a32 = vget_high_f32(vreinterpretq_f32_m128(a));
    float32x2_t b10 = vget_low_f32(vreinterpretq_f32_m128(b));
    return vreinterpretq_m128_f32(vcombine_f32(a32, b10));
}


FORCE_INLINE __m128 _mm_shuffle_ps_2301(__m128 a, __m128 b)
{
    float32x2_t a01 = vrev64_f32(vget_low_f32(vreinterpretq_f32_m128(a)));
    float32x2_t b23 = vrev64_f32(vget_high_f32(vreinterpretq_f32_m128(b)));
    return vreinterpretq_m128_f32(vcombine_f32(a01, b23));
}

FORCE_INLINE __m128 _mm_shuffle_ps_0321(__m128 a, __m128 b)
{
    float32x2_t a21 = vget_high_f32(
        vextq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(a), 3));
    float32x2_t b03 = vget_low_f32(
        vextq_f32(vreinterpretq_f32_m128(b), vreinterpretq_f32_m128(b), 3));
    return vreinterpretq_m128_f32(vcombine_f32(a21, b03));
}

FORCE_INLINE __m128 _mm_shuffle_ps_2103(__m128 a, __m128 b)
{
    float32x2_t a03 = vget_low_f32(
        vextq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(a), 3));
    float32x2_t b21 = vget_high_f32(
        vextq_f32(vreinterpretq_f32_m128(b), vreinterpretq_f32_m128(b), 3));
    return vreinterpretq_m128_f32(vcombine_f32(a03, b21));
}

FORCE_INLINE __m128 _mm_shuffle_ps_1010(__m128 a, __m128 b)
{
    float32x2_t a10 = vget_low_f32(vreinterpretq_f32_m128(a));
    float32x2_t b10 = vget_low_f32(vreinterpretq_f32_m128(b));
    return vreinterpretq_m128_f32(vcombine_f32(a10, b10));
}

FORCE_INLINE __m128 _mm_shuffle_ps_1001(__m128 a, __m128 b)
{
    float32x2_t a01 = vrev64_f32(vget_low_f32(vreinterpretq_f32_m128(a)));
    float32x2_t b10 = vget_low_f32(vreinterpretq_f32_m128(b));
    return vreinterpretq_m128_f32(vcombine_f32(a01, b10));
}

FORCE_INLINE __m128 _mm_shuffle_ps_0101(__m128 a, __m128 b)
{
    float32x2_t a01 = vrev64_f32(vget_low_f32(vreinterpretq_f32_m128(a)));
    float32x2_t b01 = vrev64_f32(vget_low_f32(vreinterpretq_f32_m128(b)));
    return vreinterpretq_m128_f32(vcombine_f32(a01, b01));
}


FORCE_INLINE __m128 _mm_shuffle_ps_3210(__m128 a, __m128 b)
{
    float32x2_t a10 = vget_low_f32(vreinterpretq_f32_m128(a));
    float32x2_t b32 = vget_high_f32(vreinterpretq_f32_m128(b));
    return vreinterpretq_m128_f32(vcombine_f32(a10, b32));
}

FORCE_INLINE __m128 _mm_shuffle_ps_0011(__m128 a, __m128 b)
{
    float32x2_t a11 = vdup_lane_f32(vget_low_f32(vreinterpretq_f32_m128(a)), 1);
    float32x2_t b00 = vdup_lane_f32(vget_low_f32(vreinterpretq_f32_m128(b)), 0);
    return vreinterpretq_m128_f32(vcombine_f32(a11, b00));
}

FORCE_INLINE __m128 _mm_shuffle_ps_0022(__m128 a, __m128 b)
{
    float32x2_t a22 =
        vdup_lane_f32(vget_high_f32(vreinterpretq_f32_m128(a)), 0);
    float32x2_t b00 = vdup_lane_f32(vget_low_f32(vreinterpretq_f32_m128(b)), 0);
    return vreinterpretq_m128_f32(vcombine_f32(a22, b00));
}

FORCE_INLINE __m128 _mm_shuffle_ps_2200(__m128 a, __m128 b)
{
    float32x2_t a00 = vdup_lane_f32(vget_low_f32(vreinterpretq_f32_m128(a)), 0);
    float32x2_t b22 =
        vdup_lane_f32(vget_high_f32(vreinterpretq_f32_m128(b)), 0);
    return vreinterpretq_m128_f32(vcombine_f32(a00, b22));
}

FORCE_INLINE __m128 _mm_shuffle_ps_3202(__m128 a, __m128 b)
{
    float32_t a0 = vgetq_lane_f32(vreinterpretq_f32_m128(a), 0);
    float32x2_t a22 =
        vdup_lane_f32(vget_high_f32(vreinterpretq_f32_m128(a)), 0);
    float32x2_t a02 = vset_lane_f32(a0, a22, 1);
    float32x2_t b32 = vget_high_f32(vreinterpretq_f32_m128(b));
    return vreinterpretq_m128_f32(vcombine_f32(a02, b32));
}

FORCE_INLINE __m128 _mm_shuffle_ps_1133(__m128 a, __m128 b)
{
    float32x2_t a33 =
        vdup_lane_f32(vget_high_f32(vreinterpretq_f32_m128(a)), 1);
    float32x2_t b11 = vdup_lane_f32(vget_low_f32(vreinterpretq_f32_m128(b)), 1);
    return vreinterpretq_m128_f32(vcombine_f32(a33, b11));
}

FORCE_INLINE __m128 _mm_shuffle_ps_2010(__m128 a, __m128 b)
{
    float32x2_t a10 = vget_low_f32(vreinterpretq_f32_m128(a));
    float32_t b2 = vgetq_lane_f32(vreinterpretq_f32_m128(b), 2);
    float32x2_t b00 = vdup_lane_f32(vget_low_f32(vreinterpretq_f32_m128(b)), 0);
    float32x2_t b20 = vset_lane_f32(b2, b00, 1);
    return vreinterpretq_m128_f32(vcombine_f32(a10, b20));
}

FORCE_INLINE __m128 _mm_shuffle_ps_2001(__m128 a, __m128 b)
{
    float32x2_t a01 = vrev64_f32(vget_low_f32(vreinterpretq_f32_m128(a)));
    float32_t b2 = vgetq_lane_f32(b, 2);
    float32x2_t b00 = vdup_lane_f32(vget_low_f32(vreinterpretq_f32_m128(b)), 0);
    float32x2_t b20 = vset_lane_f32(b2, b00, 1);
    return vreinterpretq_m128_f32(vcombine_f32(a01, b20));
}

FORCE_INLINE __m128 _mm_shuffle_ps_2032(__m128 a, __m128 b)
{
    float32x2_t a32 = vget_high_f32(vreinterpretq_f32_m128(a));
    float32_t b2 = vgetq_lane_f32(b, 2);
    float32x2_t b00 = vdup_lane_f32(vget_low_f32(vreinterpretq_f32_m128(b)), 0);
    float32x2_t b20 = vset_lane_f32(b2, b00, 1);
    return vreinterpretq_m128_f32(vcombine_f32(a32, b20));
}


#if ((defined(_M_ARM64) || defined(_M_ARM64EC)) && !defined(__clang__)) || \
    (defined(__ARM_FEATURE_CRYPTO) &&                                      \
     (defined(__aarch64__) || __has_builtin(__builtin_arm_crypto_vmullp64)))

FORCE_INLINE uint64x2_t _sse2neon_vmull_p64(uint64x1_t _a, uint64x1_t _b)
{
    poly64_t a = vget_lane_p64(vreinterpret_p64_u64(_a), 0);
    poly64_t b = vget_lane_p64(vreinterpret_p64_u64(_b), 0);
#if defined(_MSC_VER) && !defined(__clang__)
    __n64 a1 = {a}, b1 = {b};
    return vreinterpretq_u64_p128(vmull_p64(a1, b1));
#else
    return vreinterpretq_u64_p128(vmull_p64(a, b));
#endif
}
#else


static uint64x2_t _sse2neon_vmull_p64(uint64x1_t _a, uint64x1_t _b)
{
    poly8x8_t a = vreinterpret_p8_u64(_a);
    poly8x8_t b = vreinterpret_p8_u64(_b);


    uint8x16_t k48_32 = vcombine_u8(vcreate_u8(0x0000ffffffffffff),
                                    vcreate_u8(0x00000000ffffffff));
    uint8x16_t k16_00 = vcombine_u8(vcreate_u8(0x000000000000ffff),
                                    vcreate_u8(0x0000000000000000));


    uint8x16_t d = vreinterpretq_u8_p16(vmull_p8(a, b));
    uint8x16_t e =
        vreinterpretq_u8_p16(vmull_p8(a, vext_p8(b, b, 1)));
    uint8x16_t f =
        vreinterpretq_u8_p16(vmull_p8(vext_p8(a, a, 1), b));
    uint8x16_t g =
        vreinterpretq_u8_p16(vmull_p8(a, vext_p8(b, b, 2)));
    uint8x16_t h =
        vreinterpretq_u8_p16(vmull_p8(vext_p8(a, a, 2), b));
    uint8x16_t i =
        vreinterpretq_u8_p16(vmull_p8(a, vext_p8(b, b, 3)));
    uint8x16_t j =
        vreinterpretq_u8_p16(vmull_p8(vext_p8(a, a, 3), b));
    uint8x16_t k =
        vreinterpretq_u8_p16(vmull_p8(a, vext_p8(b, b, 4)));


    uint8x16_t l = veorq_u8(e, f);
    uint8x16_t m = veorq_u8(g, h);
    uint8x16_t n = veorq_u8(i, j);


#if defined(__aarch64__)
    uint8x16_t lm_p0 = vreinterpretq_u8_u64(
        vzip1q_u64(vreinterpretq_u64_u8(l), vreinterpretq_u64_u8(m)));
    uint8x16_t lm_p1 = vreinterpretq_u8_u64(
        vzip2q_u64(vreinterpretq_u64_u8(l), vreinterpretq_u64_u8(m)));
    uint8x16_t nk_p0 = vreinterpretq_u8_u64(
        vzip1q_u64(vreinterpretq_u64_u8(n), vreinterpretq_u64_u8(k)));
    uint8x16_t nk_p1 = vreinterpretq_u8_u64(
        vzip2q_u64(vreinterpretq_u64_u8(n), vreinterpretq_u64_u8(k)));
#else
    uint8x16_t lm_p0 = vcombine_u8(vget_low_u8(l), vget_low_u8(m));
    uint8x16_t lm_p1 = vcombine_u8(vget_high_u8(l), vget_high_u8(m));
    uint8x16_t nk_p0 = vcombine_u8(vget_low_u8(n), vget_low_u8(k));
    uint8x16_t nk_p1 = vcombine_u8(vget_high_u8(n), vget_high_u8(k));
#endif


    uint8x16_t t0t1_tmp = veorq_u8(lm_p0, lm_p1);
    uint8x16_t t0t1_h = vandq_u8(lm_p1, k48_32);
    uint8x16_t t0t1_l = veorq_u8(t0t1_tmp, t0t1_h);


    uint8x16_t t2t3_tmp = veorq_u8(nk_p0, nk_p1);
    uint8x16_t t2t3_h = vandq_u8(nk_p1, k16_00);
    uint8x16_t t2t3_l = veorq_u8(t2t3_tmp, t2t3_h);


#if defined(__aarch64__)
    uint8x16_t t0 = vreinterpretq_u8_u64(
        vuzp1q_u64(vreinterpretq_u64_u8(t0t1_l), vreinterpretq_u64_u8(t0t1_h)));
    uint8x16_t t1 = vreinterpretq_u8_u64(
        vuzp2q_u64(vreinterpretq_u64_u8(t0t1_l), vreinterpretq_u64_u8(t0t1_h)));
    uint8x16_t t2 = vreinterpretq_u8_u64(
        vuzp1q_u64(vreinterpretq_u64_u8(t2t3_l), vreinterpretq_u64_u8(t2t3_h)));
    uint8x16_t t3 = vreinterpretq_u8_u64(
        vuzp2q_u64(vreinterpretq_u64_u8(t2t3_l), vreinterpretq_u64_u8(t2t3_h)));
#else
    uint8x16_t t1 = vcombine_u8(vget_high_u8(t0t1_l), vget_high_u8(t0t1_h));
    uint8x16_t t0 = vcombine_u8(vget_low_u8(t0t1_l), vget_low_u8(t0t1_h));
    uint8x16_t t3 = vcombine_u8(vget_high_u8(t2t3_l), vget_high_u8(t2t3_h));
    uint8x16_t t2 = vcombine_u8(vget_low_u8(t2t3_l), vget_low_u8(t2t3_h));
#endif

    uint8x16_t t0_shift = vextq_u8(t0, t0, 15);
    uint8x16_t t1_shift = vextq_u8(t1, t1, 14);
    uint8x16_t t2_shift = vextq_u8(t2, t2, 13);
    uint8x16_t t3_shift = vextq_u8(t3, t3, 12);


    uint8x16_t cross1 = veorq_u8(t0_shift, t1_shift);
    uint8x16_t cross2 = veorq_u8(t2_shift, t3_shift);
    uint8x16_t mix = veorq_u8(d, cross1);
    uint8x16_t r = veorq_u8(mix, cross2);
    return vreinterpretq_u64_u8(r);
}
#endif


#define _mm_shuffle_epi32_default(a, imm)                                   \
    vreinterpretq_m128i_s32(vsetq_lane_s32(                                 \
        vgetq_lane_s32(vreinterpretq_s32_m128i(a), ((imm) >> 6) & 0x3),     \
        vsetq_lane_s32(                                                     \
            vgetq_lane_s32(vreinterpretq_s32_m128i(a), ((imm) >> 4) & 0x3), \
            vsetq_lane_s32(vgetq_lane_s32(vreinterpretq_s32_m128i(a),       \
                                          ((imm) >> 2) & 0x3),              \
                           vmovq_n_s32(vgetq_lane_s32(                      \
                               vreinterpretq_s32_m128i(a), (imm) & (0x3))), \
                           1),                                              \
            2),                                                             \
        3))


FORCE_INLINE __m128i _mm_shuffle_epi_1032(__m128i a)
{
    int32x2_t a32 = vget_high_s32(vreinterpretq_s32_m128i(a));
    int32x2_t a10 = vget_low_s32(vreinterpretq_s32_m128i(a));
    return vreinterpretq_m128i_s32(vcombine_s32(a32, a10));
}


FORCE_INLINE __m128i _mm_shuffle_epi_2301(__m128i a)
{
    int32x2_t a01 = vrev64_s32(vget_low_s32(vreinterpretq_s32_m128i(a)));
    int32x2_t a23 = vrev64_s32(vget_high_s32(vreinterpretq_s32_m128i(a)));
    return vreinterpretq_m128i_s32(vcombine_s32(a01, a23));
}


FORCE_INLINE __m128i _mm_shuffle_epi_0321(__m128i a)
{
    return vreinterpretq_m128i_s32(
        vextq_s32(vreinterpretq_s32_m128i(a), vreinterpretq_s32_m128i(a), 1));
}


FORCE_INLINE __m128i _mm_shuffle_epi_2103(__m128i a)
{
    return vreinterpretq_m128i_s32(
        vextq_s32(vreinterpretq_s32_m128i(a), vreinterpretq_s32_m128i(a), 3));
}


FORCE_INLINE __m128i _mm_shuffle_epi_1010(__m128i a)
{
    int32x2_t a10 = vget_low_s32(vreinterpretq_s32_m128i(a));
    return vreinterpretq_m128i_s32(vcombine_s32(a10, a10));
}


FORCE_INLINE __m128i _mm_shuffle_epi_1001(__m128i a)
{
    int32x2_t a01 = vrev64_s32(vget_low_s32(vreinterpretq_s32_m128i(a)));
    int32x2_t a10 = vget_low_s32(vreinterpretq_s32_m128i(a));
    return vreinterpretq_m128i_s32(vcombine_s32(a01, a10));
}


FORCE_INLINE __m128i _mm_shuffle_epi_0101(__m128i a)
{
    int32x2_t a01 = vrev64_s32(vget_low_s32(vreinterpretq_s32_m128i(a)));
    return vreinterpretq_m128i_s32(vcombine_s32(a01, a01));
}

FORCE_INLINE __m128i _mm_shuffle_epi_2211(__m128i a)
{
    int32x2_t a11 = vdup_lane_s32(vget_low_s32(vreinterpretq_s32_m128i(a)), 1);
    int32x2_t a22 = vdup_lane_s32(vget_high_s32(vreinterpretq_s32_m128i(a)), 0);
    return vreinterpretq_m128i_s32(vcombine_s32(a11, a22));
}

FORCE_INLINE __m128i _mm_shuffle_epi_0122(__m128i a)
{
    int32x2_t a22 = vdup_lane_s32(vget_high_s32(vreinterpretq_s32_m128i(a)), 0);
    int32x2_t a01 = vrev64_s32(vget_low_s32(vreinterpretq_s32_m128i(a)));
    return vreinterpretq_m128i_s32(vcombine_s32(a22, a01));
}

FORCE_INLINE __m128i _mm_shuffle_epi_3332(__m128i a)
{
    int32x2_t a32 = vget_high_s32(vreinterpretq_s32_m128i(a));
    int32x2_t a33 = vdup_lane_s32(vget_high_s32(vreinterpretq_s32_m128i(a)), 1);
    return vreinterpretq_m128i_s32(vcombine_s32(a32, a33));
}

#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
#define _mm_shuffle_epi32_splat(a, imm) \
    vreinterpretq_m128i_s32(vdupq_laneq_s32(vreinterpretq_s32_m128i(a), (imm)))
#else
#define _mm_shuffle_epi32_splat(a, imm) \
    vreinterpretq_m128i_s32(            \
        vdupq_n_s32(vgetq_lane_s32(vreinterpretq_s32_m128i(a), (imm))))
#endif


#define _mm_shuffle_ps_default(a, b, imm)                                      \
    vreinterpretq_m128_f32(vsetq_lane_f32(                                     \
        vgetq_lane_f32(vreinterpretq_f32_m128(b), ((imm) >> 6) & 0x3),         \
        vsetq_lane_f32(                                                        \
            vgetq_lane_f32(vreinterpretq_f32_m128(b), ((imm) >> 4) & 0x3),     \
            vsetq_lane_f32(                                                    \
                vgetq_lane_f32(vreinterpretq_f32_m128(a), ((imm) >> 2) & 0x3), \
                vmovq_n_f32(                                                   \
                    vgetq_lane_f32(vreinterpretq_f32_m128(a), (imm) & (0x3))), \
                1),                                                            \
            2),                                                                \
        3))


#define _mm_shufflelo_epi16_function(a, imm)                                  \
    _sse2neon_define1(                                                        \
        __m128i, a, int16x8_t ret = vreinterpretq_s16_m128i(_a);              \
        int16x4_t lowBits = vget_low_s16(ret);                                \
        ret = vsetq_lane_s16(vget_lane_s16(lowBits, (imm) & (0x3)), ret, 0);  \
        ret = vsetq_lane_s16(vget_lane_s16(lowBits, ((imm) >> 2) & 0x3), ret, \
                             1);                                              \
        ret = vsetq_lane_s16(vget_lane_s16(lowBits, ((imm) >> 4) & 0x3), ret, \
                             2);                                              \
        ret = vsetq_lane_s16(vget_lane_s16(lowBits, ((imm) >> 6) & 0x3), ret, \
                             3);                                              \
        _sse2neon_return(vreinterpretq_m128i_s16(ret));)


#define _mm_shufflehi_epi16_function(a, imm)                                   \
    _sse2neon_define1(                                                         \
        __m128i, a, int16x8_t ret = vreinterpretq_s16_m128i(_a);               \
        int16x4_t highBits = vget_high_s16(ret);                               \
        ret = vsetq_lane_s16(vget_lane_s16(highBits, (imm) & (0x3)), ret, 4);  \
        ret = vsetq_lane_s16(vget_lane_s16(highBits, ((imm) >> 2) & 0x3), ret, \
                             5);                                               \
        ret = vsetq_lane_s16(vget_lane_s16(highBits, ((imm) >> 4) & 0x3), ret, \
                             6);                                               \
        ret = vsetq_lane_s16(vget_lane_s16(highBits, ((imm) >> 6) & 0x3), ret, \
                             7);                                               \
        _sse2neon_return(vreinterpretq_m128i_s16(ret));)


FORCE_INLINE void _mm_empty(void) {}


FORCE_INLINE __m128 _mm_add_ps(__m128 a, __m128 b)
{
    return vreinterpretq_m128_f32(
        vaddq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(b)));
}


FORCE_INLINE __m128 _mm_add_ss(__m128 a, __m128 b)
{
    float32_t b0 = vgetq_lane_f32(vreinterpretq_f32_m128(b), 0);
    float32x4_t value = vsetq_lane_f32(b0, vdupq_n_f32(0), 0);

    return vreinterpretq_m128_f32(vaddq_f32(a, value));
}


FORCE_INLINE __m128 _mm_and_ps(__m128 a, __m128 b)
{
    return vreinterpretq_m128_s32(
        vandq_s32(vreinterpretq_s32_m128(a), vreinterpretq_s32_m128(b)));
}


FORCE_INLINE __m128 _mm_andnot_ps(__m128 a, __m128 b)
{
    return vreinterpretq_m128_s32(
        vbicq_s32(vreinterpretq_s32_m128(b),
                  vreinterpretq_s32_m128(a)));
}


FORCE_INLINE __m64 _mm_avg_pu16(__m64 a, __m64 b)
{
    return vreinterpret_m64_u16(
        vrhadd_u16(vreinterpret_u16_m64(a), vreinterpret_u16_m64(b)));
}


FORCE_INLINE __m64 _mm_avg_pu8(__m64 a, __m64 b)
{
    return vreinterpret_m64_u8(
        vrhadd_u8(vreinterpret_u8_m64(a), vreinterpret_u8_m64(b)));
}


FORCE_INLINE __m128 _mm_cmpeq_ps(__m128 a, __m128 b)
{
    return vreinterpretq_m128_u32(
        vceqq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(b)));
}


FORCE_INLINE __m128 _mm_cmpeq_ss(__m128 a, __m128 b)
{
    return _mm_move_ss(a, _mm_cmpeq_ps(a, b));
}


FORCE_INLINE __m128 _mm_cmpge_ps(__m128 a, __m128 b)
{
    return vreinterpretq_m128_u32(
        vcgeq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(b)));
}


FORCE_INLINE __m128 _mm_cmpge_ss(__m128 a, __m128 b)
{
    return _mm_move_ss(a, _mm_cmpge_ps(a, b));
}


FORCE_INLINE __m128 _mm_cmpgt_ps(__m128 a, __m128 b)
{
    return vreinterpretq_m128_u32(
        vcgtq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(b)));
}


FORCE_INLINE __m128 _mm_cmpgt_ss(__m128 a, __m128 b)
{
    return _mm_move_ss(a, _mm_cmpgt_ps(a, b));
}


FORCE_INLINE __m128 _mm_cmple_ps(__m128 a, __m128 b)
{
    return vreinterpretq_m128_u32(
        vcleq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(b)));
}


FORCE_INLINE __m128 _mm_cmple_ss(__m128 a, __m128 b)
{
    return _mm_move_ss(a, _mm_cmple_ps(a, b));
}


FORCE_INLINE __m128 _mm_cmplt_ps(__m128 a, __m128 b)
{
    return vreinterpretq_m128_u32(
        vcltq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(b)));
}


FORCE_INLINE __m128 _mm_cmplt_ss(__m128 a, __m128 b)
{
    return _mm_move_ss(a, _mm_cmplt_ps(a, b));
}


FORCE_INLINE __m128 _mm_cmpneq_ps(__m128 a, __m128 b)
{
    return vreinterpretq_m128_u32(vmvnq_u32(
        vceqq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(b))));
}


FORCE_INLINE __m128 _mm_cmpneq_ss(__m128 a, __m128 b)
{
    return _mm_move_ss(a, _mm_cmpneq_ps(a, b));
}


FORCE_INLINE __m128 _mm_cmpnge_ps(__m128 a, __m128 b)
{
    return vreinterpretq_m128_u32(vmvnq_u32(
        vcgeq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(b))));
}


FORCE_INLINE __m128 _mm_cmpnge_ss(__m128 a, __m128 b)
{
    return _mm_move_ss(a, _mm_cmpnge_ps(a, b));
}


FORCE_INLINE __m128 _mm_cmpngt_ps(__m128 a, __m128 b)
{
    return vreinterpretq_m128_u32(vmvnq_u32(
        vcgtq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(b))));
}


FORCE_INLINE __m128 _mm_cmpngt_ss(__m128 a, __m128 b)
{
    return _mm_move_ss(a, _mm_cmpngt_ps(a, b));
}


FORCE_INLINE __m128 _mm_cmpnle_ps(__m128 a, __m128 b)
{
    return vreinterpretq_m128_u32(vmvnq_u32(
        vcleq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(b))));
}


FORCE_INLINE __m128 _mm_cmpnle_ss(__m128 a, __m128 b)
{
    return _mm_move_ss(a, _mm_cmpnle_ps(a, b));
}


FORCE_INLINE __m128 _mm_cmpnlt_ps(__m128 a, __m128 b)
{
    return vreinterpretq_m128_u32(vmvnq_u32(
        vcltq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(b))));
}


FORCE_INLINE __m128 _mm_cmpnlt_ss(__m128 a, __m128 b)
{
    return _mm_move_ss(a, _mm_cmpnlt_ps(a, b));
}


FORCE_INLINE __m128 _mm_cmpord_ps(__m128 a, __m128 b)
{


    uint32x4_t ceqaa =
        vceqq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(a));
    uint32x4_t ceqbb =
        vceqq_f32(vreinterpretq_f32_m128(b), vreinterpretq_f32_m128(b));
    return vreinterpretq_m128_u32(vandq_u32(ceqaa, ceqbb));
}


FORCE_INLINE __m128 _mm_cmpord_ss(__m128 a, __m128 b)
{
    return _mm_move_ss(a, _mm_cmpord_ps(a, b));
}


FORCE_INLINE __m128 _mm_cmpunord_ps(__m128 a, __m128 b)
{
    uint32x4_t f32a =
        vceqq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(a));
    uint32x4_t f32b =
        vceqq_f32(vreinterpretq_f32_m128(b), vreinterpretq_f32_m128(b));
    return vreinterpretq_m128_u32(vmvnq_u32(vandq_u32(f32a, f32b)));
}


FORCE_INLINE __m128 _mm_cmpunord_ss(__m128 a, __m128 b)
{
    return _mm_move_ss(a, _mm_cmpunord_ps(a, b));
}


FORCE_INLINE int _mm_comieq_ss(__m128 a, __m128 b)
{
    uint32x4_t a_eq_b =
        vceqq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(b));
    return vgetq_lane_u32(a_eq_b, 0) & 0x1;
}


FORCE_INLINE int _mm_comige_ss(__m128 a, __m128 b)
{
    uint32x4_t a_ge_b =
        vcgeq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(b));
    return vgetq_lane_u32(a_ge_b, 0) & 0x1;
}


FORCE_INLINE int _mm_comigt_ss(__m128 a, __m128 b)
{
    uint32x4_t a_gt_b =
        vcgtq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(b));
    return vgetq_lane_u32(a_gt_b, 0) & 0x1;
}


FORCE_INLINE int _mm_comile_ss(__m128 a, __m128 b)
{
    uint32x4_t a_le_b =
        vcleq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(b));
    return vgetq_lane_u32(a_le_b, 0) & 0x1;
}


FORCE_INLINE int _mm_comilt_ss(__m128 a, __m128 b)
{
    uint32x4_t a_lt_b =
        vcltq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(b));
    return vgetq_lane_u32(a_lt_b, 0) & 0x1;
}


FORCE_INLINE int _mm_comineq_ss(__m128 a, __m128 b)
{
    return !_mm_comieq_ss(a, b);
}


FORCE_INLINE __m128 _mm_cvt_pi2ps(__m128 a, __m64 b)
{
    return vreinterpretq_m128_f32(
        vcombine_f32(vcvt_f32_s32(vreinterpret_s32_m64(b)),
                     vget_high_f32(vreinterpretq_f32_m128(a))));
}


FORCE_INLINE __m64 _mm_cvt_ps2pi(__m128 a)
{
#if (defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)) || \
    defined(__ARM_FEATURE_DIRECTED_ROUNDING)
    return vreinterpret_m64_s32(
        vget_low_s32(vcvtnq_s32_f32(vrndiq_f32(vreinterpretq_f32_m128(a)))));
#else
    return vreinterpret_m64_s32(vcvt_s32_f32(vget_low_f32(
        vreinterpretq_f32_m128(_mm_round_ps(a, _MM_FROUND_CUR_DIRECTION)))));
#endif
}


FORCE_INLINE __m128 _mm_cvt_si2ss(__m128 a, int b)
{
    return vreinterpretq_m128_f32(
        vsetq_lane_f32((float) b, vreinterpretq_f32_m128(a), 0));
}


FORCE_INLINE int _mm_cvt_ss2si(__m128 a)
{
#if (defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)) || \
    defined(__ARM_FEATURE_DIRECTED_ROUNDING)
    return vgetq_lane_s32(vcvtnq_s32_f32(vrndiq_f32(vreinterpretq_f32_m128(a))),
                          0);
#else
    float32_t data = vgetq_lane_f32(
        vreinterpretq_f32_m128(_mm_round_ps(a, _MM_FROUND_CUR_DIRECTION)), 0);
    return (int32_t) data;
#endif
}


FORCE_INLINE __m128 _mm_cvtpi16_ps(__m64 a)
{
    return vreinterpretq_m128_f32(
        vcvtq_f32_s32(vmovl_s16(vreinterpret_s16_m64(a))));
}


FORCE_INLINE __m128 _mm_cvtpi32_ps(__m128 a, __m64 b)
{
    return vreinterpretq_m128_f32(
        vcombine_f32(vcvt_f32_s32(vreinterpret_s32_m64(b)),
                     vget_high_f32(vreinterpretq_f32_m128(a))));
}


FORCE_INLINE __m128 _mm_cvtpi32x2_ps(__m64 a, __m64 b)
{
    return vreinterpretq_m128_f32(vcvtq_f32_s32(
        vcombine_s32(vreinterpret_s32_m64(a), vreinterpret_s32_m64(b))));
}


FORCE_INLINE __m128 _mm_cvtpi8_ps(__m64 a)
{
    return vreinterpretq_m128_f32(vcvtq_f32_s32(
        vmovl_s16(vget_low_s16(vmovl_s8(vreinterpret_s8_m64(a))))));
}


FORCE_INLINE __m64 _mm_cvtps_pi16(__m128 a)
{
    return vreinterpret_m64_s16(
        vqmovn_s32(vreinterpretq_s32_m128i(_mm_cvtps_epi32(a))));
}


#define _mm_cvtps_pi32(a) _mm_cvt_ps2pi(a)


FORCE_INLINE __m64 _mm_cvtps_pi8(__m128 a)
{
    return vreinterpret_m64_s8(vqmovn_s16(
        vcombine_s16(vreinterpret_s16_m64(_mm_cvtps_pi16(a)), vdup_n_s16(0))));
}


FORCE_INLINE __m128 _mm_cvtpu16_ps(__m64 a)
{
    return vreinterpretq_m128_f32(
        vcvtq_f32_u32(vmovl_u16(vreinterpret_u16_m64(a))));
}


FORCE_INLINE __m128 _mm_cvtpu8_ps(__m64 a)
{
    return vreinterpretq_m128_f32(vcvtq_f32_u32(
        vmovl_u16(vget_low_u16(vmovl_u8(vreinterpret_u8_m64(a))))));
}


#define _mm_cvtsi32_ss(a, b) _mm_cvt_si2ss(a, b)


FORCE_INLINE __m128 _mm_cvtsi64_ss(__m128 a, int64_t b)
{
    return vreinterpretq_m128_f32(
        vsetq_lane_f32((float) b, vreinterpretq_f32_m128(a), 0));
}


FORCE_INLINE float _mm_cvtss_f32(__m128 a)
{
    return vgetq_lane_f32(vreinterpretq_f32_m128(a), 0);
}


#define _mm_cvtss_si32(a) _mm_cvt_ss2si(a)


FORCE_INLINE int64_t _mm_cvtss_si64(__m128 a)
{
#if (defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)) || \
    defined(__ARM_FEATURE_DIRECTED_ROUNDING)
    return (int64_t) vgetq_lane_f32(vrndiq_f32(vreinterpretq_f32_m128(a)), 0);
#else
    float32_t data = vgetq_lane_f32(
        vreinterpretq_f32_m128(_mm_round_ps(a, _MM_FROUND_CUR_DIRECTION)), 0);
    return (int64_t) data;
#endif
}


FORCE_INLINE __m64 _mm_cvtt_ps2pi(__m128 a)
{
    return vreinterpret_m64_s32(
        vget_low_s32(vcvtq_s32_f32(vreinterpretq_f32_m128(a))));
}


FORCE_INLINE int _mm_cvtt_ss2si(__m128 a)
{
    return vgetq_lane_s32(vcvtq_s32_f32(vreinterpretq_f32_m128(a)), 0);
}


#define _mm_cvttps_pi32(a) _mm_cvtt_ps2pi(a)


#define _mm_cvttss_si32(a) _mm_cvtt_ss2si(a)


FORCE_INLINE int64_t _mm_cvttss_si64(__m128 a)
{
    return (int64_t) vgetq_lane_f32(vreinterpretq_f32_m128(a), 0);
}


FORCE_INLINE __m128 _mm_div_ps(__m128 a, __m128 b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128_f32(
        vdivq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(b)));
#else
    float32x4_t recip = vrecpeq_f32(vreinterpretq_f32_m128(b));
    recip = vmulq_f32(recip, vrecpsq_f32(recip, vreinterpretq_f32_m128(b)));

    recip = vmulq_f32(recip, vrecpsq_f32(recip, vreinterpretq_f32_m128(b)));
    return vreinterpretq_m128_f32(vmulq_f32(vreinterpretq_f32_m128(a), recip));
#endif
}


FORCE_INLINE __m128 _mm_div_ss(__m128 a, __m128 b)
{
    float32_t value =
        vgetq_lane_f32(vreinterpretq_f32_m128(_mm_div_ps(a, b)), 0);
    return vreinterpretq_m128_f32(
        vsetq_lane_f32(value, vreinterpretq_f32_m128(a), 0));
}


#define _mm_extract_pi16(a, imm) \
    (int32_t) vget_lane_u16(vreinterpret_u16_m64(a), (imm))


#if !defined(SSE2NEON_ALLOC_DEFINED)
FORCE_INLINE void _mm_free(void *addr)
{
#if defined(_WIN32)
    _aligned_free(addr);
#else
    free(addr);
#endif
}
#endif

FORCE_INLINE uint64_t _sse2neon_get_fpcr(void)
{
    uint64_t value;
#if defined(_MSC_VER) && !defined(__clang__)
    value = _ReadStatusReg(ARM64_FPCR);
#else
    __asm__ __volatile__("mrs %0, FPCR" : "=r"(value));
#endif
    return value;
}

FORCE_INLINE void _sse2neon_set_fpcr(uint64_t value)
{
#if defined(_MSC_VER) && !defined(__clang__)
    _WriteStatusReg(ARM64_FPCR, value);
#else
    __asm__ __volatile__("msr FPCR, %0" ::"r"(value));
#endif
}


FORCE_INLINE unsigned int _sse2neon_mm_get_flush_zero_mode(void)
{
    union {
        fpcr_bitfield field;
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
        uint64_t value;
#else
        uint32_t value;
#endif
    } r;

#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    r.value = _sse2neon_get_fpcr();
#else
    __asm__ __volatile__("vmrs %0, FPSCR" : "=r"(r.value));
#endif

    return r.field.bit24 ? _MM_FLUSH_ZERO_ON : _MM_FLUSH_ZERO_OFF;
}


FORCE_INLINE unsigned int _MM_GET_ROUNDING_MODE(void)
{
    switch (fegetround()) {
    case FE_TONEAREST:
        return _MM_ROUND_NEAREST;
    case FE_DOWNWARD:
        return _MM_ROUND_DOWN;
    case FE_UPWARD:
        return _MM_ROUND_UP;
    case FE_TOWARDZERO:
        return _MM_ROUND_TOWARD_ZERO;
    default:


        return _MM_ROUND_TOWARD_ZERO;
    }
}


#define _mm_insert_pi16(a, b, imm) \
    vreinterpret_m64_s16(vset_lane_s16((b), vreinterpret_s16_m64(a), (imm)))


FORCE_INLINE __m128 _mm_load_ps(const float *p)
{
    return vreinterpretq_m128_f32(vld1q_f32(p));
}


#define _mm_load_ps1 _mm_load1_ps


FORCE_INLINE __m128 _mm_load_ss(const float *p)
{
    return vreinterpretq_m128_f32(vsetq_lane_f32(*p, vdupq_n_f32(0), 0));
}


FORCE_INLINE __m128 _mm_load1_ps(const float *p)
{
    return vreinterpretq_m128_f32(vld1q_dup_f32(p));
}


FORCE_INLINE __m128 _mm_loadh_pi(__m128 a, __m64 const *p)
{
    return vreinterpretq_m128_f32(
        vcombine_f32(vget_low_f32(a), vld1_f32((const float32_t *) p)));
}


FORCE_INLINE __m128 _mm_loadl_pi(__m128 a, __m64 const *p)
{
    return vreinterpretq_m128_f32(
        vcombine_f32(vld1_f32((const float32_t *) p), vget_high_f32(a)));
}


FORCE_INLINE __m128 _mm_loadr_ps(const float *p)
{
    float32x4_t v = vrev64q_f32(vld1q_f32(p));
    return vreinterpretq_m128_f32(vextq_f32(v, v, 2));
}


FORCE_INLINE __m128 _mm_loadu_ps(const float *p)
{


    return vreinterpretq_m128_f32(vld1q_f32(p));
}


FORCE_INLINE __m128i _mm_loadu_si16(const void *p)
{
    return vreinterpretq_m128i_s16(
        vsetq_lane_s16(*(const unaligned_int16_t *) p, vdupq_n_s16(0), 0));
}


FORCE_INLINE __m128i _mm_loadu_si64(const void *p)
{
    return vreinterpretq_m128i_s64(
        vsetq_lane_s64(*(const unaligned_int64_t *) p, vdupq_n_s64(0), 0));
}


#if !defined(SSE2NEON_ALLOC_DEFINED)
FORCE_INLINE void *_mm_malloc(size_t size, size_t align)
{
#if defined(_WIN32)
    return _aligned_malloc(size, align);
#else
    void *ptr;
    if (align == 1)
        return malloc(size);
    if (align == 2 || (sizeof(void *) == 8 && align == 4))
        align = sizeof(void *);
    if (!posix_memalign(&ptr, align, size))
        return ptr;
    return NULL;
#endif
}
#endif


FORCE_INLINE void _mm_maskmove_si64(__m64 a, __m64 mask, char *mem_addr)
{
    int8x8_t shr_mask = vshr_n_s8(vreinterpret_s8_m64(mask), 7);
    __m128 b = _mm_load_ps((const float *) mem_addr);
    int8x8_t masked =
        vbsl_s8(vreinterpret_u8_s8(shr_mask), vreinterpret_s8_m64(a),
                vreinterpret_s8_u64(vget_low_u64(vreinterpretq_u64_m128(b))));
    vst1_s8((int8_t *) mem_addr, masked);
}


#define _m_maskmovq(a, mask, mem_addr) _mm_maskmove_si64(a, mask, mem_addr)


FORCE_INLINE __m64 _mm_max_pi16(__m64 a, __m64 b)
{
    return vreinterpret_m64_s16(
        vmax_s16(vreinterpret_s16_m64(a), vreinterpret_s16_m64(b)));
}


FORCE_INLINE __m128 _mm_max_ps(__m128 a, __m128 b)
{
#if SSE2NEON_PRECISE_MINMAX
    float32x4_t _a = vreinterpretq_f32_m128(a);
    float32x4_t _b = vreinterpretq_f32_m128(b);
    return vreinterpretq_m128_f32(vbslq_f32(vcgtq_f32(_a, _b), _a, _b));
#else
    return vreinterpretq_m128_f32(
        vmaxq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(b)));
#endif
}


FORCE_INLINE __m64 _mm_max_pu8(__m64 a, __m64 b)
{
    return vreinterpret_m64_u8(
        vmax_u8(vreinterpret_u8_m64(a), vreinterpret_u8_m64(b)));
}


FORCE_INLINE __m128 _mm_max_ss(__m128 a, __m128 b)
{
    float32_t value = vgetq_lane_f32(_mm_max_ps(a, b), 0);
    return vreinterpretq_m128_f32(
        vsetq_lane_f32(value, vreinterpretq_f32_m128(a), 0));
}


FORCE_INLINE __m64 _mm_min_pi16(__m64 a, __m64 b)
{
    return vreinterpret_m64_s16(
        vmin_s16(vreinterpret_s16_m64(a), vreinterpret_s16_m64(b)));
}


FORCE_INLINE __m128 _mm_min_ps(__m128 a, __m128 b)
{
#if SSE2NEON_PRECISE_MINMAX
    float32x4_t _a = vreinterpretq_f32_m128(a);
    float32x4_t _b = vreinterpretq_f32_m128(b);
    return vreinterpretq_m128_f32(vbslq_f32(vcltq_f32(_a, _b), _a, _b));
#else
    return vreinterpretq_m128_f32(
        vminq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(b)));
#endif
}


FORCE_INLINE __m64 _mm_min_pu8(__m64 a, __m64 b)
{
    return vreinterpret_m64_u8(
        vmin_u8(vreinterpret_u8_m64(a), vreinterpret_u8_m64(b)));
}


FORCE_INLINE __m128 _mm_min_ss(__m128 a, __m128 b)
{
    float32_t value = vgetq_lane_f32(_mm_min_ps(a, b), 0);
    return vreinterpretq_m128_f32(
        vsetq_lane_f32(value, vreinterpretq_f32_m128(a), 0));
}


FORCE_INLINE __m128 _mm_move_ss(__m128 a, __m128 b)
{
    return vreinterpretq_m128_f32(
        vsetq_lane_f32(vgetq_lane_f32(vreinterpretq_f32_m128(b), 0),
                       vreinterpretq_f32_m128(a), 0));
}


FORCE_INLINE __m128 _mm_movehl_ps(__m128 a, __m128 b)
{
#if defined(aarch64__)
    return vreinterpretq_m128_u64(
        vzip2q_u64(vreinterpretq_u64_m128(b), vreinterpretq_u64_m128(a)));
#else
    float32x2_t a32 = vget_high_f32(vreinterpretq_f32_m128(a));
    float32x2_t b32 = vget_high_f32(vreinterpretq_f32_m128(b));
    return vreinterpretq_m128_f32(vcombine_f32(b32, a32));
#endif
}


FORCE_INLINE __m128 _mm_movelh_ps(__m128 __A, __m128 __B)
{
    float32x2_t a10 = vget_low_f32(vreinterpretq_f32_m128(__A));
    float32x2_t b10 = vget_low_f32(vreinterpretq_f32_m128(__B));
    return vreinterpretq_m128_f32(vcombine_f32(a10, b10));
}


FORCE_INLINE int _mm_movemask_pi8(__m64 a)
{
    uint8x8_t input = vreinterpret_u8_m64(a);
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    static const int8_t shift[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    uint8x8_t tmp = vshr_n_u8(input, 7);
    return vaddv_u8(vshl_u8(tmp, vld1_s8(shift)));
#else

    uint8x8_t msbs = vshr_n_u8(input, 7);
    uint32x2_t bits = vreinterpret_u32_u8(msbs);
    bits = vsra_n_u32(bits, bits, 7);
    bits = vsra_n_u32(bits, bits, 14);
    uint8x8_t output = vreinterpret_u8_u32(bits);
    return (vget_lane_u8(output, 4) << 4) | vget_lane_u8(output, 0);
#endif
}


FORCE_INLINE int _mm_movemask_ps(__m128 a)
{
    uint32x4_t input = vreinterpretq_u32_m128(a);
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    static const int32_t shift[4] = {0, 1, 2, 3};
    uint32x4_t tmp = vshrq_n_u32(input, 31);
    return vaddvq_u32(vshlq_u32(tmp, vld1q_s32(shift)));
#else

    uint32x4_t msbs = vshrq_n_u32(input, 31);
    uint64x2_t bits = vreinterpretq_u64_u32(msbs);
    bits = vsraq_n_u64(bits, bits, 31);
    uint8x16_t output = vreinterpretq_u8_u64(bits);
    return (vgetq_lane_u8(output, 8) << 2) | vgetq_lane_u8(output, 0);
#endif
}


FORCE_INLINE __m128 _mm_mul_ps(__m128 a, __m128 b)
{
    return vreinterpretq_m128_f32(
        vmulq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(b)));
}


FORCE_INLINE __m128 _mm_mul_ss(__m128 a, __m128 b)
{
    return _mm_move_ss(a, _mm_mul_ps(a, b));
}


FORCE_INLINE __m64 _mm_mulhi_pu16(__m64 a, __m64 b)
{
    return vreinterpret_m64_u16(vshrn_n_u32(
        vmull_u16(vreinterpret_u16_m64(a), vreinterpret_u16_m64(b)), 16));
}


FORCE_INLINE __m128 _mm_or_ps(__m128 a, __m128 b)
{
    return vreinterpretq_m128_s32(
        vorrq_s32(vreinterpretq_s32_m128(a), vreinterpretq_s32_m128(b)));
}


#define _m_pavgb(a, b) _mm_avg_pu8(a, b)


#define _m_pavgw(a, b) _mm_avg_pu16(a, b)


#define _m_pextrw(a, imm) _mm_extract_pi16(a, imm)


#define _m_pinsrw(a, i, imm) _mm_insert_pi16(a, i, imm)


#define _m_pmaxsw(a, b) _mm_max_pi16(a, b)


#define _m_pmaxub(a, b) _mm_max_pu8(a, b)


#define _m_pminsw(a, b) _mm_min_pi16(a, b)


#define _m_pminub(a, b) _mm_min_pu8(a, b)


#define _m_pmovmskb(a) _mm_movemask_pi8(a)


#define _m_pmulhuw(a, b) _mm_mulhi_pu16(a, b)


FORCE_INLINE void _mm_prefetch(char const *p, int i)
{
    (void) i;
#if defined(_MSC_VER) && !defined(__clang__)
    switch (i) {
    case _MM_HINT_NTA:
        __prefetch2(p, 1);
        break;
    case _MM_HINT_T0:
        __prefetch2(p, 0);
        break;
    case _MM_HINT_T1:
        __prefetch2(p, 2);
        break;
    case _MM_HINT_T2:
        __prefetch2(p, 4);
        break;
    }
#else
    switch (i) {
    case _MM_HINT_NTA:
        __builtin_prefetch(p, 0, 0);
        break;
    case _MM_HINT_T0:
        __builtin_prefetch(p, 0, 3);
        break;
    case _MM_HINT_T1:
        __builtin_prefetch(p, 0, 2);
        break;
    case _MM_HINT_T2:
        __builtin_prefetch(p, 0, 1);
        break;
    }
#endif
}


#define _m_psadbw(a, b) _mm_sad_pu8(a, b)


#define _m_pshufw(a, imm) _mm_shuffle_pi16(a, imm)


FORCE_INLINE __m128 _mm_rcp_ps(__m128 in)
{
    float32x4_t recip = vrecpeq_f32(vreinterpretq_f32_m128(in));
    recip = vmulq_f32(recip, vrecpsq_f32(recip, vreinterpretq_f32_m128(in)));
#if SSE2NEON_PRECISE_DIV

    recip = vmulq_f32(recip, vrecpsq_f32(recip, vreinterpretq_f32_m128(in)));
#endif
    return vreinterpretq_m128_f32(recip);
}


FORCE_INLINE __m128 _mm_rcp_ss(__m128 a)
{
    return _mm_move_ss(a, _mm_rcp_ps(a));
}


FORCE_INLINE __m128 _mm_rsqrt_ps(__m128 in)
{
    float32x4_t out = vrsqrteq_f32(vreinterpretq_f32_m128(in));


    const uint32x4_t pos_inf = vdupq_n_u32(0x7F800000);
    const uint32x4_t neg_inf = vdupq_n_u32(0xFF800000);
    const uint32x4_t has_pos_zero =
        vceqq_u32(pos_inf, vreinterpretq_u32_f32(out));
    const uint32x4_t has_neg_zero =
        vceqq_u32(neg_inf, vreinterpretq_u32_f32(out));

    out = vmulq_f32(
        out, vrsqrtsq_f32(vmulq_f32(vreinterpretq_f32_m128(in), out), out));
#if SSE2NEON_PRECISE_SQRT

    out = vmulq_f32(
        out, vrsqrtsq_f32(vmulq_f32(vreinterpretq_f32_m128(in), out), out));
#endif


    out = vbslq_f32(has_pos_zero, (float32x4_t) pos_inf, out);
    out = vbslq_f32(has_neg_zero, (float32x4_t) neg_inf, out);

    return vreinterpretq_m128_f32(out);
}


FORCE_INLINE __m128 _mm_rsqrt_ss(__m128 in)
{
    return vsetq_lane_f32(vgetq_lane_f32(_mm_rsqrt_ps(in), 0), in, 0);
}


FORCE_INLINE __m64 _mm_sad_pu8(__m64 a, __m64 b)
{
    uint64x1_t t = vpaddl_u32(vpaddl_u16(
        vpaddl_u8(vabd_u8(vreinterpret_u8_m64(a), vreinterpret_u8_m64(b)))));
    return vreinterpret_m64_u16(
        vset_lane_u16((uint16_t) vget_lane_u64(t, 0), vdup_n_u16(0), 0));
}


FORCE_INLINE void _sse2neon_mm_set_flush_zero_mode(unsigned int flag)
{


    union {
        fpcr_bitfield field;
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
        uint64_t value;
#else
        uint32_t value;
#endif
    } r;

#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    r.value = _sse2neon_get_fpcr();
#else
    __asm__ __volatile__("vmrs %0, FPSCR" : "=r"(r.value));
#endif

    r.field.bit24 = (flag & _MM_FLUSH_ZERO_MASK) == _MM_FLUSH_ZERO_ON;

#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    _sse2neon_set_fpcr(r.value);
#else
    __asm__ __volatile__("vmsr FPSCR, %0" ::"r"(r));
#endif
}


FORCE_INLINE __m128 _mm_set_ps(float w, float z, float y, float x)
{
    float ALIGN_STRUCT(16) data[4] = {x, y, z, w};
    return vreinterpretq_m128_f32(vld1q_f32(data));
}


FORCE_INLINE __m128 _mm_set_ps1(float _w)
{
    return vreinterpretq_m128_f32(vdupq_n_f32(_w));
}


FORCE_INLINE void _MM_SET_ROUNDING_MODE(int rounding)
{
    switch (rounding) {
    case _MM_ROUND_NEAREST:
        rounding = FE_TONEAREST;
        break;
    case _MM_ROUND_DOWN:
        rounding = FE_DOWNWARD;
        break;
    case _MM_ROUND_UP:
        rounding = FE_UPWARD;
        break;
    case _MM_ROUND_TOWARD_ZERO:
        rounding = FE_TOWARDZERO;
        break;
    default:


        rounding = FE_TOWARDZERO;
    }
    fesetround(rounding);
}


FORCE_INLINE __m128 _mm_set_ss(float a)
{
    return vreinterpretq_m128_f32(vsetq_lane_f32(a, vdupq_n_f32(0), 0));
}


FORCE_INLINE __m128 _mm_set1_ps(float _w)
{
    return vreinterpretq_m128_f32(vdupq_n_f32(_w));
}


FORCE_INLINE void _mm_setcsr(unsigned int a)
{
    _MM_SET_ROUNDING_MODE(a);
}


FORCE_INLINE unsigned int _mm_getcsr(void)
{
    return _MM_GET_ROUNDING_MODE();
}


FORCE_INLINE __m128 _mm_setr_ps(float w, float z, float y, float x)
{
    float ALIGN_STRUCT(16) data[4] = {w, z, y, x};
    return vreinterpretq_m128_f32(vld1q_f32(data));
}


FORCE_INLINE __m128 _mm_setzero_ps(void)
{
    return vreinterpretq_m128_f32(vdupq_n_f32(0));
}


#ifdef _sse2neon_shuffle
#define _mm_shuffle_pi16(a, imm)                                         \
    vreinterpret_m64_s16(vshuffle_s16(                                   \
        vreinterpret_s16_m64(a), vreinterpret_s16_m64(a), ((imm) & 0x3), \
        (((imm) >> 2) & 0x3), (((imm) >> 4) & 0x3), (((imm) >> 6) & 0x3)))
#else
#define _mm_shuffle_pi16(a, imm)                                              \
    _sse2neon_define1(                                                        \
        __m64, a, int16x4_t ret;                                              \
        ret = vmov_n_s16(                                                     \
            vget_lane_s16(vreinterpret_s16_m64(_a), (imm) & (0x3)));          \
        ret = vset_lane_s16(                                                  \
            vget_lane_s16(vreinterpret_s16_m64(_a), ((imm) >> 2) & 0x3), ret, \
            1);                                                               \
        ret = vset_lane_s16(                                                  \
            vget_lane_s16(vreinterpret_s16_m64(_a), ((imm) >> 4) & 0x3), ret, \
            2);                                                               \
        ret = vset_lane_s16(                                                  \
            vget_lane_s16(vreinterpret_s16_m64(_a), ((imm) >> 6) & 0x3), ret, \
            3);                                                               \
        _sse2neon_return(vreinterpret_m64_s16(ret));)
#endif


FORCE_INLINE void _mm_sfence(void)
{
    _sse2neon_smp_mb();
}


FORCE_INLINE void _mm_mfence(void)
{
    _sse2neon_smp_mb();
}


FORCE_INLINE void _mm_lfence(void)
{
    _sse2neon_smp_mb();
}


#ifdef _sse2neon_shuffle
#define _mm_shuffle_ps(a, b, imm)                                              \
    __extension__({                                                            \
        float32x4_t _input1 = vreinterpretq_f32_m128(a);                       \
        float32x4_t _input2 = vreinterpretq_f32_m128(b);                       \
        float32x4_t _shuf =                                                    \
            vshuffleq_s32(_input1, _input2, (imm) & (0x3), ((imm) >> 2) & 0x3, \
                          (((imm) >> 4) & 0x3) + 4, (((imm) >> 6) & 0x3) + 4); \
        vreinterpretq_m128_f32(_shuf);                                         \
    })
#else
#define _mm_shuffle_ps(a, b, imm)                            \
    _sse2neon_define2(                                       \
        __m128, a, b, __m128 ret; switch (imm) {             \
            case _MM_SHUFFLE(1, 0, 3, 2):                    \
                ret = _mm_shuffle_ps_1032(_a, _b);           \
                break;                                       \
            case _MM_SHUFFLE(2, 3, 0, 1):                    \
                ret = _mm_shuffle_ps_2301(_a, _b);           \
                break;                                       \
            case _MM_SHUFFLE(0, 3, 2, 1):                    \
                ret = _mm_shuffle_ps_0321(_a, _b);           \
                break;                                       \
            case _MM_SHUFFLE(2, 1, 0, 3):                    \
                ret = _mm_shuffle_ps_2103(_a, _b);           \
                break;                                       \
            case _MM_SHUFFLE(1, 0, 1, 0):                    \
                ret = _mm_movelh_ps(_a, _b);                 \
                break;                                       \
            case _MM_SHUFFLE(1, 0, 0, 1):                    \
                ret = _mm_shuffle_ps_1001(_a, _b);           \
                break;                                       \
            case _MM_SHUFFLE(0, 1, 0, 1):                    \
                ret = _mm_shuffle_ps_0101(_a, _b);           \
                break;                                       \
            case _MM_SHUFFLE(3, 2, 1, 0):                    \
                ret = _mm_shuffle_ps_3210(_a, _b);           \
                break;                                       \
            case _MM_SHUFFLE(0, 0, 1, 1):                    \
                ret = _mm_shuffle_ps_0011(_a, _b);           \
                break;                                       \
            case _MM_SHUFFLE(0, 0, 2, 2):                    \
                ret = _mm_shuffle_ps_0022(_a, _b);           \
                break;                                       \
            case _MM_SHUFFLE(2, 2, 0, 0):                    \
                ret = _mm_shuffle_ps_2200(_a, _b);           \
                break;                                       \
            case _MM_SHUFFLE(3, 2, 0, 2):                    \
                ret = _mm_shuffle_ps_3202(_a, _b);           \
                break;                                       \
            case _MM_SHUFFLE(3, 2, 3, 2):                    \
                ret = _mm_movehl_ps(_b, _a);                 \
                break;                                       \
            case _MM_SHUFFLE(1, 1, 3, 3):                    \
                ret = _mm_shuffle_ps_1133(_a, _b);           \
                break;                                       \
            case _MM_SHUFFLE(2, 0, 1, 0):                    \
                ret = _mm_shuffle_ps_2010(_a, _b);           \
                break;                                       \
            case _MM_SHUFFLE(2, 0, 0, 1):                    \
                ret = _mm_shuffle_ps_2001(_a, _b);           \
                break;                                       \
            case _MM_SHUFFLE(2, 0, 3, 2):                    \
                ret = _mm_shuffle_ps_2032(_a, _b);           \
                break;                                       \
            default:                                         \
                ret = _mm_shuffle_ps_default(_a, _b, (imm)); \
                break;                                       \
        } _sse2neon_return(ret);)
#endif


FORCE_INLINE __m128 _mm_sqrt_ps(__m128 in)
{
#if (defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)) && \
    !SSE2NEON_PRECISE_SQRT
    return vreinterpretq_m128_f32(vsqrtq_f32(vreinterpretq_f32_m128(in)));
#else
    float32x4_t recip = vrsqrteq_f32(vreinterpretq_f32_m128(in));


    const uint32x4_t pos_inf = vdupq_n_u32(0x7F800000);
    const uint32x4_t div_by_zero =
        vceqq_u32(pos_inf, vreinterpretq_u32_f32(recip));
    recip = vreinterpretq_f32_u32(
        vandq_u32(vmvnq_u32(div_by_zero), vreinterpretq_u32_f32(recip)));

    recip = vmulq_f32(
        vrsqrtsq_f32(vmulq_f32(recip, recip), vreinterpretq_f32_m128(in)),
        recip);

    recip = vmulq_f32(
        vrsqrtsq_f32(vmulq_f32(recip, recip), vreinterpretq_f32_m128(in)),
        recip);


    return vreinterpretq_m128_f32(vmulq_f32(vreinterpretq_f32_m128(in), recip));
#endif
}


FORCE_INLINE __m128 _mm_sqrt_ss(__m128 in)
{
    float32_t value =
        vgetq_lane_f32(vreinterpretq_f32_m128(_mm_sqrt_ps(in)), 0);
    return vreinterpretq_m128_f32(
        vsetq_lane_f32(value, vreinterpretq_f32_m128(in), 0));
}


FORCE_INLINE void _mm_store_ps(float *p, __m128 a)
{
    vst1q_f32(p, vreinterpretq_f32_m128(a));
}


FORCE_INLINE void _mm_store_ps1(float *p, __m128 a)
{
    float32_t a0 = vgetq_lane_f32(vreinterpretq_f32_m128(a), 0);
    vst1q_f32(p, vdupq_n_f32(a0));
}


FORCE_INLINE void _mm_store_ss(float *p, __m128 a)
{
    vst1q_lane_f32(p, vreinterpretq_f32_m128(a), 0);
}


#define _mm_store1_ps _mm_store_ps1


FORCE_INLINE void _mm_storeh_pi(__m64 *p, __m128 a)
{
    *p = vreinterpret_m64_f32(vget_high_f32(a));
}


FORCE_INLINE void _mm_storel_pi(__m64 *p, __m128 a)
{
    *p = vreinterpret_m64_f32(vget_low_f32(a));
}


FORCE_INLINE void _mm_storer_ps(float *p, __m128 a)
{
    float32x4_t tmp = vrev64q_f32(vreinterpretq_f32_m128(a));
    float32x4_t rev = vextq_f32(tmp, tmp, 2);
    vst1q_f32(p, rev);
}


FORCE_INLINE void _mm_storeu_ps(float *p, __m128 a)
{
    vst1q_f32(p, vreinterpretq_f32_m128(a));
}


FORCE_INLINE void _mm_storeu_si16(void *p, __m128i a)
{
    vst1q_lane_s16((int16_t *) p, vreinterpretq_s16_m128i(a), 0);
}


FORCE_INLINE void _mm_storeu_si64(void *p, __m128i a)
{
    vst1q_lane_s64((int64_t *) p, vreinterpretq_s64_m128i(a), 0);
}


FORCE_INLINE void _mm_stream_pi(__m64 *p, __m64 a)
{
    vst1_s64((int64_t *) p, vreinterpret_s64_m64(a));
}


FORCE_INLINE void _mm_stream_ps(float *p, __m128 a)
{
#if __has_builtin(__builtin_nontemporal_store)
    __builtin_nontemporal_store(a, (float32x4_t *) p);
#else
    vst1q_f32(p, vreinterpretq_f32_m128(a));
#endif
}


FORCE_INLINE __m128 _mm_sub_ps(__m128 a, __m128 b)
{
    return vreinterpretq_m128_f32(
        vsubq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(b)));
}


FORCE_INLINE __m128 _mm_sub_ss(__m128 a, __m128 b)
{
    return _mm_move_ss(a, _mm_sub_ps(a, b));
}


#define _MM_TRANSPOSE4_PS(row0, row1, row2, row3)         \
    do {                                                  \
        float32x4x2_t ROW01 = vtrnq_f32(row0, row1);      \
        float32x4x2_t ROW23 = vtrnq_f32(row2, row3);      \
        row0 = vcombine_f32(vget_low_f32(ROW01.val[0]),   \
                            vget_low_f32(ROW23.val[0]));  \
        row1 = vcombine_f32(vget_low_f32(ROW01.val[1]),   \
                            vget_low_f32(ROW23.val[1]));  \
        row2 = vcombine_f32(vget_high_f32(ROW01.val[0]),  \
                            vget_high_f32(ROW23.val[0])); \
        row3 = vcombine_f32(vget_high_f32(ROW01.val[1]),  \
                            vget_high_f32(ROW23.val[1])); \
    } while (0)


#define _mm_ucomieq_ss _mm_comieq_ss
#define _mm_ucomige_ss _mm_comige_ss
#define _mm_ucomigt_ss _mm_comigt_ss
#define _mm_ucomile_ss _mm_comile_ss
#define _mm_ucomilt_ss _mm_comilt_ss
#define _mm_ucomineq_ss _mm_comineq_ss


FORCE_INLINE __m128i _mm_undefined_si128(void)
{
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#endif
    __m128i a;
#if defined(_MSC_VER)
    a = _mm_setzero_si128();
#endif
    return a;
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
}


FORCE_INLINE __m128 _mm_undefined_ps(void)
{
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#endif
    __m128 a;
#if defined(_MSC_VER)
    a = _mm_setzero_ps();
#endif
    return a;
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
}


FORCE_INLINE __m128 _mm_unpackhi_ps(__m128 a, __m128 b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128_f32(
        vzip2q_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(b)));
#else
    float32x2_t a1 = vget_high_f32(vreinterpretq_f32_m128(a));
    float32x2_t b1 = vget_high_f32(vreinterpretq_f32_m128(b));
    float32x2x2_t result = vzip_f32(a1, b1);
    return vreinterpretq_m128_f32(vcombine_f32(result.val[0], result.val[1]));
#endif
}


FORCE_INLINE __m128 _mm_unpacklo_ps(__m128 a, __m128 b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128_f32(
        vzip1q_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(b)));
#else
    float32x2_t a1 = vget_low_f32(vreinterpretq_f32_m128(a));
    float32x2_t b1 = vget_low_f32(vreinterpretq_f32_m128(b));
    float32x2x2_t result = vzip_f32(a1, b1);
    return vreinterpretq_m128_f32(vcombine_f32(result.val[0], result.val[1]));
#endif
}


FORCE_INLINE __m128 _mm_xor_ps(__m128 a, __m128 b)
{
    return vreinterpretq_m128_s32(
        veorq_s32(vreinterpretq_s32_m128(a), vreinterpretq_s32_m128(b)));
}


FORCE_INLINE __m128i _mm_add_epi16(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s16(
        vaddq_s16(vreinterpretq_s16_m128i(a), vreinterpretq_s16_m128i(b)));
}


FORCE_INLINE __m128i _mm_add_epi32(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s32(
        vaddq_s32(vreinterpretq_s32_m128i(a), vreinterpretq_s32_m128i(b)));
}


FORCE_INLINE __m128i _mm_add_epi64(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s64(
        vaddq_s64(vreinterpretq_s64_m128i(a), vreinterpretq_s64_m128i(b)));
}


FORCE_INLINE __m128i _mm_add_epi8(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s8(
        vaddq_s8(vreinterpretq_s8_m128i(a), vreinterpretq_s8_m128i(b)));
}


FORCE_INLINE __m128d _mm_add_pd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(
        vaddq_f64(vreinterpretq_f64_m128d(a), vreinterpretq_f64_m128d(b)));
#else
    double a0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    double a1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    double b0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    double b1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 1));
    double c[2];
    c[0] = a0 + b0;
    c[1] = a1 + b1;
    return vld1q_f32((float32_t *) c);
#endif
}


FORCE_INLINE __m128d _mm_add_sd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return _mm_move_sd(a, _mm_add_pd(a, b));
#else
    double a0, a1, b0;
    a0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    a1 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    b0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    double c[2];
    c[0] = a0 + b0;
    c[1] = a1;
    return vld1q_f32((float32_t *) c);
#endif
}


FORCE_INLINE __m64 _mm_add_si64(__m64 a, __m64 b)
{
    return vreinterpret_m64_s64(
        vadd_s64(vreinterpret_s64_m64(a), vreinterpret_s64_m64(b)));
}


FORCE_INLINE __m128i _mm_adds_epi16(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s16(
        vqaddq_s16(vreinterpretq_s16_m128i(a), vreinterpretq_s16_m128i(b)));
}


FORCE_INLINE __m128i _mm_adds_epi8(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s8(
        vqaddq_s8(vreinterpretq_s8_m128i(a), vreinterpretq_s8_m128i(b)));
}


FORCE_INLINE __m128i _mm_adds_epu16(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_u16(
        vqaddq_u16(vreinterpretq_u16_m128i(a), vreinterpretq_u16_m128i(b)));
}


FORCE_INLINE __m128i _mm_adds_epu8(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_u8(
        vqaddq_u8(vreinterpretq_u8_m128i(a), vreinterpretq_u8_m128i(b)));
}


FORCE_INLINE __m128d _mm_and_pd(__m128d a, __m128d b)
{
    return vreinterpretq_m128d_s64(
        vandq_s64(vreinterpretq_s64_m128d(a), vreinterpretq_s64_m128d(b)));
}


FORCE_INLINE __m128i _mm_and_si128(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s32(
        vandq_s32(vreinterpretq_s32_m128i(a), vreinterpretq_s32_m128i(b)));
}


FORCE_INLINE __m128d _mm_andnot_pd(__m128d a, __m128d b)
{

    return vreinterpretq_m128d_s64(
        vbicq_s64(vreinterpretq_s64_m128d(b), vreinterpretq_s64_m128d(a)));
}


FORCE_INLINE __m128i _mm_andnot_si128(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s32(
        vbicq_s32(vreinterpretq_s32_m128i(b),
                  vreinterpretq_s32_m128i(a)));
}


FORCE_INLINE __m128i _mm_avg_epu16(__m128i a, __m128i b)
{
    return (__m128i) vrhaddq_u16(vreinterpretq_u16_m128i(a),
                                 vreinterpretq_u16_m128i(b));
}


FORCE_INLINE __m128i _mm_avg_epu8(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_u8(
        vrhaddq_u8(vreinterpretq_u8_m128i(a), vreinterpretq_u8_m128i(b)));
}


#define _mm_bslli_si128(a, imm) _mm_slli_si128(a, imm)


#define _mm_bsrli_si128(a, imm) _mm_srli_si128(a, imm)


FORCE_INLINE __m128 _mm_castpd_ps(__m128d a)
{
    return vreinterpretq_m128_s64(vreinterpretq_s64_m128d(a));
}


FORCE_INLINE __m128i _mm_castpd_si128(__m128d a)
{
    return vreinterpretq_m128i_s64(vreinterpretq_s64_m128d(a));
}


FORCE_INLINE __m128d _mm_castps_pd(__m128 a)
{
    return vreinterpretq_m128d_s32(vreinterpretq_s32_m128(a));
}


FORCE_INLINE __m128i _mm_castps_si128(__m128 a)
{
    return vreinterpretq_m128i_s32(vreinterpretq_s32_m128(a));
}


FORCE_INLINE __m128d _mm_castsi128_pd(__m128i a)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(vreinterpretq_f64_m128i(a));
#else
    return vreinterpretq_m128d_f32(vreinterpretq_f32_m128i(a));
#endif
}


FORCE_INLINE __m128 _mm_castsi128_ps(__m128i a)
{
    return vreinterpretq_m128_s32(vreinterpretq_s32_m128i(a));
}


#if defined(__APPLE__)
#include <libkern/OSCacheControl.h>
#endif
FORCE_INLINE void _mm_clflush(void const *p)
{
    (void) p;


#if defined(__APPLE__)
    sys_icache_invalidate((void *) (uintptr_t) p, SSE2NEON_CACHELINE_SIZE);
#elif defined(__GNUC__) || defined(__clang__)
    uintptr_t ptr = (uintptr_t) p;
    __builtin___clear_cache((char *) ptr,
                            (char *) ptr + SSE2NEON_CACHELINE_SIZE);
#elif (_MSC_VER) && SSE2NEON_INCLUDE_WINDOWS_H
    FlushInstructionCache(GetCurrentProcess(), p, SSE2NEON_CACHELINE_SIZE);
#endif
}


FORCE_INLINE __m128i _mm_cmpeq_epi16(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_u16(
        vceqq_s16(vreinterpretq_s16_m128i(a), vreinterpretq_s16_m128i(b)));
}


FORCE_INLINE __m128i _mm_cmpeq_epi32(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_u32(
        vceqq_s32(vreinterpretq_s32_m128i(a), vreinterpretq_s32_m128i(b)));
}


FORCE_INLINE __m128i _mm_cmpeq_epi8(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_u8(
        vceqq_s8(vreinterpretq_s8_m128i(a), vreinterpretq_s8_m128i(b)));
}


FORCE_INLINE __m128d _mm_cmpeq_pd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_u64(
        vceqq_f64(vreinterpretq_f64_m128d(a), vreinterpretq_f64_m128d(b)));
#else

    uint32x4_t cmp =
        vceqq_u32(vreinterpretq_u32_m128d(a), vreinterpretq_u32_m128d(b));
    uint32x4_t swapped = vrev64q_u32(cmp);
    return vreinterpretq_m128d_u32(vandq_u32(cmp, swapped));
#endif
}


FORCE_INLINE __m128d _mm_cmpeq_sd(__m128d a, __m128d b)
{
    return _mm_move_sd(a, _mm_cmpeq_pd(a, b));
}


FORCE_INLINE __m128d _mm_cmpge_pd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_u64(
        vcgeq_f64(vreinterpretq_f64_m128d(a), vreinterpretq_f64_m128d(b)));
#else
    double a0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    double a1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    double b0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    double b1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 1));
    uint64_t d[2];
    d[0] = a0 >= b0 ? ~UINT64_C(0) : UINT64_C(0);
    d[1] = a1 >= b1 ? ~UINT64_C(0) : UINT64_C(0);

    return vreinterpretq_m128d_u64(vld1q_u64(d));
#endif
}


FORCE_INLINE __m128d _mm_cmpge_sd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return _mm_move_sd(a, _mm_cmpge_pd(a, b));
#else

    double a0, b0;
    a0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    uint64_t a1 = vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1);
    b0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    uint64_t d[2];
    d[0] = a0 >= b0 ? ~UINT64_C(0) : UINT64_C(0);
    d[1] = a1;

    return vreinterpretq_m128d_u64(vld1q_u64(d));
#endif
}


FORCE_INLINE __m128i _mm_cmpgt_epi16(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_u16(
        vcgtq_s16(vreinterpretq_s16_m128i(a), vreinterpretq_s16_m128i(b)));
}


FORCE_INLINE __m128i _mm_cmpgt_epi32(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_u32(
        vcgtq_s32(vreinterpretq_s32_m128i(a), vreinterpretq_s32_m128i(b)));
}


FORCE_INLINE __m128i _mm_cmpgt_epi8(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_u8(
        vcgtq_s8(vreinterpretq_s8_m128i(a), vreinterpretq_s8_m128i(b)));
}


FORCE_INLINE __m128d _mm_cmpgt_pd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_u64(
        vcgtq_f64(vreinterpretq_f64_m128d(a), vreinterpretq_f64_m128d(b)));
#else
    double a0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    double a1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    double b0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    double b1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 1));
    uint64_t d[2];
    d[0] = a0 > b0 ? ~UINT64_C(0) : UINT64_C(0);
    d[1] = a1 > b1 ? ~UINT64_C(0) : UINT64_C(0);

    return vreinterpretq_m128d_u64(vld1q_u64(d));
#endif
}


FORCE_INLINE __m128d _mm_cmpgt_sd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return _mm_move_sd(a, _mm_cmpgt_pd(a, b));
#else

    double a0, b0;
    a0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    uint64_t a1 = vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1);
    b0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    uint64_t d[2];
    d[0] = a0 > b0 ? ~UINT64_C(0) : UINT64_C(0);
    d[1] = a1;

    return vreinterpretq_m128d_u64(vld1q_u64(d));
#endif
}


FORCE_INLINE __m128d _mm_cmple_pd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_u64(
        vcleq_f64(vreinterpretq_f64_m128d(a), vreinterpretq_f64_m128d(b)));
#else
    double a0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    double a1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    double b0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    double b1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 1));
    uint64_t d[2];
    d[0] = a0 <= b0 ? ~UINT64_C(0) : UINT64_C(0);
    d[1] = a1 <= b1 ? ~UINT64_C(0) : UINT64_C(0);

    return vreinterpretq_m128d_u64(vld1q_u64(d));
#endif
}


FORCE_INLINE __m128d _mm_cmple_sd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return _mm_move_sd(a, _mm_cmple_pd(a, b));
#else

    double a0, b0;
    a0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    uint64_t a1 = vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1);
    b0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    uint64_t d[2];
    d[0] = a0 <= b0 ? ~UINT64_C(0) : UINT64_C(0);
    d[1] = a1;

    return vreinterpretq_m128d_u64(vld1q_u64(d));
#endif
}


FORCE_INLINE __m128i _mm_cmplt_epi16(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_u16(
        vcltq_s16(vreinterpretq_s16_m128i(a), vreinterpretq_s16_m128i(b)));
}


FORCE_INLINE __m128i _mm_cmplt_epi32(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_u32(
        vcltq_s32(vreinterpretq_s32_m128i(a), vreinterpretq_s32_m128i(b)));
}


FORCE_INLINE __m128i _mm_cmplt_epi8(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_u8(
        vcltq_s8(vreinterpretq_s8_m128i(a), vreinterpretq_s8_m128i(b)));
}


FORCE_INLINE __m128d _mm_cmplt_pd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_u64(
        vcltq_f64(vreinterpretq_f64_m128d(a), vreinterpretq_f64_m128d(b)));
#else
    double a0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    double a1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    double b0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    double b1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 1));
    uint64_t d[2];
    d[0] = a0 < b0 ? ~UINT64_C(0) : UINT64_C(0);
    d[1] = a1 < b1 ? ~UINT64_C(0) : UINT64_C(0);

    return vreinterpretq_m128d_u64(vld1q_u64(d));
#endif
}


FORCE_INLINE __m128d _mm_cmplt_sd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return _mm_move_sd(a, _mm_cmplt_pd(a, b));
#else
    double a0, b0;
    a0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    uint64_t a1 = vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1);
    b0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    uint64_t d[2];
    d[0] = a0 < b0 ? ~UINT64_C(0) : UINT64_C(0);
    d[1] = a1;

    return vreinterpretq_m128d_u64(vld1q_u64(d));
#endif
}


FORCE_INLINE __m128d _mm_cmpneq_pd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_s32(vmvnq_s32(vreinterpretq_s32_u64(
        vceqq_f64(vreinterpretq_f64_m128d(a), vreinterpretq_f64_m128d(b)))));
#else

    uint32x4_t cmp =
        vceqq_u32(vreinterpretq_u32_m128d(a), vreinterpretq_u32_m128d(b));
    uint32x4_t swapped = vrev64q_u32(cmp);
    return vreinterpretq_m128d_u32(vmvnq_u32(vandq_u32(cmp, swapped)));
#endif
}


FORCE_INLINE __m128d _mm_cmpneq_sd(__m128d a, __m128d b)
{
    return _mm_move_sd(a, _mm_cmpneq_pd(a, b));
}


FORCE_INLINE __m128d _mm_cmpnge_pd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_u64(veorq_u64(
        vcgeq_f64(vreinterpretq_f64_m128d(a), vreinterpretq_f64_m128d(b)),
        vdupq_n_u64(UINT64_MAX)));
#else
    double a0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    double a1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    double b0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    double b1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 1));
    uint64_t d[2];
    d[0] = !(a0 >= b0) ? ~UINT64_C(0) : UINT64_C(0);
    d[1] = !(a1 >= b1) ? ~UINT64_C(0) : UINT64_C(0);

    return vreinterpretq_m128d_u64(vld1q_u64(d));
#endif
}


FORCE_INLINE __m128d _mm_cmpnge_sd(__m128d a, __m128d b)
{
    return _mm_move_sd(a, _mm_cmpnge_pd(a, b));
}


FORCE_INLINE __m128d _mm_cmpngt_pd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_u64(veorq_u64(
        vcgtq_f64(vreinterpretq_f64_m128d(a), vreinterpretq_f64_m128d(b)),
        vdupq_n_u64(UINT64_MAX)));
#else
    double a0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    double a1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    double b0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    double b1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 1));
    uint64_t d[2];
    d[0] = !(a0 > b0) ? ~UINT64_C(0) : UINT64_C(0);
    d[1] = !(a1 > b1) ? ~UINT64_C(0) : UINT64_C(0);

    return vreinterpretq_m128d_u64(vld1q_u64(d));
#endif
}


FORCE_INLINE __m128d _mm_cmpngt_sd(__m128d a, __m128d b)
{
    return _mm_move_sd(a, _mm_cmpngt_pd(a, b));
}


FORCE_INLINE __m128d _mm_cmpnle_pd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_u64(veorq_u64(
        vcleq_f64(vreinterpretq_f64_m128d(a), vreinterpretq_f64_m128d(b)),
        vdupq_n_u64(UINT64_MAX)));
#else
    double a0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    double a1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    double b0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    double b1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 1));
    uint64_t d[2];
    d[0] = !(a0 <= b0) ? ~UINT64_C(0) : UINT64_C(0);
    d[1] = !(a1 <= b1) ? ~UINT64_C(0) : UINT64_C(0);

    return vreinterpretq_m128d_u64(vld1q_u64(d));
#endif
}


FORCE_INLINE __m128d _mm_cmpnle_sd(__m128d a, __m128d b)
{
    return _mm_move_sd(a, _mm_cmpnle_pd(a, b));
}


FORCE_INLINE __m128d _mm_cmpnlt_pd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_u64(veorq_u64(
        vcltq_f64(vreinterpretq_f64_m128d(a), vreinterpretq_f64_m128d(b)),
        vdupq_n_u64(UINT64_MAX)));
#else
    double a0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    double a1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    double b0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    double b1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 1));
    uint64_t d[2];
    d[0] = !(a0 < b0) ? ~UINT64_C(0) : UINT64_C(0);
    d[1] = !(a1 < b1) ? ~UINT64_C(0) : UINT64_C(0);

    return vreinterpretq_m128d_u64(vld1q_u64(d));
#endif
}


FORCE_INLINE __m128d _mm_cmpnlt_sd(__m128d a, __m128d b)
{
    return _mm_move_sd(a, _mm_cmpnlt_pd(a, b));
}


FORCE_INLINE __m128d _mm_cmpord_pd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)

    uint64x2_t not_nan_a =
        vceqq_f64(vreinterpretq_f64_m128d(a), vreinterpretq_f64_m128d(a));
    uint64x2_t not_nan_b =
        vceqq_f64(vreinterpretq_f64_m128d(b), vreinterpretq_f64_m128d(b));
    return vreinterpretq_m128d_u64(vandq_u64(not_nan_a, not_nan_b));
#else
    double a0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    double a1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    double b0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    double b1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 1));
    uint64_t d[2];
    d[0] = (a0 == a0 && b0 == b0) ? ~UINT64_C(0) : UINT64_C(0);
    d[1] = (a1 == a1 && b1 == b1) ? ~UINT64_C(0) : UINT64_C(0);

    return vreinterpretq_m128d_u64(vld1q_u64(d));
#endif
}


FORCE_INLINE __m128d _mm_cmpord_sd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return _mm_move_sd(a, _mm_cmpord_pd(a, b));
#else
    double a0, b0;
    a0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    uint64_t a1 = vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1);
    b0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    uint64_t d[2];
    d[0] = (a0 == a0 && b0 == b0) ? ~UINT64_C(0) : UINT64_C(0);
    d[1] = a1;

    return vreinterpretq_m128d_u64(vld1q_u64(d));
#endif
}


FORCE_INLINE __m128d _mm_cmpunord_pd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)

    uint64x2_t not_nan_a =
        vceqq_f64(vreinterpretq_f64_m128d(a), vreinterpretq_f64_m128d(a));
    uint64x2_t not_nan_b =
        vceqq_f64(vreinterpretq_f64_m128d(b), vreinterpretq_f64_m128d(b));
    return vreinterpretq_m128d_s32(
        vmvnq_s32(vreinterpretq_s32_u64(vandq_u64(not_nan_a, not_nan_b))));
#else
    double a0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    double a1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    double b0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    double b1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 1));
    uint64_t d[2];
    d[0] = (a0 == a0 && b0 == b0) ? UINT64_C(0) : ~UINT64_C(0);
    d[1] = (a1 == a1 && b1 == b1) ? UINT64_C(0) : ~UINT64_C(0);

    return vreinterpretq_m128d_u64(vld1q_u64(d));
#endif
}


FORCE_INLINE __m128d _mm_cmpunord_sd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return _mm_move_sd(a, _mm_cmpunord_pd(a, b));
#else
    double a0, b0;
    a0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    uint64_t a1 = vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1);
    b0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    uint64_t d[2];
    d[0] = (a0 == a0 && b0 == b0) ? UINT64_C(0) : ~UINT64_C(0);
    d[1] = a1;

    return vreinterpretq_m128d_u64(vld1q_u64(d));
#endif
}


FORCE_INLINE int _mm_comige_sd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vgetq_lane_u64(vcgeq_f64(a, b), 0) & 0x1;
#else
    double a0, b0;
    a0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    b0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    return a0 >= b0;
#endif
}


FORCE_INLINE int _mm_comigt_sd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vgetq_lane_u64(vcgtq_f64(a, b), 0) & 0x1;
#else
    double a0, b0;
    a0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    b0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));

    return a0 > b0;
#endif
}


FORCE_INLINE int _mm_comile_sd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vgetq_lane_u64(vcleq_f64(a, b), 0) & 0x1;
#else
    double a0, b0;
    a0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    b0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));

    return a0 <= b0;
#endif
}


FORCE_INLINE int _mm_comilt_sd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vgetq_lane_u64(vcltq_f64(a, b), 0) & 0x1;
#else
    double a0, b0;
    a0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    b0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));

    return a0 < b0;
#endif
}


FORCE_INLINE int _mm_comieq_sd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vgetq_lane_u64(vceqq_f64(a, b), 0) & 0x1;
#else
    uint32x4_t a_not_nan =
        vceqq_u32(vreinterpretq_u32_m128d(a), vreinterpretq_u32_m128d(a));
    uint32x4_t b_not_nan =
        vceqq_u32(vreinterpretq_u32_m128d(b), vreinterpretq_u32_m128d(b));
    uint32x4_t a_and_b_not_nan = vandq_u32(a_not_nan, b_not_nan);
    uint32x4_t a_eq_b =
        vceqq_u32(vreinterpretq_u32_m128d(a), vreinterpretq_u32_m128d(b));
    uint64x2_t and_results = vandq_u64(vreinterpretq_u64_u32(a_and_b_not_nan),
                                       vreinterpretq_u64_u32(a_eq_b));
    return vgetq_lane_u64(and_results, 0) & 0x1;
#endif
}


FORCE_INLINE int _mm_comineq_sd(__m128d a, __m128d b)
{
    return !_mm_comieq_sd(a, b);
}


FORCE_INLINE __m128d _mm_cvtepi32_pd(__m128i a)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(
        vcvtq_f64_s64(vmovl_s32(vget_low_s32(vreinterpretq_s32_m128i(a)))));
#else
    double a0 = (double) vgetq_lane_s32(vreinterpretq_s32_m128i(a), 0);
    double a1 = (double) vgetq_lane_s32(vreinterpretq_s32_m128i(a), 1);
    return _mm_set_pd(a1, a0);
#endif
}


FORCE_INLINE __m128 _mm_cvtepi32_ps(__m128i a)
{
    return vreinterpretq_m128_f32(vcvtq_f32_s32(vreinterpretq_s32_m128i(a)));
}


FORCE_INLINE __m128i _mm_cvtpd_epi32(__m128d a)
{

#if defined(__ARM_FEATURE_FRINT) && !defined(__clang__)
    float64x2_t rounded = vrnd32xq_f64(vreinterpretq_f64_m128d(a));
    int64x2_t integers = vcvtq_s64_f64(rounded);
    return vreinterpretq_m128i_s32(
        vcombine_s32(vmovn_s64(integers), vdup_n_s32(0)));
#else
    __m128d rnd = _mm_round_pd(a, _MM_FROUND_CUR_DIRECTION);
    double d0, d1;
    d0 = sse2neon_recast_u64_f64(
        vgetq_lane_u64(vreinterpretq_u64_m128d(rnd), 0));
    d1 = sse2neon_recast_u64_f64(
        vgetq_lane_u64(vreinterpretq_u64_m128d(rnd), 1));
    return _mm_set_epi32(0, 0, (int32_t) d1, (int32_t) d0);
#endif
}


FORCE_INLINE __m64 _mm_cvtpd_pi32(__m128d a)
{
    __m128d rnd = _mm_round_pd(a, _MM_FROUND_CUR_DIRECTION);
    double d0, d1;
    d0 = sse2neon_recast_u64_f64(
        vgetq_lane_u64(vreinterpretq_u64_m128d(rnd), 0));
    d1 = sse2neon_recast_u64_f64(
        vgetq_lane_u64(vreinterpretq_u64_m128d(rnd), 1));
    int32_t ALIGN_STRUCT(16) data[2] = {(int32_t) d0, (int32_t) d1};
    return vreinterpret_m64_s32(vld1_s32(data));
}


FORCE_INLINE __m128 _mm_cvtpd_ps(__m128d a)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    float32x2_t tmp = vcvt_f32_f64(vreinterpretq_f64_m128d(a));
    return vreinterpretq_m128_f32(vcombine_f32(tmp, vdup_n_f32(0)));
#else
    double a0, a1;
    a0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    a1 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    return _mm_set_ps(0, 0, (float) a1, (float) a0);
#endif
}


FORCE_INLINE __m128d _mm_cvtpi32_pd(__m64 a)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(
        vcvtq_f64_s64(vmovl_s32(vreinterpret_s32_m64(a))));
#else
    double a0 = (double) vget_lane_s32(vreinterpret_s32_m64(a), 0);
    double a1 = (double) vget_lane_s32(vreinterpret_s32_m64(a), 1);
    return _mm_set_pd(a1, a0);
#endif
}


FORCE_INLINE __m128i _mm_cvtps_epi32(__m128 a)
{
#if defined(__ARM_FEATURE_FRINT)
    return vreinterpretq_m128i_s32(vcvtq_s32_f32(vrnd32xq_f32(a)));
#elif (defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)) || \
    defined(__ARM_FEATURE_DIRECTED_ROUNDING)
    switch (_MM_GET_ROUNDING_MODE()) {
    case _MM_ROUND_NEAREST:
        return vreinterpretq_m128i_s32(vcvtnq_s32_f32(a));
    case _MM_ROUND_DOWN:
        return vreinterpretq_m128i_s32(vcvtmq_s32_f32(a));
    case _MM_ROUND_UP:
        return vreinterpretq_m128i_s32(vcvtpq_s32_f32(a));
    default:
        return vreinterpretq_m128i_s32(vcvtq_s32_f32(a));
    }
#else
    float *f = (float *) &a;
    switch (_MM_GET_ROUNDING_MODE()) {
    case _MM_ROUND_NEAREST: {
        uint32x4_t signmask = vdupq_n_u32(0x80000000);
        float32x4_t half = vbslq_f32(signmask, vreinterpretq_f32_m128(a),
                                     vdupq_n_f32(0.5f));
        int32x4_t r_normal = vcvtq_s32_f32(vaddq_f32(
            vreinterpretq_f32_m128(a), half));
        int32x4_t r_trunc = vcvtq_s32_f32(
            vreinterpretq_f32_m128(a));
        int32x4_t plusone = vreinterpretq_s32_u32(vshrq_n_u32(
            vreinterpretq_u32_s32(vnegq_s32(r_trunc)), 31));
        int32x4_t r_even = vbicq_s32(vaddq_s32(r_trunc, plusone),
                                     vdupq_n_s32(1));
        float32x4_t delta = vsubq_f32(
            vreinterpretq_f32_m128(a),
            vcvtq_f32_s32(r_trunc));
        uint32x4_t is_delta_half =
            vceqq_f32(delta, half);
        return vreinterpretq_m128i_s32(
            vbslq_s32(is_delta_half, r_even, r_normal));
    }
    case _MM_ROUND_DOWN:
        return _mm_set_epi32(floorf(f[3]), floorf(f[2]), floorf(f[1]),
                             floorf(f[0]));
    case _MM_ROUND_UP:
        return _mm_set_epi32(ceilf(f[3]), ceilf(f[2]), ceilf(f[1]),
                             ceilf(f[0]));
    default:
        return _mm_set_epi32((int32_t) f[3], (int32_t) f[2], (int32_t) f[1],
                             (int32_t) f[0]);
    }
#endif
}


FORCE_INLINE __m128d _mm_cvtps_pd(__m128 a)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(
        vcvt_f64_f32(vget_low_f32(vreinterpretq_f32_m128(a))));
#else
    double a0 = (double) vgetq_lane_f32(vreinterpretq_f32_m128(a), 0);
    double a1 = (double) vgetq_lane_f32(vreinterpretq_f32_m128(a), 1);
    return _mm_set_pd(a1, a0);
#endif
}


FORCE_INLINE double _mm_cvtsd_f64(__m128d a)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return (double) vgetq_lane_f64(vreinterpretq_f64_m128d(a), 0);
#else
    double _a =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    return _a;
#endif
}


FORCE_INLINE int32_t _mm_cvtsd_si32(__m128d a)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return (int32_t) vgetq_lane_f64(vrndiq_f64(vreinterpretq_f64_m128d(a)), 0);
#else
    __m128d rnd = _mm_round_pd(a, _MM_FROUND_CUR_DIRECTION);
    double ret = sse2neon_recast_u64_f64(
        vgetq_lane_u64(vreinterpretq_u64_m128d(rnd), 0));
    return (int32_t) ret;
#endif
}


FORCE_INLINE int64_t _mm_cvtsd_si64(__m128d a)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return (int64_t) vgetq_lane_f64(vrndiq_f64(vreinterpretq_f64_m128d(a)), 0);
#else
    __m128d rnd = _mm_round_pd(a, _MM_FROUND_CUR_DIRECTION);
    double ret = sse2neon_recast_u64_f64(
        vgetq_lane_u64(vreinterpretq_u64_m128d(rnd), 0));
    return (int64_t) ret;
#endif
}


#define _mm_cvtsd_si64x _mm_cvtsd_si64


FORCE_INLINE __m128 _mm_cvtsd_ss(__m128 a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128_f32(vsetq_lane_f32(
        vget_lane_f32(vcvt_f32_f64(vreinterpretq_f64_m128d(b)), 0),
        vreinterpretq_f32_m128(a), 0));
#else
    double b0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    return vreinterpretq_m128_f32(
        vsetq_lane_f32((float) b0, vreinterpretq_f32_m128(a), 0));
#endif
}


FORCE_INLINE int _mm_cvtsi128_si32(__m128i a)
{
    return vgetq_lane_s32(vreinterpretq_s32_m128i(a), 0);
}


FORCE_INLINE int64_t _mm_cvtsi128_si64(__m128i a)
{
    return vgetq_lane_s64(vreinterpretq_s64_m128i(a), 0);
}


#define _mm_cvtsi128_si64x(a) _mm_cvtsi128_si64(a)


FORCE_INLINE __m128d _mm_cvtsi32_sd(__m128d a, int32_t b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(
        vsetq_lane_f64((double) b, vreinterpretq_f64_m128d(a), 0));
#else
    int64_t _b = sse2neon_recast_f64_s64((double) b);
    return vreinterpretq_m128d_s64(
        vsetq_lane_s64(_b, vreinterpretq_s64_m128d(a), 0));
#endif
}


#define _mm_cvtsi128_si64x(a) _mm_cvtsi128_si64(a)


FORCE_INLINE __m128i _mm_cvtsi32_si128(int a)
{
    return vreinterpretq_m128i_s32(vsetq_lane_s32(a, vdupq_n_s32(0), 0));
}


FORCE_INLINE __m128d _mm_cvtsi64_sd(__m128d a, int64_t b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(
        vsetq_lane_f64((double) b, vreinterpretq_f64_m128d(a), 0));
#else
    int64_t _b = sse2neon_recast_f64_s64((double) b);
    return vreinterpretq_m128d_s64(
        vsetq_lane_s64(_b, vreinterpretq_s64_m128d(a), 0));
#endif
}


FORCE_INLINE __m128i _mm_cvtsi64_si128(int64_t a)
{
    return vreinterpretq_m128i_s64(vsetq_lane_s64(a, vdupq_n_s64(0), 0));
}


#define _mm_cvtsi64x_si128(a) _mm_cvtsi64_si128(a)


#define _mm_cvtsi64x_sd(a, b) _mm_cvtsi64_sd(a, b)


FORCE_INLINE __m128d _mm_cvtss_sd(__m128d a, __m128 b)
{
    double d = (double) vgetq_lane_f32(vreinterpretq_f32_m128(b), 0);
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(
        vsetq_lane_f64(d, vreinterpretq_f64_m128d(a), 0));
#else
    return vreinterpretq_m128d_s64(vsetq_lane_s64(
        sse2neon_recast_f64_s64(d), vreinterpretq_s64_m128d(a), 0));
#endif
}


FORCE_INLINE __m128i _mm_cvttpd_epi32(__m128d a)
{
    double a0, a1;
    a0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    a1 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    return _mm_set_epi32(0, 0, (int32_t) a1, (int32_t) a0);
}


FORCE_INLINE __m64 _mm_cvttpd_pi32(__m128d a)
{
    double a0, a1;
    a0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    a1 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    int32_t ALIGN_STRUCT(16) data[2] = {(int32_t) a0, (int32_t) a1};
    return vreinterpret_m64_s32(vld1_s32(data));
}


FORCE_INLINE __m128i _mm_cvttps_epi32(__m128 a)
{
    return vreinterpretq_m128i_s32(vcvtq_s32_f32(vreinterpretq_f32_m128(a)));
}


FORCE_INLINE int32_t _mm_cvttsd_si32(__m128d a)
{
    double _a =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    return (int32_t) _a;
}


FORCE_INLINE int64_t _mm_cvttsd_si64(__m128d a)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vgetq_lane_s64(vcvtq_s64_f64(vreinterpretq_f64_m128d(a)), 0);
#else
    double _a =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    return (int64_t) _a;
#endif
}


#define _mm_cvttsd_si64x(a) _mm_cvttsd_si64(a)


FORCE_INLINE __m128d _mm_div_pd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(
        vdivq_f64(vreinterpretq_f64_m128d(a), vreinterpretq_f64_m128d(b)));
#else
    double a0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    double a1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    double b0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    double b1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 1));
    double c[2];
    c[0] = a0 / b0;
    c[1] = a1 / b1;
    return vld1q_f32((float32_t *) c);
#endif
}


FORCE_INLINE __m128d _mm_div_sd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    float64x2_t tmp =
        vdivq_f64(vreinterpretq_f64_m128d(a), vreinterpretq_f64_m128d(b));
    return vreinterpretq_m128d_f64(
        vsetq_lane_f64(vgetq_lane_f64(vreinterpretq_f64_m128d(a), 1), tmp, 1));
#else
    return _mm_move_sd(a, _mm_div_pd(a, b));
#endif
}


#define _mm_extract_epi16(a, imm) \
    vgetq_lane_u16(vreinterpretq_u16_m128i(a), (imm))


#define _mm_insert_epi16(a, b, imm) \
    vreinterpretq_m128i_s16(        \
        vsetq_lane_s16((b), vreinterpretq_s16_m128i(a), (imm)))


FORCE_INLINE __m128d _mm_load_pd(const double *p)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(vld1q_f64(p));
#else
    const float *fp = (const float *) p;
    float ALIGN_STRUCT(16) data[4] = {fp[0], fp[1], fp[2], fp[3]};
    return vreinterpretq_m128d_f32(vld1q_f32(data));
#endif
}


#define _mm_load_pd1 _mm_load1_pd


FORCE_INLINE __m128d _mm_load_sd(const double *p)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(vsetq_lane_f64(*p, vdupq_n_f64(0), 0));
#else
    const float *fp = (const float *) p;
    float ALIGN_STRUCT(16) data[4] = {fp[0], fp[1], 0, 0};
    return vreinterpretq_m128d_f32(vld1q_f32(data));
#endif
}


FORCE_INLINE __m128i _mm_load_si128(const __m128i *p)
{
    return vreinterpretq_m128i_s32(vld1q_s32((const int32_t *) p));
}


FORCE_INLINE __m128d _mm_load1_pd(const double *p)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(vld1q_dup_f64(p));
#else
    return vreinterpretq_m128d_s64(vdupq_n_s64(*(const int64_t *) p));
#endif
}


FORCE_INLINE __m128d _mm_loadh_pd(__m128d a, const double *p)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(
        vcombine_f64(vget_low_f64(vreinterpretq_f64_m128d(a)), vld1_f64(p)));
#else
    return vreinterpretq_m128d_f32(vcombine_f32(
        vget_low_f32(vreinterpretq_f32_m128d(a)), vld1_f32((const float *) p)));
#endif
}


FORCE_INLINE __m128i _mm_loadl_epi64(__m128i const *p)
{


    return vreinterpretq_m128i_s32(
        vcombine_s32(vld1_s32((int32_t const *) p), vcreate_s32(0)));
}


FORCE_INLINE __m128d _mm_loadl_pd(__m128d a, const double *p)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(
        vcombine_f64(vld1_f64(p), vget_high_f64(vreinterpretq_f64_m128d(a))));
#else
    return vreinterpretq_m128d_f32(
        vcombine_f32(vld1_f32((const float *) p),
                     vget_high_f32(vreinterpretq_f32_m128d(a))));
#endif
}


FORCE_INLINE __m128d _mm_loadr_pd(const double *p)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    float64x2_t v = vld1q_f64(p);
    return vreinterpretq_m128d_f64(vextq_f64(v, v, 1));
#else
    int64x2_t v = vld1q_s64((const int64_t *) p);
    return vreinterpretq_m128d_s64(vextq_s64(v, v, 1));
#endif
}


FORCE_INLINE __m128d _mm_loadu_pd(const double *p)
{
    return _mm_load_pd(p);
}


FORCE_INLINE __m128i _mm_loadu_si128(const __m128i *p)
{
    return vreinterpretq_m128i_s32(vld1q_s32((const unaligned_int32_t *) p));
}


FORCE_INLINE __m128i _mm_loadu_si32(const void *p)
{
    return vreinterpretq_m128i_s32(
        vsetq_lane_s32(*(const unaligned_int32_t *) p, vdupq_n_s32(0), 0));
}


FORCE_INLINE __m128i _mm_madd_epi16(__m128i a, __m128i b)
{
    int32x4_t low = vmull_s16(vget_low_s16(vreinterpretq_s16_m128i(a)),
                              vget_low_s16(vreinterpretq_s16_m128i(b)));
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    int32x4_t high =
        vmull_high_s16(vreinterpretq_s16_m128i(a), vreinterpretq_s16_m128i(b));

    return vreinterpretq_m128i_s32(vpaddq_s32(low, high));
#else
    int32x4_t high = vmull_s16(vget_high_s16(vreinterpretq_s16_m128i(a)),
                               vget_high_s16(vreinterpretq_s16_m128i(b)));

    int32x2_t low_sum = vpadd_s32(vget_low_s32(low), vget_high_s32(low));
    int32x2_t high_sum = vpadd_s32(vget_low_s32(high), vget_high_s32(high));

    return vreinterpretq_m128i_s32(vcombine_s32(low_sum, high_sum));
#endif
}


FORCE_INLINE void _mm_maskmoveu_si128(__m128i a, __m128i mask, char *mem_addr)
{
    int8x16_t shr_mask = vshrq_n_s8(vreinterpretq_s8_m128i(mask), 7);
    __m128 b = _mm_load_ps((const float *) mem_addr);
    int8x16_t masked =
        vbslq_s8(vreinterpretq_u8_s8(shr_mask), vreinterpretq_s8_m128i(a),
                 vreinterpretq_s8_m128(b));
    vst1q_s8((int8_t *) mem_addr, masked);
}


FORCE_INLINE __m128i _mm_max_epi16(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s16(
        vmaxq_s16(vreinterpretq_s16_m128i(a), vreinterpretq_s16_m128i(b)));
}


FORCE_INLINE __m128i _mm_max_epu8(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_u8(
        vmaxq_u8(vreinterpretq_u8_m128i(a), vreinterpretq_u8_m128i(b)));
}


FORCE_INLINE __m128d _mm_max_pd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
#if SSE2NEON_PRECISE_MINMAX
    float64x2_t _a = vreinterpretq_f64_m128d(a);
    float64x2_t _b = vreinterpretq_f64_m128d(b);
    return vreinterpretq_m128d_f64(vbslq_f64(vcgtq_f64(_a, _b), _a, _b));
#else
    return vreinterpretq_m128d_f64(
        vmaxq_f64(vreinterpretq_f64_m128d(a), vreinterpretq_f64_m128d(b)));
#endif
#else
    double a0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    double a1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    double b0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    double b1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 1));
    int64_t d[2];
    d[0] = a0 > b0 ? sse2neon_recast_f64_s64(a0) : sse2neon_recast_f64_s64(b0);
    d[1] = a1 > b1 ? sse2neon_recast_f64_s64(a1) : sse2neon_recast_f64_s64(b1);

    return vreinterpretq_m128d_s64(vld1q_s64(d));
#endif
}


FORCE_INLINE __m128d _mm_max_sd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return _mm_move_sd(a, _mm_max_pd(a, b));
#else
    double a0, a1, b0;
    a0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    a1 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    b0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    double c[2] = {a0 > b0 ? a0 : b0, a1};
    return vreinterpretq_m128d_f32(vld1q_f32((float32_t *) c));
#endif
}


FORCE_INLINE __m128i _mm_min_epi16(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s16(
        vminq_s16(vreinterpretq_s16_m128i(a), vreinterpretq_s16_m128i(b)));
}


FORCE_INLINE __m128i _mm_min_epu8(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_u8(
        vminq_u8(vreinterpretq_u8_m128i(a), vreinterpretq_u8_m128i(b)));
}


FORCE_INLINE __m128d _mm_min_pd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
#if SSE2NEON_PRECISE_MINMAX
    float64x2_t _a = vreinterpretq_f64_m128d(a);
    float64x2_t _b = vreinterpretq_f64_m128d(b);
    return vreinterpretq_m128d_f64(vbslq_f64(vcltq_f64(_a, _b), _a, _b));
#else
    return vreinterpretq_m128d_f64(
        vminq_f64(vreinterpretq_f64_m128d(a), vreinterpretq_f64_m128d(b)));
#endif
#else
    double a0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    double a1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    double b0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    double b1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 1));
    int64_t d[2];
    d[0] = a0 < b0 ? sse2neon_recast_f64_s64(a0) : sse2neon_recast_f64_s64(b0);
    d[1] = a1 < b1 ? sse2neon_recast_f64_s64(a1) : sse2neon_recast_f64_s64(b1);
    return vreinterpretq_m128d_s64(vld1q_s64(d));
#endif
}


FORCE_INLINE __m128d _mm_min_sd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return _mm_move_sd(a, _mm_min_pd(a, b));
#else
    double a0, a1, b0;
    a0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    a1 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    b0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    double c[2] = {a0 < b0 ? a0 : b0, a1};
    return vreinterpretq_m128d_f32(vld1q_f32((float32_t *) c));
#endif
}


FORCE_INLINE __m128i _mm_move_epi64(__m128i a)
{
    return vreinterpretq_m128i_s64(
        vsetq_lane_s64(0, vreinterpretq_s64_m128i(a), 1));
}


FORCE_INLINE __m128d _mm_move_sd(__m128d a, __m128d b)
{
    return vreinterpretq_m128d_f32(
        vcombine_f32(vget_low_f32(vreinterpretq_f32_m128d(b)),
                     vget_high_f32(vreinterpretq_f32_m128d(a))));
}


FORCE_INLINE int _mm_movemask_epi8(__m128i a)
{


    uint8x16_t input = vreinterpretq_u8_m128i(a);


    uint8x16_t msbs = vshrq_n_u8(input, 7);


    uint64x2_t bits = vreinterpretq_u64_u8(msbs);


    bits = vsraq_n_u64(bits, bits, 7);


    bits = vsraq_n_u64(bits, bits, 14);


    bits = vsraq_n_u64(bits, bits, 28);


    uint8x16_t output = vreinterpretq_u8_u64(bits);


    int low = vgetq_lane_u8(output, 0);
    int high = vgetq_lane_u8(output, 8);

    return (high << 8) | low;
}


FORCE_INLINE int _mm_movemask_pd(__m128d a)
{
    uint64x2_t input = vreinterpretq_u64_m128d(a);
    uint64x2_t high_bits = vshrq_n_u64(input, 63);
    return (int) (vgetq_lane_u64(high_bits, 0) |
                  (vgetq_lane_u64(high_bits, 1) << 1));
}


FORCE_INLINE __m64 _mm_movepi64_pi64(__m128i a)
{
    return vreinterpret_m64_s64(vget_low_s64(vreinterpretq_s64_m128i(a)));
}


FORCE_INLINE __m128i _mm_movpi64_epi64(__m64 a)
{
    return vreinterpretq_m128i_s64(
        vcombine_s64(vreinterpret_s64_m64(a), vdup_n_s64(0)));
}


FORCE_INLINE __m128i _mm_mul_epu32(__m128i a, __m128i b)
{

    uint32x2_t a_lo = vmovn_u64(vreinterpretq_u64_m128i(a));
    uint32x2_t b_lo = vmovn_u64(vreinterpretq_u64_m128i(b));
    return vreinterpretq_m128i_u64(vmull_u32(a_lo, b_lo));
}


FORCE_INLINE __m128d _mm_mul_pd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(
        vmulq_f64(vreinterpretq_f64_m128d(a), vreinterpretq_f64_m128d(b)));
#else
    double a0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    double a1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    double b0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    double b1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 1));
    double c[2];
    c[0] = a0 * b0;
    c[1] = a1 * b1;
    return vld1q_f32((float32_t *) c);
#endif
}


FORCE_INLINE __m128d _mm_mul_sd(__m128d a, __m128d b)
{
    return _mm_move_sd(a, _mm_mul_pd(a, b));
}


FORCE_INLINE __m64 _mm_mul_su32(__m64 a, __m64 b)
{
    return vreinterpret_m64_u64(vget_low_u64(
        vmull_u32(vreinterpret_u32_m64(a), vreinterpret_u32_m64(b))));
}


FORCE_INLINE __m128i _mm_mulhi_epi16(__m128i a, __m128i b)
{


    int16x4_t a3210 = vget_low_s16(vreinterpretq_s16_m128i(a));
    int16x4_t b3210 = vget_low_s16(vreinterpretq_s16_m128i(b));
    int32x4_t ab3210 = vmull_s16(a3210, b3210);
    int16x4_t a7654 = vget_high_s16(vreinterpretq_s16_m128i(a));
    int16x4_t b7654 = vget_high_s16(vreinterpretq_s16_m128i(b));
    int32x4_t ab7654 = vmull_s16(a7654, b7654);
    uint16x8x2_t r =
        vuzpq_u16(vreinterpretq_u16_s32(ab3210), vreinterpretq_u16_s32(ab7654));
    return vreinterpretq_m128i_u16(r.val[1]);
}


FORCE_INLINE __m128i _mm_mulhi_epu16(__m128i a, __m128i b)
{
    uint16x4_t a3210 = vget_low_u16(vreinterpretq_u16_m128i(a));
    uint16x4_t b3210 = vget_low_u16(vreinterpretq_u16_m128i(b));
    uint32x4_t ab3210 = vmull_u16(a3210, b3210);
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    uint32x4_t ab7654 =
        vmull_high_u16(vreinterpretq_u16_m128i(a), vreinterpretq_u16_m128i(b));
    uint16x8_t r = vuzp2q_u16(vreinterpretq_u16_u32(ab3210),
                              vreinterpretq_u16_u32(ab7654));
    return vreinterpretq_m128i_u16(r);
#else
    uint16x4_t a7654 = vget_high_u16(vreinterpretq_u16_m128i(a));
    uint16x4_t b7654 = vget_high_u16(vreinterpretq_u16_m128i(b));
    uint32x4_t ab7654 = vmull_u16(a7654, b7654);
    uint16x8x2_t r =
        vuzpq_u16(vreinterpretq_u16_u32(ab3210), vreinterpretq_u16_u32(ab7654));
    return vreinterpretq_m128i_u16(r.val[1]);
#endif
}


FORCE_INLINE __m128i _mm_mullo_epi16(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s16(
        vmulq_s16(vreinterpretq_s16_m128i(a), vreinterpretq_s16_m128i(b)));
}


FORCE_INLINE __m128d _mm_or_pd(__m128d a, __m128d b)
{
    return vreinterpretq_m128d_s64(
        vorrq_s64(vreinterpretq_s64_m128d(a), vreinterpretq_s64_m128d(b)));
}


FORCE_INLINE __m128i _mm_or_si128(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s32(
        vorrq_s32(vreinterpretq_s32_m128i(a), vreinterpretq_s32_m128i(b)));
}


FORCE_INLINE __m128i _mm_packs_epi16(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s8(
        vcombine_s8(vqmovn_s16(vreinterpretq_s16_m128i(a)),
                    vqmovn_s16(vreinterpretq_s16_m128i(b))));
}


FORCE_INLINE __m128i _mm_packs_epi32(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s16(
        vcombine_s16(vqmovn_s32(vreinterpretq_s32_m128i(a)),
                     vqmovn_s32(vreinterpretq_s32_m128i(b))));
}


FORCE_INLINE __m128i _mm_packus_epi16(const __m128i a, const __m128i b)
{
    return vreinterpretq_m128i_u8(
        vcombine_u8(vqmovun_s16(vreinterpretq_s16_m128i(a)),
                    vqmovun_s16(vreinterpretq_s16_m128i(b))));
}


FORCE_INLINE void _mm_pause(void)
{
#if defined(_MSC_VER) && !defined(__clang__)
    __isb(_ARM64_BARRIER_SY);
#else
    __asm__ __volatile__("isb\n");
#endif
}


FORCE_INLINE __m128i _mm_sad_epu8(__m128i a, __m128i b)
{
    uint16x8_t t = vpaddlq_u8(vabdq_u8((uint8x16_t) a, (uint8x16_t) b));
    return vreinterpretq_m128i_u64(vpaddlq_u32(vpaddlq_u16(t)));
}


FORCE_INLINE __m128i _mm_set_epi16(short i7,
                                   short i6,
                                   short i5,
                                   short i4,
                                   short i3,
                                   short i2,
                                   short i1,
                                   short i0)
{
    int16_t ALIGN_STRUCT(16) data[8] = {i0, i1, i2, i3, i4, i5, i6, i7};
    return vreinterpretq_m128i_s16(vld1q_s16(data));
}


FORCE_INLINE __m128i _mm_set_epi32(int i3, int i2, int i1, int i0)
{
    int32_t ALIGN_STRUCT(16) data[4] = {i0, i1, i2, i3};
    return vreinterpretq_m128i_s32(vld1q_s32(data));
}


FORCE_INLINE __m128i _mm_set_epi64(__m64 i1, __m64 i2)
{
    return _mm_set_epi64x(vget_lane_s64(i1, 0), vget_lane_s64(i2, 0));
}


FORCE_INLINE __m128i _mm_set_epi64x(int64_t i1, int64_t i2)
{
    return vreinterpretq_m128i_s64(
        vcombine_s64(vcreate_s64(i2), vcreate_s64(i1)));
}


FORCE_INLINE __m128i _mm_set_epi8(signed char b15,
                                  signed char b14,
                                  signed char b13,
                                  signed char b12,
                                  signed char b11,
                                  signed char b10,
                                  signed char b9,
                                  signed char b8,
                                  signed char b7,
                                  signed char b6,
                                  signed char b5,
                                  signed char b4,
                                  signed char b3,
                                  signed char b2,
                                  signed char b1,
                                  signed char b0)
{
    int8_t ALIGN_STRUCT(16) data[16] = {
        (int8_t) b0,  (int8_t) b1,  (int8_t) b2,  (int8_t) b3,
        (int8_t) b4,  (int8_t) b5,  (int8_t) b6,  (int8_t) b7,
        (int8_t) b8,  (int8_t) b9,  (int8_t) b10, (int8_t) b11,
        (int8_t) b12, (int8_t) b13, (int8_t) b14, (int8_t) b15};
    return (__m128i) vld1q_s8(data);
}


FORCE_INLINE __m128d _mm_set_pd(double e1, double e0)
{
    double ALIGN_STRUCT(16) data[2] = {e0, e1};
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(vld1q_f64((float64_t *) data));
#else
    return vreinterpretq_m128d_f32(vld1q_f32((float32_t *) data));
#endif
}


#define _mm_set_pd1 _mm_set1_pd


FORCE_INLINE __m128d _mm_set_sd(double a)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(vsetq_lane_f64(a, vdupq_n_f64(0), 0));
#else
    return _mm_set_pd(0, a);
#endif
}


FORCE_INLINE __m128i _mm_set1_epi16(short w)
{
    return vreinterpretq_m128i_s16(vdupq_n_s16(w));
}


FORCE_INLINE __m128i _mm_set1_epi32(int _i)
{
    return vreinterpretq_m128i_s32(vdupq_n_s32(_i));
}


FORCE_INLINE __m128i _mm_set1_epi64(__m64 _i)
{
    return vreinterpretq_m128i_s64(vdupq_lane_s64(_i, 0));
}


FORCE_INLINE __m128i _mm_set1_epi64x(int64_t _i)
{
    return vreinterpretq_m128i_s64(vdupq_n_s64(_i));
}


FORCE_INLINE __m128i _mm_set1_epi8(signed char w)
{
    return vreinterpretq_m128i_s8(vdupq_n_s8(w));
}


FORCE_INLINE __m128d _mm_set1_pd(double d)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(vdupq_n_f64(d));
#else
    int64_t _d = sse2neon_recast_f64_s64(d);
    return vreinterpretq_m128d_s64(vdupq_n_s64(_d));
#endif
}


FORCE_INLINE __m128i _mm_setr_epi16(short w0,
                                    short w1,
                                    short w2,
                                    short w3,
                                    short w4,
                                    short w5,
                                    short w6,
                                    short w7)
{
    int16_t ALIGN_STRUCT(16) data[8] = {w0, w1, w2, w3, w4, w5, w6, w7};
    return vreinterpretq_m128i_s16(vld1q_s16((int16_t *) data));
}


FORCE_INLINE __m128i _mm_setr_epi32(int i3, int i2, int i1, int i0)
{
    int32_t ALIGN_STRUCT(16) data[4] = {i3, i2, i1, i0};
    return vreinterpretq_m128i_s32(vld1q_s32(data));
}


FORCE_INLINE __m128i _mm_setr_epi64(__m64 e1, __m64 e0)
{
    return vreinterpretq_m128i_s64(vcombine_s64(e1, e0));
}


FORCE_INLINE __m128i _mm_setr_epi8(signed char b0,
                                   signed char b1,
                                   signed char b2,
                                   signed char b3,
                                   signed char b4,
                                   signed char b5,
                                   signed char b6,
                                   signed char b7,
                                   signed char b8,
                                   signed char b9,
                                   signed char b10,
                                   signed char b11,
                                   signed char b12,
                                   signed char b13,
                                   signed char b14,
                                   signed char b15)
{
    int8_t ALIGN_STRUCT(16) data[16] = {
        (int8_t) b0,  (int8_t) b1,  (int8_t) b2,  (int8_t) b3,
        (int8_t) b4,  (int8_t) b5,  (int8_t) b6,  (int8_t) b7,
        (int8_t) b8,  (int8_t) b9,  (int8_t) b10, (int8_t) b11,
        (int8_t) b12, (int8_t) b13, (int8_t) b14, (int8_t) b15};
    return (__m128i) vld1q_s8(data);
}


FORCE_INLINE __m128d _mm_setr_pd(double e1, double e0)
{
    return _mm_set_pd(e0, e1);
}


FORCE_INLINE __m128d _mm_setzero_pd(void)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(vdupq_n_f64(0));
#else
    return vreinterpretq_m128d_f32(vdupq_n_f32(0));
#endif
}


FORCE_INLINE __m128i _mm_setzero_si128(void)
{
    return vreinterpretq_m128i_s32(vdupq_n_s32(0));
}


#if defined(_sse2neon_shuffle)
#define _mm_shuffle_epi32(a, imm)                                            \
    __extension__({                                                          \
        int32x4_t _input = vreinterpretq_s32_m128i(a);                       \
        int32x4_t _shuf =                                                    \
            vshuffleq_s32(_input, _input, (imm) & (0x3), ((imm) >> 2) & 0x3, \
                          ((imm) >> 4) & 0x3, ((imm) >> 6) & 0x3);           \
        vreinterpretq_m128i_s32(_shuf);                                      \
    })
#else
#define _mm_shuffle_epi32(a, imm)                           \
    _sse2neon_define1(                                      \
        __m128i, a, __m128i ret; switch (imm) {             \
            case _MM_SHUFFLE(1, 0, 3, 2):                   \
                ret = _mm_shuffle_epi_1032(_a);             \
                break;                                      \
            case _MM_SHUFFLE(2, 3, 0, 1):                   \
                ret = _mm_shuffle_epi_2301(_a);             \
                break;                                      \
            case _MM_SHUFFLE(0, 3, 2, 1):                   \
                ret = _mm_shuffle_epi_0321(_a);             \
                break;                                      \
            case _MM_SHUFFLE(2, 1, 0, 3):                   \
                ret = _mm_shuffle_epi_2103(_a);             \
                break;                                      \
            case _MM_SHUFFLE(1, 0, 1, 0):                   \
                ret = _mm_shuffle_epi_1010(_a);             \
                break;                                      \
            case _MM_SHUFFLE(1, 0, 0, 1):                   \
                ret = _mm_shuffle_epi_1001(_a);             \
                break;                                      \
            case _MM_SHUFFLE(0, 1, 0, 1):                   \
                ret = _mm_shuffle_epi_0101(_a);             \
                break;                                      \
            case _MM_SHUFFLE(2, 2, 1, 1):                   \
                ret = _mm_shuffle_epi_2211(_a);             \
                break;                                      \
            case _MM_SHUFFLE(0, 1, 2, 2):                   \
                ret = _mm_shuffle_epi_0122(_a);             \
                break;                                      \
            case _MM_SHUFFLE(3, 3, 3, 2):                   \
                ret = _mm_shuffle_epi_3332(_a);             \
                break;                                      \
            case _MM_SHUFFLE(0, 0, 0, 0):                   \
                ret = _mm_shuffle_epi32_splat(_a, 0);       \
                break;                                      \
            case _MM_SHUFFLE(1, 1, 1, 1):                   \
                ret = _mm_shuffle_epi32_splat(_a, 1);       \
                break;                                      \
            case _MM_SHUFFLE(2, 2, 2, 2):                   \
                ret = _mm_shuffle_epi32_splat(_a, 2);       \
                break;                                      \
            case _MM_SHUFFLE(3, 3, 3, 3):                   \
                ret = _mm_shuffle_epi32_splat(_a, 3);       \
                break;                                      \
            default:                                        \
                ret = _mm_shuffle_epi32_default(_a, (imm)); \
                break;                                      \
        } _sse2neon_return(ret);)
#endif


#ifdef _sse2neon_shuffle
#define _mm_shuffle_pd(a, b, imm8)                                            \
    vreinterpretq_m128d_s64(                                                  \
        vshuffleq_s64(vreinterpretq_s64_m128d(a), vreinterpretq_s64_m128d(b), \
                      (imm8) & 0x1, (((imm8) & 0x2) >> 1) + 2))
#else
#define _mm_shuffle_pd(a, b, imm8)                                       \
    _mm_castsi128_pd(_mm_set_epi64x(                                     \
        vgetq_lane_s64(vreinterpretq_s64_m128d(b), ((imm8) & 0x2) >> 1), \
        vgetq_lane_s64(vreinterpretq_s64_m128d(a), (imm8) & 0x1)))
#endif


#if defined(_sse2neon_shuffle)
#define _mm_shufflehi_epi16(a, imm)                                           \
    __extension__({                                                           \
        int16x8_t _input = vreinterpretq_s16_m128i(a);                        \
        int16x8_t _shuf =                                                     \
            vshuffleq_s16(_input, _input, 0, 1, 2, 3, ((imm) & (0x3)) + 4,    \
                          (((imm) >> 2) & 0x3) + 4, (((imm) >> 4) & 0x3) + 4, \
                          (((imm) >> 6) & 0x3) + 4);                          \
        vreinterpretq_m128i_s16(_shuf);                                       \
    })
#else
#define _mm_shufflehi_epi16(a, imm) _mm_shufflehi_epi16_function((a), (imm))
#endif


#if defined(_sse2neon_shuffle)
#define _mm_shufflelo_epi16(a, imm)                                  \
    __extension__({                                                  \
        int16x8_t _input = vreinterpretq_s16_m128i(a);               \
        int16x8_t _shuf = vshuffleq_s16(                             \
            _input, _input, ((imm) & (0x3)), (((imm) >> 2) & 0x3),   \
            (((imm) >> 4) & 0x3), (((imm) >> 6) & 0x3), 4, 5, 6, 7); \
        vreinterpretq_m128i_s16(_shuf);                              \
    })
#else
#define _mm_shufflelo_epi16(a, imm) _mm_shufflelo_epi16_function((a), (imm))
#endif


FORCE_INLINE __m128i _mm_sll_epi16(__m128i a, __m128i count)
{
    uint64_t c = vreinterpretq_nth_u64_m128i(count, 0);
    if (_sse2neon_unlikely(c & ~15))
        return _mm_setzero_si128();

    int16x8_t vc = vdupq_n_s16((int16_t) c);
    return vreinterpretq_m128i_s16(vshlq_s16(vreinterpretq_s16_m128i(a), vc));
}


FORCE_INLINE __m128i _mm_sll_epi32(__m128i a, __m128i count)
{
    uint64_t c = vreinterpretq_nth_u64_m128i(count, 0);
    if (_sse2neon_unlikely(c & ~31))
        return _mm_setzero_si128();

    int32x4_t vc = vdupq_n_s32((int32_t) c);
    return vreinterpretq_m128i_s32(vshlq_s32(vreinterpretq_s32_m128i(a), vc));
}


FORCE_INLINE __m128i _mm_sll_epi64(__m128i a, __m128i count)
{
    uint64_t c = vreinterpretq_nth_u64_m128i(count, 0);
    if (_sse2neon_unlikely(c & ~63))
        return _mm_setzero_si128();

    int64x2_t vc = vdupq_n_s64((int64_t) c);
    return vreinterpretq_m128i_s64(vshlq_s64(vreinterpretq_s64_m128i(a), vc));
}


FORCE_INLINE __m128i _mm_slli_epi16(__m128i a, int imm)
{
    if (_sse2neon_unlikely(imm & ~15))
        return _mm_setzero_si128();
    return vreinterpretq_m128i_s16(
        vshlq_s16(vreinterpretq_s16_m128i(a), vdupq_n_s16((int16_t) imm)));
}


FORCE_INLINE __m128i _mm_slli_epi32(__m128i a, int imm)
{
    if (_sse2neon_unlikely(imm & ~31))
        return _mm_setzero_si128();
    return vreinterpretq_m128i_s32(
        vshlq_s32(vreinterpretq_s32_m128i(a), vdupq_n_s32(imm)));
}


FORCE_INLINE __m128i _mm_slli_epi64(__m128i a, int imm)
{
    if (_sse2neon_unlikely(imm & ~63))
        return _mm_setzero_si128();
    return vreinterpretq_m128i_s64(
        vshlq_s64(vreinterpretq_s64_m128i(a), vdupq_n_s64(imm)));
}


#define _mm_slli_si128(a, imm)                                                \
    _sse2neon_define1(                                                        \
        __m128i, a, int8x16_t ret;                                            \
        if (_sse2neon_unlikely((imm) == 0)) ret = vreinterpretq_s8_m128i(_a); \
        else if (_sse2neon_unlikely((imm) & ~15)) ret = vdupq_n_s8(0);        \
        else ret = vextq_s8(vdupq_n_s8(0), vreinterpretq_s8_m128i(_a),        \
                            (((imm) <= 0 || (imm) > 15) ? 0 : (16 - (imm)))); \
        _sse2neon_return(vreinterpretq_m128i_s8(ret));)


FORCE_INLINE __m128d _mm_sqrt_pd(__m128d a)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(vsqrtq_f64(vreinterpretq_f64_m128d(a)));
#else
    double a0, a1;
    a0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    a1 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    double _a0 = sqrt(a0);
    double _a1 = sqrt(a1);
    return _mm_set_pd(_a1, _a0);
#endif
}


FORCE_INLINE __m128d _mm_sqrt_sd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return _mm_move_sd(a, _mm_sqrt_pd(b));
#else
    double _a, _b;
    _a = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    _b = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    return _mm_set_pd(_a, sqrt(_b));
#endif
}


FORCE_INLINE __m128i _mm_sra_epi16(__m128i a, __m128i count)
{
    int64_t c = vgetq_lane_s64(count, 0);
    if (_sse2neon_unlikely(c & ~15))
        return _mm_cmplt_epi16(a, _mm_setzero_si128());
    return vreinterpretq_m128i_s16(
        vshlq_s16((int16x8_t) a, vdupq_n_s16((int16_t) -c)));
}


FORCE_INLINE __m128i _mm_sra_epi32(__m128i a, __m128i count)
{
    int64_t c = vgetq_lane_s64(count, 0);
    if (_sse2neon_unlikely(c & ~31))
        return _mm_cmplt_epi32(a, _mm_setzero_si128());
    return vreinterpretq_m128i_s32(
        vshlq_s32((int32x4_t) a, vdupq_n_s32((int) -c)));
}


FORCE_INLINE __m128i _mm_srai_epi16(__m128i a, int imm)
{
    const int16_t count = (imm & ~15) ? 15 : (int16_t) imm;
    return (__m128i) vshlq_s16((int16x8_t) a, vdupq_n_s16(-count));
}


#define _mm_srai_epi32(a, imm)                                                \
    _sse2neon_define0(                                                        \
        __m128i, a, __m128i ret; if (_sse2neon_unlikely((imm) == 0)) {        \
            ret = _a;                                                         \
        } else if (_sse2neon_likely(0 < (imm) && (imm) < 32)) {               \
            ret = vreinterpretq_m128i_s32(                                    \
                vshlq_s32(vreinterpretq_s32_m128i(_a), vdupq_n_s32(-(imm)))); \
        } else {                                                              \
            ret = vreinterpretq_m128i_s32(                                    \
                vshrq_n_s32(vreinterpretq_s32_m128i(_a), 31));                \
        } _sse2neon_return(ret);)


FORCE_INLINE __m128i _mm_srl_epi16(__m128i a, __m128i count)
{
    uint64_t c = vreinterpretq_nth_u64_m128i(count, 0);
    if (_sse2neon_unlikely(c & ~15))
        return _mm_setzero_si128();

    int16x8_t vc = vdupq_n_s16(-(int16_t) c);
    return vreinterpretq_m128i_u16(vshlq_u16(vreinterpretq_u16_m128i(a), vc));
}


FORCE_INLINE __m128i _mm_srl_epi32(__m128i a, __m128i count)
{
    uint64_t c = vreinterpretq_nth_u64_m128i(count, 0);
    if (_sse2neon_unlikely(c & ~31))
        return _mm_setzero_si128();

    int32x4_t vc = vdupq_n_s32(-(int32_t) c);
    return vreinterpretq_m128i_u32(vshlq_u32(vreinterpretq_u32_m128i(a), vc));
}


FORCE_INLINE __m128i _mm_srl_epi64(__m128i a, __m128i count)
{
    uint64_t c = vreinterpretq_nth_u64_m128i(count, 0);
    if (_sse2neon_unlikely(c & ~63))
        return _mm_setzero_si128();

    int64x2_t vc = vdupq_n_s64(-(int64_t) c);
    return vreinterpretq_m128i_u64(vshlq_u64(vreinterpretq_u64_m128i(a), vc));
}


#define _mm_srli_epi16(a, imm)                                                 \
    _sse2neon_define0(                                                         \
        __m128i, a, __m128i ret; if (_sse2neon_unlikely((imm) & ~15)) {        \
            ret = _mm_setzero_si128();                                         \
        } else {                                                               \
            ret = vreinterpretq_m128i_u16(vshlq_u16(                           \
                vreinterpretq_u16_m128i(_a), vdupq_n_s16((int16_t) - (imm)))); \
        } _sse2neon_return(ret);)


#define _mm_srli_epi32(a, imm)                                                \
    _sse2neon_define0(                                                        \
        __m128i, a, __m128i ret; if (_sse2neon_unlikely((imm) & ~31)) {       \
            ret = _mm_setzero_si128();                                        \
        } else {                                                              \
            ret = vreinterpretq_m128i_u32(                                    \
                vshlq_u32(vreinterpretq_u32_m128i(_a), vdupq_n_s32(-(imm)))); \
        } _sse2neon_return(ret);)


#define _mm_srli_epi64(a, imm)                                                \
    _sse2neon_define0(                                                        \
        __m128i, a, __m128i ret; if (_sse2neon_unlikely((imm) & ~63)) {       \
            ret = _mm_setzero_si128();                                        \
        } else {                                                              \
            ret = vreinterpretq_m128i_u64(                                    \
                vshlq_u64(vreinterpretq_u64_m128i(_a), vdupq_n_s64(-(imm)))); \
        } _sse2neon_return(ret);)


#define _mm_srli_si128(a, imm)                                         \
    _sse2neon_define1(                                                 \
        __m128i, a, int8x16_t ret;                                     \
        if (_sse2neon_unlikely((imm) & ~15)) ret = vdupq_n_s8(0);      \
        else ret = vextq_s8(vreinterpretq_s8_m128i(_a), vdupq_n_s8(0), \
                            ((imm) > 15 ? 0 : (imm)));                 \
        _sse2neon_return(vreinterpretq_m128i_s8(ret));)


FORCE_INLINE void _mm_store_pd(double *mem_addr, __m128d a)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    vst1q_f64((float64_t *) mem_addr, vreinterpretq_f64_m128d(a));
#else
    vst1q_f32((float32_t *) mem_addr, vreinterpretq_f32_m128d(a));
#endif
}


FORCE_INLINE void _mm_store_pd1(double *mem_addr, __m128d a)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    float64x1_t a_low = vget_low_f64(vreinterpretq_f64_m128d(a));
    vst1q_f64((float64_t *) mem_addr,
              vreinterpretq_f64_m128d(vcombine_f64(a_low, a_low)));
#else
    float32x2_t a_low = vget_low_f32(vreinterpretq_f32_m128d(a));
    vst1q_f32((float32_t *) mem_addr,
              vreinterpretq_f32_m128d(vcombine_f32(a_low, a_low)));
#endif
}


FORCE_INLINE void _mm_store_sd(double *mem_addr, __m128d a)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    vst1_f64((float64_t *) mem_addr, vget_low_f64(vreinterpretq_f64_m128d(a)));
#else
    vst1_u64((uint64_t *) mem_addr, vget_low_u64(vreinterpretq_u64_m128d(a)));
#endif
}


FORCE_INLINE void _mm_store_si128(__m128i *p, __m128i a)
{
    vst1q_s32((int32_t *) p, vreinterpretq_s32_m128i(a));
}


#define _mm_store1_pd _mm_store_pd1


FORCE_INLINE void _mm_storeh_pd(double *mem_addr, __m128d a)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    vst1_f64((float64_t *) mem_addr, vget_high_f64(vreinterpretq_f64_m128d(a)));
#else
    vst1_f32((float32_t *) mem_addr, vget_high_f32(vreinterpretq_f32_m128d(a)));
#endif
}


FORCE_INLINE void _mm_storel_epi64(__m128i *a, __m128i b)
{
    vst1_u64((uint64_t *) a, vget_low_u64(vreinterpretq_u64_m128i(b)));
}


FORCE_INLINE void _mm_storel_pd(double *mem_addr, __m128d a)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    vst1_f64((float64_t *) mem_addr, vget_low_f64(vreinterpretq_f64_m128d(a)));
#else
    vst1_f32((float32_t *) mem_addr, vget_low_f32(vreinterpretq_f32_m128d(a)));
#endif
}


FORCE_INLINE void _mm_storer_pd(double *mem_addr, __m128d a)
{
    float32x4_t f = vreinterpretq_f32_m128d(a);
    _mm_store_pd(mem_addr, vreinterpretq_m128d_f32(vextq_f32(f, f, 2)));
}


FORCE_INLINE void _mm_storeu_pd(double *mem_addr, __m128d a)
{
    _mm_store_pd(mem_addr, a);
}


FORCE_INLINE void _mm_storeu_si128(__m128i *p, __m128i a)
{
    vst1q_s32((int32_t *) p, vreinterpretq_s32_m128i(a));
}


FORCE_INLINE void _mm_storeu_si32(void *p, __m128i a)
{
    vst1q_lane_s32((int32_t *) p, vreinterpretq_s32_m128i(a), 0);
}


FORCE_INLINE void _mm_stream_pd(double *p, __m128d a)
{
#if __has_builtin(__builtin_nontemporal_store)
    __builtin_nontemporal_store(a, (__m128d *) p);
#elif defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    vst1q_f64(p, vreinterpretq_f64_m128d(a));
#else
    vst1q_s64((int64_t *) p, vreinterpretq_s64_m128d(a));
#endif
}


FORCE_INLINE void _mm_stream_si128(__m128i *p, __m128i a)
{
#if __has_builtin(__builtin_nontemporal_store)
    __builtin_nontemporal_store(a, p);
#else
    vst1q_s64((int64_t *) p, vreinterpretq_s64_m128i(a));
#endif
}


FORCE_INLINE void _mm_stream_si32(int *p, int a)
{
    vst1q_lane_s32((int32_t *) p, vdupq_n_s32(a), 0);
}


FORCE_INLINE void _mm_stream_si64(__int64 *p, __int64 a)
{
    vst1_s64((int64_t *) p, vdup_n_s64((int64_t) a));
}


FORCE_INLINE __m128i _mm_sub_epi16(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s16(
        vsubq_s16(vreinterpretq_s16_m128i(a), vreinterpretq_s16_m128i(b)));
}


FORCE_INLINE __m128i _mm_sub_epi32(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s32(
        vsubq_s32(vreinterpretq_s32_m128i(a), vreinterpretq_s32_m128i(b)));
}


FORCE_INLINE __m128i _mm_sub_epi64(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s64(
        vsubq_s64(vreinterpretq_s64_m128i(a), vreinterpretq_s64_m128i(b)));
}


FORCE_INLINE __m128i _mm_sub_epi8(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s8(
        vsubq_s8(vreinterpretq_s8_m128i(a), vreinterpretq_s8_m128i(b)));
}


FORCE_INLINE __m128d _mm_sub_pd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(
        vsubq_f64(vreinterpretq_f64_m128d(a), vreinterpretq_f64_m128d(b)));
#else
    double a0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    double a1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    double b0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    double b1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 1));
    double c[2];
    c[0] = a0 - b0;
    c[1] = a1 - b1;
    return vld1q_f32((float32_t *) c);
#endif
}


FORCE_INLINE __m128d _mm_sub_sd(__m128d a, __m128d b)
{
    return _mm_move_sd(a, _mm_sub_pd(a, b));
}


FORCE_INLINE __m64 _mm_sub_si64(__m64 a, __m64 b)
{
    return vreinterpret_m64_s64(
        vsub_s64(vreinterpret_s64_m64(a), vreinterpret_s64_m64(b)));
}


FORCE_INLINE __m128i _mm_subs_epi16(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s16(
        vqsubq_s16(vreinterpretq_s16_m128i(a), vreinterpretq_s16_m128i(b)));
}


FORCE_INLINE __m128i _mm_subs_epi8(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s8(
        vqsubq_s8(vreinterpretq_s8_m128i(a), vreinterpretq_s8_m128i(b)));
}


FORCE_INLINE __m128i _mm_subs_epu16(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_u16(
        vqsubq_u16(vreinterpretq_u16_m128i(a), vreinterpretq_u16_m128i(b)));
}


FORCE_INLINE __m128i _mm_subs_epu8(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_u8(
        vqsubq_u8(vreinterpretq_u8_m128i(a), vreinterpretq_u8_m128i(b)));
}

#define _mm_ucomieq_sd _mm_comieq_sd
#define _mm_ucomige_sd _mm_comige_sd
#define _mm_ucomigt_sd _mm_comigt_sd
#define _mm_ucomile_sd _mm_comile_sd
#define _mm_ucomilt_sd _mm_comilt_sd
#define _mm_ucomineq_sd _mm_comineq_sd


FORCE_INLINE __m128d _mm_undefined_pd(void)
{
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#endif
    __m128d a;
#if defined(_MSC_VER) && !defined(__clang__)
    a = _mm_setzero_pd();
#endif
    return a;
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
}


FORCE_INLINE __m128i _mm_unpackhi_epi16(__m128i a, __m128i b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128i_s16(
        vzip2q_s16(vreinterpretq_s16_m128i(a), vreinterpretq_s16_m128i(b)));
#else
    int16x4_t a1 = vget_high_s16(vreinterpretq_s16_m128i(a));
    int16x4_t b1 = vget_high_s16(vreinterpretq_s16_m128i(b));
    int16x4x2_t result = vzip_s16(a1, b1);
    return vreinterpretq_m128i_s16(vcombine_s16(result.val[0], result.val[1]));
#endif
}


FORCE_INLINE __m128i _mm_unpackhi_epi32(__m128i a, __m128i b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128i_s32(
        vzip2q_s32(vreinterpretq_s32_m128i(a), vreinterpretq_s32_m128i(b)));
#else
    int32x2_t a1 = vget_high_s32(vreinterpretq_s32_m128i(a));
    int32x2_t b1 = vget_high_s32(vreinterpretq_s32_m128i(b));
    int32x2x2_t result = vzip_s32(a1, b1);
    return vreinterpretq_m128i_s32(vcombine_s32(result.val[0], result.val[1]));
#endif
}


FORCE_INLINE __m128i _mm_unpackhi_epi64(__m128i a, __m128i b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128i_s64(
        vzip2q_s64(vreinterpretq_s64_m128i(a), vreinterpretq_s64_m128i(b)));
#else
    int64x1_t a_h = vget_high_s64(vreinterpretq_s64_m128i(a));
    int64x1_t b_h = vget_high_s64(vreinterpretq_s64_m128i(b));
    return vreinterpretq_m128i_s64(vcombine_s64(a_h, b_h));
#endif
}


FORCE_INLINE __m128i _mm_unpackhi_epi8(__m128i a, __m128i b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128i_s8(
        vzip2q_s8(vreinterpretq_s8_m128i(a), vreinterpretq_s8_m128i(b)));
#else
    int8x8_t a1 =
        vreinterpret_s8_s16(vget_high_s16(vreinterpretq_s16_m128i(a)));
    int8x8_t b1 =
        vreinterpret_s8_s16(vget_high_s16(vreinterpretq_s16_m128i(b)));
    int8x8x2_t result = vzip_s8(a1, b1);
    return vreinterpretq_m128i_s8(vcombine_s8(result.val[0], result.val[1]));
#endif
}


FORCE_INLINE __m128d _mm_unpackhi_pd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(
        vzip2q_f64(vreinterpretq_f64_m128d(a), vreinterpretq_f64_m128d(b)));
#else
    return vreinterpretq_m128d_s64(
        vcombine_s64(vget_high_s64(vreinterpretq_s64_m128d(a)),
                     vget_high_s64(vreinterpretq_s64_m128d(b))));
#endif
}


FORCE_INLINE __m128i _mm_unpacklo_epi16(__m128i a, __m128i b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128i_s16(
        vzip1q_s16(vreinterpretq_s16_m128i(a), vreinterpretq_s16_m128i(b)));
#else
    int16x4_t a1 = vget_low_s16(vreinterpretq_s16_m128i(a));
    int16x4_t b1 = vget_low_s16(vreinterpretq_s16_m128i(b));
    int16x4x2_t result = vzip_s16(a1, b1);
    return vreinterpretq_m128i_s16(vcombine_s16(result.val[0], result.val[1]));
#endif
}


FORCE_INLINE __m128i _mm_unpacklo_epi32(__m128i a, __m128i b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128i_s32(
        vzip1q_s32(vreinterpretq_s32_m128i(a), vreinterpretq_s32_m128i(b)));
#else
    int32x2_t a1 = vget_low_s32(vreinterpretq_s32_m128i(a));
    int32x2_t b1 = vget_low_s32(vreinterpretq_s32_m128i(b));
    int32x2x2_t result = vzip_s32(a1, b1);
    return vreinterpretq_m128i_s32(vcombine_s32(result.val[0], result.val[1]));
#endif
}


FORCE_INLINE __m128i _mm_unpacklo_epi64(__m128i a, __m128i b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128i_s64(
        vzip1q_s64(vreinterpretq_s64_m128i(a), vreinterpretq_s64_m128i(b)));
#else
    int64x1_t a_l = vget_low_s64(vreinterpretq_s64_m128i(a));
    int64x1_t b_l = vget_low_s64(vreinterpretq_s64_m128i(b));
    return vreinterpretq_m128i_s64(vcombine_s64(a_l, b_l));
#endif
}


FORCE_INLINE __m128i _mm_unpacklo_epi8(__m128i a, __m128i b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128i_s8(
        vzip1q_s8(vreinterpretq_s8_m128i(a), vreinterpretq_s8_m128i(b)));
#else
    int8x8_t a1 = vreinterpret_s8_s16(vget_low_s16(vreinterpretq_s16_m128i(a)));
    int8x8_t b1 = vreinterpret_s8_s16(vget_low_s16(vreinterpretq_s16_m128i(b)));
    int8x8x2_t result = vzip_s8(a1, b1);
    return vreinterpretq_m128i_s8(vcombine_s8(result.val[0], result.val[1]));
#endif
}


FORCE_INLINE __m128d _mm_unpacklo_pd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(
        vzip1q_f64(vreinterpretq_f64_m128d(a), vreinterpretq_f64_m128d(b)));
#else
    return vreinterpretq_m128d_s64(
        vcombine_s64(vget_low_s64(vreinterpretq_s64_m128d(a)),
                     vget_low_s64(vreinterpretq_s64_m128d(b))));
#endif
}


FORCE_INLINE __m128d _mm_xor_pd(__m128d a, __m128d b)
{
    return vreinterpretq_m128d_s64(
        veorq_s64(vreinterpretq_s64_m128d(a), vreinterpretq_s64_m128d(b)));
}


FORCE_INLINE __m128i _mm_xor_si128(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s32(
        veorq_s32(vreinterpretq_s32_m128i(a), vreinterpretq_s32_m128i(b)));
}


FORCE_INLINE __m128d _mm_addsub_pd(__m128d a, __m128d b)
{
    _sse2neon_const __m128d mask = _mm_set_pd(1.0f, -1.0f);
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(vfmaq_f64(vreinterpretq_f64_m128d(a),
                                             vreinterpretq_f64_m128d(b),
                                             vreinterpretq_f64_m128d(mask)));
#else
    return _mm_add_pd(_mm_mul_pd(b, mask), a);
#endif
}


FORCE_INLINE __m128 _mm_addsub_ps(__m128 a, __m128 b)
{
    _sse2neon_const __m128 mask = _mm_setr_ps(-1.0f, 1.0f, -1.0f, 1.0f);
#if (defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)) || \
    defined(__ARM_FEATURE_FMA)
    return vreinterpretq_m128_f32(vfmaq_f32(vreinterpretq_f32_m128(a),
                                            vreinterpretq_f32_m128(mask),
                                            vreinterpretq_f32_m128(b)));
#else
    return _mm_add_ps(_mm_mul_ps(b, mask), a);
#endif
}


FORCE_INLINE __m128d _mm_hadd_pd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(
        vpaddq_f64(vreinterpretq_f64_m128d(a), vreinterpretq_f64_m128d(b)));
#else
    double a0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    double a1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    double b0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    double b1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 1));
    double c[] = {a0 + a1, b0 + b1};
    return vreinterpretq_m128d_u64(vld1q_u64((uint64_t *) c));
#endif
}


FORCE_INLINE __m128 _mm_hadd_ps(__m128 a, __m128 b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128_f32(
        vpaddq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(b)));
#else
    float32x2_t a10 = vget_low_f32(vreinterpretq_f32_m128(a));
    float32x2_t a32 = vget_high_f32(vreinterpretq_f32_m128(a));
    float32x2_t b10 = vget_low_f32(vreinterpretq_f32_m128(b));
    float32x2_t b32 = vget_high_f32(vreinterpretq_f32_m128(b));
    return vreinterpretq_m128_f32(
        vcombine_f32(vpadd_f32(a10, a32), vpadd_f32(b10, b32)));
#endif
}


FORCE_INLINE __m128d _mm_hsub_pd(__m128d a, __m128d b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    float64x2_t _a = vreinterpretq_f64_m128d(a);
    float64x2_t _b = vreinterpretq_f64_m128d(b);
    return vreinterpretq_m128d_f64(
        vsubq_f64(vuzp1q_f64(_a, _b), vuzp2q_f64(_a, _b)));
#else
    double a0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    double a1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    double b0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    double b1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 1));
    double c[] = {a0 - a1, b0 - b1};
    return vreinterpretq_m128d_u64(vld1q_u64((uint64_t *) c));
#endif
}


FORCE_INLINE __m128 _mm_hsub_ps(__m128 _a, __m128 _b)
{
    float32x4_t a = vreinterpretq_f32_m128(_a);
    float32x4_t b = vreinterpretq_f32_m128(_b);
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128_f32(
        vsubq_f32(vuzp1q_f32(a, b), vuzp2q_f32(a, b)));
#else
    float32x4x2_t c = vuzpq_f32(a, b);
    return vreinterpretq_m128_f32(vsubq_f32(c.val[0], c.val[1]));
#endif
}


#define _mm_lddqu_si128 _mm_loadu_si128


#define _mm_loaddup_pd _mm_load1_pd


FORCE_INLINE __m128d _mm_movedup_pd(__m128d a)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(
        vdupq_laneq_f64(vreinterpretq_f64_m128d(a), 0));
#else
    return vreinterpretq_m128d_u64(
        vdupq_n_u64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0)));
#endif
}


FORCE_INLINE __m128 _mm_movehdup_ps(__m128 a)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128_f32(
        vtrn2q_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(a)));
#elif defined(_sse2neon_shuffle)
    return vreinterpretq_m128_f32(vshuffleq_s32(
        vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(a), 1, 1, 3, 3));
#else
    float32_t a1 = vgetq_lane_f32(vreinterpretq_f32_m128(a), 1);
    float32_t a3 = vgetq_lane_f32(vreinterpretq_f32_m128(a), 3);
    float ALIGN_STRUCT(16) data[4] = {a1, a1, a3, a3};
    return vreinterpretq_m128_f32(vld1q_f32(data));
#endif
}


FORCE_INLINE __m128 _mm_moveldup_ps(__m128 a)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128_f32(
        vtrn1q_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(a)));
#elif defined(_sse2neon_shuffle)
    return vreinterpretq_m128_f32(vshuffleq_s32(
        vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(a), 0, 0, 2, 2));
#else
    float32_t a0 = vgetq_lane_f32(vreinterpretq_f32_m128(a), 0);
    float32_t a2 = vgetq_lane_f32(vreinterpretq_f32_m128(a), 2);
    float ALIGN_STRUCT(16) data[4] = {a0, a0, a2, a2};
    return vreinterpretq_m128_f32(vld1q_f32(data));
#endif
}


FORCE_INLINE __m128i _mm_abs_epi16(__m128i a)
{
    return vreinterpretq_m128i_s16(vabsq_s16(vreinterpretq_s16_m128i(a)));
}


FORCE_INLINE __m128i _mm_abs_epi32(__m128i a)
{
    return vreinterpretq_m128i_s32(vabsq_s32(vreinterpretq_s32_m128i(a)));
}


FORCE_INLINE __m128i _mm_abs_epi8(__m128i a)
{
    return vreinterpretq_m128i_s8(vabsq_s8(vreinterpretq_s8_m128i(a)));
}


FORCE_INLINE __m64 _mm_abs_pi16(__m64 a)
{
    return vreinterpret_m64_s16(vabs_s16(vreinterpret_s16_m64(a)));
}


FORCE_INLINE __m64 _mm_abs_pi32(__m64 a)
{
    return vreinterpret_m64_s32(vabs_s32(vreinterpret_s32_m64(a)));
}


FORCE_INLINE __m64 _mm_abs_pi8(__m64 a)
{
    return vreinterpret_m64_s8(vabs_s8(vreinterpret_s8_m64(a)));
}


#if defined(__GNUC__) && !defined(__clang__)
#define _mm_alignr_epi8(a, b, imm)                                 \
    __extension__({                                                \
        uint8x16_t _a = vreinterpretq_u8_m128i(a);                 \
        uint8x16_t _b = vreinterpretq_u8_m128i(b);                 \
        __m128i ret;                                               \
        if (_sse2neon_unlikely((imm) & ~31))                       \
            ret = vreinterpretq_m128i_u8(vdupq_n_u8(0));           \
        else if ((imm) >= 16)                                      \
            ret = _mm_srli_si128(a, (imm) >= 16 ? (imm) - 16 : 0); \
        else                                                       \
            ret = vreinterpretq_m128i_u8(                          \
                vextq_u8(_b, _a, (imm) < 16 ? (imm) : 0));         \
        ret;                                                       \
    })

#else
#define _mm_alignr_epi8(a, b, imm)                                  \
    _sse2neon_define2(                                              \
        __m128i, a, b, uint8x16_t __a = vreinterpretq_u8_m128i(_a); \
        uint8x16_t __b = vreinterpretq_u8_m128i(_b); __m128i ret;   \
        if (_sse2neon_unlikely((imm) & ~31)) ret =                  \
            vreinterpretq_m128i_u8(vdupq_n_u8(0));                  \
        else if ((imm) >= 16) ret =                                 \
            _mm_srli_si128(_a, (imm) >= 16 ? (imm) - 16 : 0);       \
        else ret = vreinterpretq_m128i_u8(                          \
            vextq_u8(__b, __a, (imm) < 16 ? (imm) : 0));            \
        _sse2neon_return(ret);)

#endif


#define _mm_alignr_pi8(a, b, imm)                                           \
    _sse2neon_define2(                                                      \
        __m64, a, b, __m64 ret; if (_sse2neon_unlikely((imm) >= 16)) {      \
            ret = vreinterpret_m64_s8(vdup_n_s8(0));                        \
        } else {                                                            \
            uint8x8_t tmp_low;                                              \
            uint8x8_t tmp_high;                                             \
            if ((imm) >= 8) {                                               \
                const int idx = (imm) - 8;                                  \
                tmp_low = vreinterpret_u8_m64(_a);                          \
                tmp_high = vdup_n_u8(0);                                    \
                ret = vreinterpret_m64_u8(vext_u8(tmp_low, tmp_high, idx)); \
            } else {                                                        \
                const int idx = (imm);                                      \
                tmp_low = vreinterpret_u8_m64(_b);                          \
                tmp_high = vreinterpret_u8_m64(_a);                         \
                ret = vreinterpret_m64_u8(vext_u8(tmp_low, tmp_high, idx)); \
            }                                                               \
        } _sse2neon_return(ret);)


FORCE_INLINE __m128i _mm_hadd_epi16(__m128i _a, __m128i _b)
{
    int16x8_t a = vreinterpretq_s16_m128i(_a);
    int16x8_t b = vreinterpretq_s16_m128i(_b);
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128i_s16(vpaddq_s16(a, b));
#else
    return vreinterpretq_m128i_s16(
        vcombine_s16(vpadd_s16(vget_low_s16(a), vget_high_s16(a)),
                     vpadd_s16(vget_low_s16(b), vget_high_s16(b))));
#endif
}


FORCE_INLINE __m128i _mm_hadd_epi32(__m128i _a, __m128i _b)
{
    int32x4_t a = vreinterpretq_s32_m128i(_a);
    int32x4_t b = vreinterpretq_s32_m128i(_b);
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128i_s32(vpaddq_s32(a, b));
#else
    return vreinterpretq_m128i_s32(
        vcombine_s32(vpadd_s32(vget_low_s32(a), vget_high_s32(a)),
                     vpadd_s32(vget_low_s32(b), vget_high_s32(b))));
#endif
}


FORCE_INLINE __m64 _mm_hadd_pi16(__m64 a, __m64 b)
{
    return vreinterpret_m64_s16(
        vpadd_s16(vreinterpret_s16_m64(a), vreinterpret_s16_m64(b)));
}


FORCE_INLINE __m64 _mm_hadd_pi32(__m64 a, __m64 b)
{
    return vreinterpret_m64_s32(
        vpadd_s32(vreinterpret_s32_m64(a), vreinterpret_s32_m64(b)));
}


FORCE_INLINE __m128i _mm_hadds_epi16(__m128i _a, __m128i _b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    int16x8_t a = vreinterpretq_s16_m128i(_a);
    int16x8_t b = vreinterpretq_s16_m128i(_b);
    return vreinterpretq_s64_s16(
        vqaddq_s16(vuzp1q_s16(a, b), vuzp2q_s16(a, b)));
#else
    int32x4_t a = vreinterpretq_s32_m128i(_a);
    int32x4_t b = vreinterpretq_s32_m128i(_b);


    int16x8_t ab0246 = vcombine_s16(vmovn_s32(a), vmovn_s32(b));
    int16x8_t ab1357 = vcombine_s16(vshrn_n_s32(a, 16), vshrn_n_s32(b, 16));

    return vreinterpretq_m128i_s16(vqaddq_s16(ab0246, ab1357));
#endif
}


FORCE_INLINE __m64 _mm_hadds_pi16(__m64 _a, __m64 _b)
{
    int16x4_t a = vreinterpret_s16_m64(_a);
    int16x4_t b = vreinterpret_s16_m64(_b);
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpret_s64_s16(vqadd_s16(vuzp1_s16(a, b), vuzp2_s16(a, b)));
#else
    int16x4x2_t res = vuzp_s16(a, b);
    return vreinterpret_s64_s16(vqadd_s16(res.val[0], res.val[1]));
#endif
}


FORCE_INLINE __m128i _mm_hsub_epi16(__m128i _a, __m128i _b)
{
    int16x8_t a = vreinterpretq_s16_m128i(_a);
    int16x8_t b = vreinterpretq_s16_m128i(_b);
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128i_s16(
        vsubq_s16(vuzp1q_s16(a, b), vuzp2q_s16(a, b)));
#else
    int16x8x2_t c = vuzpq_s16(a, b);
    return vreinterpretq_m128i_s16(vsubq_s16(c.val[0], c.val[1]));
#endif
}


FORCE_INLINE __m128i _mm_hsub_epi32(__m128i _a, __m128i _b)
{
    int32x4_t a = vreinterpretq_s32_m128i(_a);
    int32x4_t b = vreinterpretq_s32_m128i(_b);
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128i_s32(
        vsubq_s32(vuzp1q_s32(a, b), vuzp2q_s32(a, b)));
#else
    int32x4x2_t c = vuzpq_s32(a, b);
    return vreinterpretq_m128i_s32(vsubq_s32(c.val[0], c.val[1]));
#endif
}


FORCE_INLINE __m64 _mm_hsub_pi16(__m64 _a, __m64 _b)
{
    int16x4_t a = vreinterpret_s16_m64(_a);
    int16x4_t b = vreinterpret_s16_m64(_b);
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpret_m64_s16(vsub_s16(vuzp1_s16(a, b), vuzp2_s16(a, b)));
#else
    int16x4x2_t c = vuzp_s16(a, b);
    return vreinterpret_m64_s16(vsub_s16(c.val[0], c.val[1]));
#endif
}


FORCE_INLINE __m64 _mm_hsub_pi32(__m64 _a, __m64 _b)
{
    int32x2_t a = vreinterpret_s32_m64(_a);
    int32x2_t b = vreinterpret_s32_m64(_b);
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpret_m64_s32(vsub_s32(vuzp1_s32(a, b), vuzp2_s32(a, b)));
#else
    int32x2x2_t c = vuzp_s32(a, b);
    return vreinterpret_m64_s32(vsub_s32(c.val[0], c.val[1]));
#endif
}


FORCE_INLINE __m128i _mm_hsubs_epi16(__m128i _a, __m128i _b)
{
    int16x8_t a = vreinterpretq_s16_m128i(_a);
    int16x8_t b = vreinterpretq_s16_m128i(_b);
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128i_s16(
        vqsubq_s16(vuzp1q_s16(a, b), vuzp2q_s16(a, b)));
#else
    int16x8x2_t c = vuzpq_s16(a, b);
    return vreinterpretq_m128i_s16(vqsubq_s16(c.val[0], c.val[1]));
#endif
}


FORCE_INLINE __m64 _mm_hsubs_pi16(__m64 _a, __m64 _b)
{
    int16x4_t a = vreinterpret_s16_m64(_a);
    int16x4_t b = vreinterpret_s16_m64(_b);
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpret_m64_s16(vqsub_s16(vuzp1_s16(a, b), vuzp2_s16(a, b)));
#else
    int16x4x2_t c = vuzp_s16(a, b);
    return vreinterpret_m64_s16(vqsub_s16(c.val[0], c.val[1]));
#endif
}


FORCE_INLINE __m128i _mm_maddubs_epi16(__m128i _a, __m128i _b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    uint8x16_t a = vreinterpretq_u8_m128i(_a);
    int8x16_t b = vreinterpretq_s8_m128i(_b);
    int16x8_t tl = vmulq_s16(vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(a))),
                             vmovl_s8(vget_low_s8(b)));
    int16x8_t th = vmulq_s16(vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(a))),
                             vmovl_s8(vget_high_s8(b)));
    return vreinterpretq_m128i_s16(
        vqaddq_s16(vuzp1q_s16(tl, th), vuzp2q_s16(tl, th)));
#else


    uint16x8_t a = vreinterpretq_u16_m128i(_a);
    int16x8_t b = vreinterpretq_s16_m128i(_b);


    int16x8_t a_odd = vreinterpretq_s16_u16(vshrq_n_u16(a, 8));
    int16x8_t a_even = vreinterpretq_s16_u16(vbicq_u16(a, vdupq_n_u16(0xff00)));


    int16x8_t b_even = vshrq_n_s16(vshlq_n_s16(b, 8), 8);
    int16x8_t b_odd = vshrq_n_s16(b, 8);


    int16x8_t prod1 = vmulq_s16(a_even, b_even);
    int16x8_t prod2 = vmulq_s16(a_odd, b_odd);


    return vreinterpretq_m128i_s16(vqaddq_s16(prod1, prod2));
#endif
}


FORCE_INLINE __m64 _mm_maddubs_pi16(__m64 _a, __m64 _b)
{
    uint16x4_t a = vreinterpret_u16_m64(_a);
    int16x4_t b = vreinterpret_s16_m64(_b);


    int16x4_t a_odd = vreinterpret_s16_u16(vshr_n_u16(a, 8));
    int16x4_t a_even = vreinterpret_s16_u16(vand_u16(a, vdup_n_u16(0xff)));


    int16x4_t b_even = vshr_n_s16(vshl_n_s16(b, 8), 8);
    int16x4_t b_odd = vshr_n_s16(b, 8);


    int16x4_t prod1 = vmul_s16(a_even, b_even);
    int16x4_t prod2 = vmul_s16(a_odd, b_odd);


    return vreinterpret_m64_s16(vqadd_s16(prod1, prod2));
}


FORCE_INLINE __m128i _mm_mulhrs_epi16(__m128i a, __m128i b)
{


    int32x4_t mul_lo = vmull_s16(vget_low_s16(vreinterpretq_s16_m128i(a)),
                                 vget_low_s16(vreinterpretq_s16_m128i(b)));
    int32x4_t mul_hi = vmull_s16(vget_high_s16(vreinterpretq_s16_m128i(a)),
                                 vget_high_s16(vreinterpretq_s16_m128i(b)));


    int16x4_t narrow_lo = vrshrn_n_s32(mul_lo, 15);
    int16x4_t narrow_hi = vrshrn_n_s32(mul_hi, 15);


    return vreinterpretq_m128i_s16(vcombine_s16(narrow_lo, narrow_hi));
}


FORCE_INLINE __m64 _mm_mulhrs_pi16(__m64 a, __m64 b)
{
    int32x4_t mul_extend =
        vmull_s16((vreinterpret_s16_m64(a)), (vreinterpret_s16_m64(b)));


    return vreinterpret_m64_s16(vrshrn_n_s32(mul_extend, 15));
}


FORCE_INLINE __m128i _mm_shuffle_epi8(__m128i a, __m128i b)
{
    int8x16_t tbl = vreinterpretq_s8_m128i(a);
    uint8x16_t idx = vreinterpretq_u8_m128i(b);
    uint8x16_t idx_masked =
        vandq_u8(idx, vdupq_n_u8(0x8F));
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128i_s8(vqtbl1q_s8(tbl, idx_masked));
#elif defined(__GNUC__)
    int8x16_t ret;


    __asm__ __volatile__(
        "vtbl.8  %e[ret], {%e[tbl], %f[tbl]}, %e[idx]\n"
        "vtbl.8  %f[ret], {%e[tbl], %f[tbl]}, %f[idx]\n"
        : [ret] "=&w"(ret)
        : [tbl] "w"(tbl), [idx] "w"(idx_masked));
    return vreinterpretq_m128i_s8(ret);
#else

    int8x8x2_t a_split = {vget_low_s8(tbl), vget_high_s8(tbl)};
    return vreinterpretq_m128i_s8(
        vcombine_s8(vtbl2_s8(a_split, vget_low_u8(idx_masked)),
                    vtbl2_s8(a_split, vget_high_u8(idx_masked))));
#endif
}


FORCE_INLINE __m64 _mm_shuffle_pi8(__m64 a, __m64 b)
{
    const int8x8_t controlMask =
        vand_s8(vreinterpret_s8_m64(b), vdup_n_s8((int8_t) (0x1 << 7 | 0x07)));
    int8x8_t res = vtbl1_s8(vreinterpret_s8_m64(a), controlMask);
    return vreinterpret_m64_s8(res);
}


FORCE_INLINE __m128i _mm_sign_epi16(__m128i _a, __m128i _b)
{
    int16x8_t a = vreinterpretq_s16_m128i(_a);
    int16x8_t b = vreinterpretq_s16_m128i(_b);


    uint16x8_t ltMask = vreinterpretq_u16_s16(vshrq_n_s16(b, 15));

#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    int16x8_t zeroMask = vreinterpretq_s16_u16(vceqzq_s16(b));
#else
    int16x8_t zeroMask = vreinterpretq_s16_u16(vceqq_s16(b, vdupq_n_s16(0)));
#endif


    int16x8_t masked = vbslq_s16(ltMask, vnegq_s16(a), a);

    int16x8_t res = vbicq_s16(masked, zeroMask);
    return vreinterpretq_m128i_s16(res);
}


FORCE_INLINE __m128i _mm_sign_epi32(__m128i _a, __m128i _b)
{
    int32x4_t a = vreinterpretq_s32_m128i(_a);
    int32x4_t b = vreinterpretq_s32_m128i(_b);


    uint32x4_t ltMask = vreinterpretq_u32_s32(vshrq_n_s32(b, 31));


#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    int32x4_t zeroMask = vreinterpretq_s32_u32(vceqzq_s32(b));
#else
    int32x4_t zeroMask = vreinterpretq_s32_u32(vceqq_s32(b, vdupq_n_s32(0)));
#endif


    int32x4_t masked = vbslq_s32(ltMask, vnegq_s32(a), a);

    int32x4_t res = vbicq_s32(masked, zeroMask);
    return vreinterpretq_m128i_s32(res);
}


FORCE_INLINE __m128i _mm_sign_epi8(__m128i _a, __m128i _b)
{
    int8x16_t a = vreinterpretq_s8_m128i(_a);
    int8x16_t b = vreinterpretq_s8_m128i(_b);


    uint8x16_t ltMask = vreinterpretq_u8_s8(vshrq_n_s8(b, 7));


#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    int8x16_t zeroMask = vreinterpretq_s8_u8(vceqzq_s8(b));
#else
    int8x16_t zeroMask = vreinterpretq_s8_u8(vceqq_s8(b, vdupq_n_s8(0)));
#endif


    int8x16_t masked = vbslq_s8(ltMask, vnegq_s8(a), a);

    int8x16_t res = vbicq_s8(masked, zeroMask);

    return vreinterpretq_m128i_s8(res);
}


FORCE_INLINE __m64 _mm_sign_pi16(__m64 _a, __m64 _b)
{
    int16x4_t a = vreinterpret_s16_m64(_a);
    int16x4_t b = vreinterpret_s16_m64(_b);


    uint16x4_t ltMask = vreinterpret_u16_s16(vshr_n_s16(b, 15));


#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    int16x4_t zeroMask = vreinterpret_s16_u16(vceqz_s16(b));
#else
    int16x4_t zeroMask = vreinterpret_s16_u16(vceq_s16(b, vdup_n_s16(0)));
#endif


    int16x4_t masked = vbsl_s16(ltMask, vneg_s16(a), a);

    int16x4_t res = vbic_s16(masked, zeroMask);

    return vreinterpret_m64_s16(res);
}


FORCE_INLINE __m64 _mm_sign_pi32(__m64 _a, __m64 _b)
{
    int32x2_t a = vreinterpret_s32_m64(_a);
    int32x2_t b = vreinterpret_s32_m64(_b);


    uint32x2_t ltMask = vreinterpret_u32_s32(vshr_n_s32(b, 31));


#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    int32x2_t zeroMask = vreinterpret_s32_u32(vceqz_s32(b));
#else
    int32x2_t zeroMask = vreinterpret_s32_u32(vceq_s32(b, vdup_n_s32(0)));
#endif


    int32x2_t masked = vbsl_s32(ltMask, vneg_s32(a), a);

    int32x2_t res = vbic_s32(masked, zeroMask);

    return vreinterpret_m64_s32(res);
}


FORCE_INLINE __m64 _mm_sign_pi8(__m64 _a, __m64 _b)
{
    int8x8_t a = vreinterpret_s8_m64(_a);
    int8x8_t b = vreinterpret_s8_m64(_b);


    uint8x8_t ltMask = vreinterpret_u8_s8(vshr_n_s8(b, 7));


#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    int8x8_t zeroMask = vreinterpret_s8_u8(vceqz_s8(b));
#else
    int8x8_t zeroMask = vreinterpret_s8_u8(vceq_s8(b, vdup_n_s8(0)));
#endif


    int8x8_t masked = vbsl_s8(ltMask, vneg_s8(a), a);

    int8x8_t res = vbic_s8(masked, zeroMask);

    return vreinterpret_m64_s8(res);
}


#define _mm_blend_epi16(a, b, imm)                                      \
    _sse2neon_define2(                                                  \
        __m128i, a, b,                                                  \
        const uint16_t _mask[8] =                                       \
            _sse2neon_init(((imm) & (1 << 0)) ? (uint16_t) - 1 : 0x0,   \
                           ((imm) & (1 << 1)) ? (uint16_t) - 1 : 0x0,   \
                           ((imm) & (1 << 2)) ? (uint16_t) - 1 : 0x0,   \
                           ((imm) & (1 << 3)) ? (uint16_t) - 1 : 0x0,   \
                           ((imm) & (1 << 4)) ? (uint16_t) - 1 : 0x0,   \
                           ((imm) & (1 << 5)) ? (uint16_t) - 1 : 0x0,   \
                           ((imm) & (1 << 6)) ? (uint16_t) - 1 : 0x0,   \
                           ((imm) & (1 << 7)) ? (uint16_t) - 1 : 0x0);  \
        uint16x8_t _mask_vec = vld1q_u16(_mask);                        \
        uint16x8_t __a = vreinterpretq_u16_m128i(_a);                   \
        uint16x8_t __b = vreinterpretq_u16_m128i(_b); _sse2neon_return( \
            vreinterpretq_m128i_u16(vbslq_u16(_mask_vec, __b, __a)));)


#define _mm_blend_pd(a, b, imm)                                              \
    _sse2neon_define2(                                                       \
        __m128d, a, b,                                                       \
        const uint64_t _mask[2] =                                            \
            _sse2neon_init(((imm) & (1 << 0)) ? ~UINT64_C(0) : UINT64_C(0),  \
                           ((imm) & (1 << 1)) ? ~UINT64_C(0) : UINT64_C(0)); \
        uint64x2_t _mask_vec = vld1q_u64(_mask);                             \
        uint64x2_t __a = vreinterpretq_u64_m128d(_a);                        \
        uint64x2_t __b = vreinterpretq_u64_m128d(_b); _sse2neon_return(      \
            vreinterpretq_m128d_u64(vbslq_u64(_mask_vec, __b, __a)));)


FORCE_INLINE __m128 _mm_blend_ps(__m128 _a, __m128 _b, const char imm8)
{
    const uint32_t ALIGN_STRUCT(16) data[4] = {
        (imm8 & (1 << 0)) ? UINT32_MAX : 0, (imm8 & (1 << 1)) ? UINT32_MAX : 0,
        (imm8 & (1 << 2)) ? UINT32_MAX : 0, (imm8 & (1 << 3)) ? UINT32_MAX : 0};
    uint32x4_t mask = vld1q_u32(data);
    float32x4_t a = vreinterpretq_f32_m128(_a);
    float32x4_t b = vreinterpretq_f32_m128(_b);
    return vreinterpretq_m128_f32(vbslq_f32(mask, b, a));
}


FORCE_INLINE __m128i _mm_blendv_epi8(__m128i _a, __m128i _b, __m128i _mask)
{

    uint8x16_t mask =
        vreinterpretq_u8_s8(vshrq_n_s8(vreinterpretq_s8_m128i(_mask), 7));
    uint8x16_t a = vreinterpretq_u8_m128i(_a);
    uint8x16_t b = vreinterpretq_u8_m128i(_b);
    return vreinterpretq_m128i_u8(vbslq_u8(mask, b, a));
}


FORCE_INLINE __m128d _mm_blendv_pd(__m128d _a, __m128d _b, __m128d _mask)
{
    uint64x2_t mask =
        vreinterpretq_u64_s64(vshrq_n_s64(vreinterpretq_s64_m128d(_mask), 63));
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    float64x2_t a = vreinterpretq_f64_m128d(_a);
    float64x2_t b = vreinterpretq_f64_m128d(_b);
    return vreinterpretq_m128d_f64(vbslq_f64(mask, b, a));
#else
    uint64x2_t a = vreinterpretq_u64_m128d(_a);
    uint64x2_t b = vreinterpretq_u64_m128d(_b);
    return vreinterpretq_m128d_u64(vbslq_u64(mask, b, a));
#endif
}


FORCE_INLINE __m128 _mm_blendv_ps(__m128 _a, __m128 _b, __m128 _mask)
{

    uint32x4_t mask =
        vreinterpretq_u32_s32(vshrq_n_s32(vreinterpretq_s32_m128(_mask), 31));
    float32x4_t a = vreinterpretq_f32_m128(_a);
    float32x4_t b = vreinterpretq_f32_m128(_b);
    return vreinterpretq_m128_f32(vbslq_f32(mask, b, a));
}


FORCE_INLINE __m128d _mm_ceil_pd(__m128d a)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(vrndpq_f64(vreinterpretq_f64_m128d(a)));
#else
    double a0, a1;
    a0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    a1 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    return _mm_set_pd(ceil(a1), ceil(a0));
#endif
}


FORCE_INLINE __m128 _mm_ceil_ps(__m128 a)
{
#if (defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)) || \
    defined(__ARM_FEATURE_DIRECTED_ROUNDING)
    return vreinterpretq_m128_f32(vrndpq_f32(vreinterpretq_f32_m128(a)));
#else
    float *f = (float *) &a;
    return _mm_set_ps(ceilf(f[3]), ceilf(f[2]), ceilf(f[1]), ceilf(f[0]));
#endif
}


FORCE_INLINE __m128d _mm_ceil_sd(__m128d a, __m128d b)
{
    return _mm_move_sd(a, _mm_ceil_pd(b));
}


FORCE_INLINE __m128 _mm_ceil_ss(__m128 a, __m128 b)
{
    return _mm_move_ss(a, _mm_ceil_ps(b));
}


FORCE_INLINE __m128i _mm_cmpeq_epi64(__m128i a, __m128i b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128i_u64(
        vceqq_u64(vreinterpretq_u64_m128i(a), vreinterpretq_u64_m128i(b)));
#else


    uint32x4_t cmp =
        vceqq_u32(vreinterpretq_u32_m128i(a), vreinterpretq_u32_m128i(b));
    uint32x4_t swapped = vrev64q_u32(cmp);
    return vreinterpretq_m128i_u32(vandq_u32(cmp, swapped));
#endif
}


FORCE_INLINE __m128i _mm_cvtepi16_epi32(__m128i a)
{
    return vreinterpretq_m128i_s32(
        vmovl_s16(vget_low_s16(vreinterpretq_s16_m128i(a))));
}


FORCE_INLINE __m128i _mm_cvtepi16_epi64(__m128i a)
{
    int16x8_t s16x8 = vreinterpretq_s16_m128i(a);
    int32x4_t s32x4 = vmovl_s16(vget_low_s16(s16x8));
    int64x2_t s64x2 = vmovl_s32(vget_low_s32(s32x4));
    return vreinterpretq_m128i_s64(s64x2);
}


FORCE_INLINE __m128i _mm_cvtepi32_epi64(__m128i a)
{
    return vreinterpretq_m128i_s64(
        vmovl_s32(vget_low_s32(vreinterpretq_s32_m128i(a))));
}


FORCE_INLINE __m128i _mm_cvtepi8_epi16(__m128i a)
{
    int8x16_t s8x16 = vreinterpretq_s8_m128i(a);
    int16x8_t s16x8 = vmovl_s8(vget_low_s8(s8x16));
    return vreinterpretq_m128i_s16(s16x8);
}


FORCE_INLINE __m128i _mm_cvtepi8_epi32(__m128i a)
{
    int8x16_t s8x16 = vreinterpretq_s8_m128i(a);
    int16x8_t s16x8 = vmovl_s8(vget_low_s8(s8x16));
    int32x4_t s32x4 = vmovl_s16(vget_low_s16(s16x8));
    return vreinterpretq_m128i_s32(s32x4);
}


FORCE_INLINE __m128i _mm_cvtepi8_epi64(__m128i a)
{
    int8x16_t s8x16 = vreinterpretq_s8_m128i(a);
    int16x8_t s16x8 = vmovl_s8(vget_low_s8(s8x16));
    int32x4_t s32x4 = vmovl_s16(vget_low_s16(s16x8));
    int64x2_t s64x2 = vmovl_s32(vget_low_s32(s32x4));
    return vreinterpretq_m128i_s64(s64x2);
}


FORCE_INLINE __m128i _mm_cvtepu16_epi32(__m128i a)
{
    return vreinterpretq_m128i_u32(
        vmovl_u16(vget_low_u16(vreinterpretq_u16_m128i(a))));
}


FORCE_INLINE __m128i _mm_cvtepu16_epi64(__m128i a)
{
    uint16x8_t u16x8 = vreinterpretq_u16_m128i(a);
    uint32x4_t u32x4 = vmovl_u16(vget_low_u16(u16x8));
    uint64x2_t u64x2 = vmovl_u32(vget_low_u32(u32x4));
    return vreinterpretq_m128i_u64(u64x2);
}


FORCE_INLINE __m128i _mm_cvtepu32_epi64(__m128i a)
{
    return vreinterpretq_m128i_u64(
        vmovl_u32(vget_low_u32(vreinterpretq_u32_m128i(a))));
}


FORCE_INLINE __m128i _mm_cvtepu8_epi16(__m128i a)
{
    uint8x16_t u8x16 = vreinterpretq_u8_m128i(a);
    uint16x8_t u16x8 = vmovl_u8(vget_low_u8(u8x16));
    return vreinterpretq_m128i_u16(u16x8);
}


FORCE_INLINE __m128i _mm_cvtepu8_epi32(__m128i a)
{
    uint8x16_t u8x16 = vreinterpretq_u8_m128i(a);
    uint16x8_t u16x8 = vmovl_u8(vget_low_u8(u8x16));
    uint32x4_t u32x4 = vmovl_u16(vget_low_u16(u16x8));
    return vreinterpretq_m128i_u32(u32x4);
}


FORCE_INLINE __m128i _mm_cvtepu8_epi64(__m128i a)
{
    uint8x16_t u8x16 = vreinterpretq_u8_m128i(a);
    uint16x8_t u16x8 = vmovl_u8(vget_low_u8(u8x16));
    uint32x4_t u32x4 = vmovl_u16(vget_low_u16(u16x8));
    uint64x2_t u64x2 = vmovl_u32(vget_low_u32(u32x4));
    return vreinterpretq_m128i_u64(u64x2);
}


FORCE_INLINE __m128d _mm_dp_pd(__m128d a, __m128d b, const int imm)
{

    const int64_t bit0Mask = imm & 0x01 ? UINT64_MAX : 0;
    const int64_t bit1Mask = imm & 0x02 ? UINT64_MAX : 0;
#if !SSE2NEON_PRECISE_DP
    const int64_t bit4Mask = imm & 0x10 ? UINT64_MAX : 0;
    const int64_t bit5Mask = imm & 0x20 ? UINT64_MAX : 0;
#endif

#if !SSE2NEON_PRECISE_DP
    __m128d mul = _mm_mul_pd(a, b);
    const __m128d mulMask =
        _mm_castsi128_pd(_mm_set_epi64x(bit5Mask, bit4Mask));
    __m128d tmp = _mm_and_pd(mul, mulMask);
#else
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    double d0 = (imm & 0x10) ? vgetq_lane_f64(vreinterpretq_f64_m128d(a), 0) *
                                   vgetq_lane_f64(vreinterpretq_f64_m128d(b), 0)
                             : 0;
    double d1 = (imm & 0x20) ? vgetq_lane_f64(vreinterpretq_f64_m128d(a), 1) *
                                   vgetq_lane_f64(vreinterpretq_f64_m128d(b), 1)
                             : 0;
#else
    double a0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    double a1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    double b0 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 0));
    double b1 =
        sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(b), 1));
    double d0 = (imm & 0x10) ? a0 * b0 : 0;
    double d1 = (imm & 0x20) ? a1 * b1 : 0;
#endif
    __m128d tmp = _mm_set_pd(d1, d0);
#endif

#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    double sum = vpaddd_f64(vreinterpretq_f64_m128d(tmp));
#else
    double _tmp0 = sse2neon_recast_u64_f64(
        vgetq_lane_u64(vreinterpretq_u64_m128d(tmp), 0));
    double _tmp1 = sse2neon_recast_u64_f64(
        vgetq_lane_u64(vreinterpretq_u64_m128d(tmp), 1));
    double sum = _tmp0 + _tmp1;
#endif

    const __m128d sumMask =
        _mm_castsi128_pd(_mm_set_epi64x(bit1Mask, bit0Mask));
    __m128d res = _mm_and_pd(_mm_set_pd1(sum), sumMask);
    return res;
}


FORCE_INLINE __m128 _mm_dp_ps(__m128 a, __m128 b, const int imm)
{
    float32x4_t elementwise_prod = _mm_mul_ps(a, b);

#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)

    if (imm == 0xFF) {
        return _mm_set1_ps(vaddvq_f32(elementwise_prod));
    }

    if ((imm & 0x0F) == 0x0F) {
        if (!(imm & (1 << 4)))
            elementwise_prod = vsetq_lane_f32(0.0f, elementwise_prod, 0);
        if (!(imm & (1 << 5)))
            elementwise_prod = vsetq_lane_f32(0.0f, elementwise_prod, 1);
        if (!(imm & (1 << 6)))
            elementwise_prod = vsetq_lane_f32(0.0f, elementwise_prod, 2);
        if (!(imm & (1 << 7)))
            elementwise_prod = vsetq_lane_f32(0.0f, elementwise_prod, 3);

        return _mm_set1_ps(vaddvq_f32(elementwise_prod));
    }
#endif

    float s = 0.0f;

    if (imm & (1 << 4))
        s += vgetq_lane_f32(elementwise_prod, 0);
    if (imm & (1 << 5))
        s += vgetq_lane_f32(elementwise_prod, 1);
    if (imm & (1 << 6))
        s += vgetq_lane_f32(elementwise_prod, 2);
    if (imm & (1 << 7))
        s += vgetq_lane_f32(elementwise_prod, 3);

    const float32_t res[4] = {
        (imm & 0x1) ? s : 0.0f,
        (imm & 0x2) ? s : 0.0f,
        (imm & 0x4) ? s : 0.0f,
        (imm & 0x8) ? s : 0.0f,
    };
    return vreinterpretq_m128_f32(vld1q_f32(res));
}


#define _mm_extract_epi32(a, imm) \
    vgetq_lane_s32(vreinterpretq_s32_m128i(a), (imm))


#define _mm_extract_epi64(a, imm) \
    vgetq_lane_s64(vreinterpretq_s64_m128i(a), (imm))


#define _mm_extract_epi8(a, imm) vgetq_lane_u8(vreinterpretq_u8_m128i(a), (imm))


#define _mm_extract_ps(a, imm) vgetq_lane_s32(vreinterpretq_s32_m128(a), (imm))


FORCE_INLINE __m128d _mm_floor_pd(__m128d a)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128d_f64(vrndmq_f64(vreinterpretq_f64_m128d(a)));
#else
    double a0, a1;
    a0 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 0));
    a1 = sse2neon_recast_u64_f64(vgetq_lane_u64(vreinterpretq_u64_m128d(a), 1));
    return _mm_set_pd(floor(a1), floor(a0));
#endif
}


FORCE_INLINE __m128 _mm_floor_ps(__m128 a)
{
#if (defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)) || \
    defined(__ARM_FEATURE_DIRECTED_ROUNDING)
    return vreinterpretq_m128_f32(vrndmq_f32(vreinterpretq_f32_m128(a)));
#else
    float *f = (float *) &a;
    return _mm_set_ps(floorf(f[3]), floorf(f[2]), floorf(f[1]), floorf(f[0]));
#endif
}


FORCE_INLINE __m128d _mm_floor_sd(__m128d a, __m128d b)
{
    return _mm_move_sd(a, _mm_floor_pd(b));
}


FORCE_INLINE __m128 _mm_floor_ss(__m128 a, __m128 b)
{
    return _mm_move_ss(a, _mm_floor_ps(b));
}


#define _mm_insert_epi32(a, b, imm) \
    vreinterpretq_m128i_s32(        \
        vsetq_lane_s32((b), vreinterpretq_s32_m128i(a), (imm)))


#define _mm_insert_epi64(a, b, imm) \
    vreinterpretq_m128i_s64(        \
        vsetq_lane_s64((b), vreinterpretq_s64_m128i(a), (imm)))


#define _mm_insert_epi8(a, b, imm) \
    vreinterpretq_m128i_s8(vsetq_lane_s8((b), vreinterpretq_s8_m128i(a), (imm)))


#define _mm_insert_ps(a, b, imm8)                                              \
    _sse2neon_define2(                                                         \
        __m128, a, b,                                                          \
        float32x4_t tmp1 =                                                     \
            vsetq_lane_f32(vgetq_lane_f32(_b, ((imm8) >> 6) & 0x3),            \
                           vreinterpretq_f32_m128(_a), 0);                     \
        float32x4_t tmp2 =                                                     \
            vsetq_lane_f32(vgetq_lane_f32(tmp1, 0),                            \
                           vreinterpretq_f32_m128(_a), (((imm8) >> 4) & 0x3)); \
        const uint32_t data[4] =                                               \
            _sse2neon_init(((imm8) & (1 << 0)) ? UINT32_MAX : 0,               \
                           ((imm8) & (1 << 1)) ? UINT32_MAX : 0,               \
                           ((imm8) & (1 << 2)) ? UINT32_MAX : 0,               \
                           ((imm8) & (1 << 3)) ? UINT32_MAX : 0);              \
        uint32x4_t mask = vld1q_u32(data);                                     \
        float32x4_t all_zeros = vdupq_n_f32(0);                                \
                                                                               \
        _sse2neon_return(vreinterpretq_m128_f32(                               \
            vbslq_f32(mask, all_zeros, vreinterpretq_f32_m128(tmp2))));)


FORCE_INLINE __m128i _mm_max_epi32(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s32(
        vmaxq_s32(vreinterpretq_s32_m128i(a), vreinterpretq_s32_m128i(b)));
}


FORCE_INLINE __m128i _mm_max_epi8(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s8(
        vmaxq_s8(vreinterpretq_s8_m128i(a), vreinterpretq_s8_m128i(b)));
}


FORCE_INLINE __m128i _mm_max_epu16(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_u16(
        vmaxq_u16(vreinterpretq_u16_m128i(a), vreinterpretq_u16_m128i(b)));
}


FORCE_INLINE __m128i _mm_max_epu32(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_u32(
        vmaxq_u32(vreinterpretq_u32_m128i(a), vreinterpretq_u32_m128i(b)));
}


FORCE_INLINE __m128i _mm_min_epi32(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s32(
        vminq_s32(vreinterpretq_s32_m128i(a), vreinterpretq_s32_m128i(b)));
}


FORCE_INLINE __m128i _mm_min_epi8(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s8(
        vminq_s8(vreinterpretq_s8_m128i(a), vreinterpretq_s8_m128i(b)));
}


FORCE_INLINE __m128i _mm_min_epu16(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_u16(
        vminq_u16(vreinterpretq_u16_m128i(a), vreinterpretq_u16_m128i(b)));
}


FORCE_INLINE __m128i _mm_min_epu32(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_u32(
        vminq_u32(vreinterpretq_u32_m128i(a), vreinterpretq_u32_m128i(b)));
}


FORCE_INLINE __m128i _mm_minpos_epu16(__m128i a)
{
    __m128i dst;
    uint16_t min, idx = 0;
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)

    min = vminvq_u16(vreinterpretq_u16_m128i(a));


    static const uint16_t idxv[] = {0, 1, 2, 3, 4, 5, 6, 7};
    uint16x8_t minv = vdupq_n_u16(min);
    uint16x8_t cmeq = vceqq_u16(minv, vreinterpretq_u16_m128i(a));
    idx = vminvq_u16(vornq_u16(vld1q_u16(idxv), cmeq));
#else

    __m64 tmp;
    tmp = vreinterpret_m64_u16(
        vmin_u16(vget_low_u16(vreinterpretq_u16_m128i(a)),
                 vget_high_u16(vreinterpretq_u16_m128i(a))));
    tmp = vreinterpret_m64_u16(
        vpmin_u16(vreinterpret_u16_m64(tmp), vreinterpret_u16_m64(tmp)));
    tmp = vreinterpret_m64_u16(
        vpmin_u16(vreinterpret_u16_m64(tmp), vreinterpret_u16_m64(tmp)));
    min = vget_lane_u16(vreinterpret_u16_m64(tmp), 0);

    int i;
    for (i = 0; i < 8; i++) {
        if (min == vgetq_lane_u16(vreinterpretq_u16_m128i(a), 0)) {
            idx = (uint16_t) i;
            break;
        }
        a = _mm_srli_si128(a, 2);
    }
#endif

    dst = _mm_setzero_si128();
    dst = vreinterpretq_m128i_u16(
        vsetq_lane_u16(min, vreinterpretq_u16_m128i(dst), 0));
    dst = vreinterpretq_m128i_u16(
        vsetq_lane_u16(idx, vreinterpretq_u16_m128i(dst), 1));
    return dst;
}


FORCE_INLINE __m128i _mm_mpsadbw_epu8(__m128i a, __m128i b, const int imm)
{
    uint8x16_t _a, _b;

    switch (imm & 0x4) {
    case 0:

        _a = vreinterpretq_u8_m128i(a);
        break;
    case 4:
        _a = vreinterpretq_u8_u32(vextq_u32(vreinterpretq_u32_m128i(a),
                                            vreinterpretq_u32_m128i(a), 1));
        break;
    default:
#if defined(__GNUC__) || defined(__clang__)
        __builtin_unreachable();
#elif defined(_MSC_VER)
        __assume(0);
#endif
        break;
    }

    switch (imm & 0x3) {
    case 0:
        _b = vreinterpretq_u8_u32(
            vdupq_n_u32(vgetq_lane_u32(vreinterpretq_u32_m128i(b), 0)));
        break;
    case 1:
        _b = vreinterpretq_u8_u32(
            vdupq_n_u32(vgetq_lane_u32(vreinterpretq_u32_m128i(b), 1)));
        break;
    case 2:
        _b = vreinterpretq_u8_u32(
            vdupq_n_u32(vgetq_lane_u32(vreinterpretq_u32_m128i(b), 2)));
        break;
    case 3:
        _b = vreinterpretq_u8_u32(
            vdupq_n_u32(vgetq_lane_u32(vreinterpretq_u32_m128i(b), 3)));
        break;
    default:
#if defined(__GNUC__) || defined(__clang__)
        __builtin_unreachable();
#elif defined(_MSC_VER)
        __assume(0);
#endif
        break;
    }

    int16x8_t c04, c15, c26, c37;
    uint8x8_t low_b = vget_low_u8(_b);
    c04 = vreinterpretq_s16_u16(vabdl_u8(vget_low_u8(_a), low_b));
    uint8x16_t _a_1 = vextq_u8(_a, _a, 1);
    c15 = vreinterpretq_s16_u16(vabdl_u8(vget_low_u8(_a_1), low_b));
    uint8x16_t _a_2 = vextq_u8(_a, _a, 2);
    c26 = vreinterpretq_s16_u16(vabdl_u8(vget_low_u8(_a_2), low_b));
    uint8x16_t _a_3 = vextq_u8(_a, _a, 3);
    c37 = vreinterpretq_s16_u16(vabdl_u8(vget_low_u8(_a_3), low_b));
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)

    c04 = vpaddq_s16(c04, c26);

    c15 = vpaddq_s16(c15, c37);

    int32x4_t trn1_c =
        vtrn1q_s32(vreinterpretq_s32_s16(c04), vreinterpretq_s32_s16(c15));
    int32x4_t trn2_c =
        vtrn2q_s32(vreinterpretq_s32_s16(c04), vreinterpretq_s32_s16(c15));
    return vreinterpretq_m128i_s16(vpaddq_s16(vreinterpretq_s16_s32(trn1_c),
                                              vreinterpretq_s16_s32(trn2_c)));
#else
    int16x4_t c01, c23, c45, c67;
    c01 = vpadd_s16(vget_low_s16(c04), vget_low_s16(c15));
    c23 = vpadd_s16(vget_low_s16(c26), vget_low_s16(c37));
    c45 = vpadd_s16(vget_high_s16(c04), vget_high_s16(c15));
    c67 = vpadd_s16(vget_high_s16(c26), vget_high_s16(c37));

    return vreinterpretq_m128i_s16(
        vcombine_s16(vpadd_s16(c01, c23), vpadd_s16(c45, c67)));
#endif
}


FORCE_INLINE __m128i _mm_mul_epi32(__m128i a, __m128i b)
{

    int32x2_t a_lo = vmovn_s64(vreinterpretq_s64_m128i(a));
    int32x2_t b_lo = vmovn_s64(vreinterpretq_s64_m128i(b));
    return vreinterpretq_m128i_s64(vmull_s32(a_lo, b_lo));
}


FORCE_INLINE __m128i _mm_mullo_epi32(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s32(
        vmulq_s32(vreinterpretq_s32_m128i(a), vreinterpretq_s32_m128i(b)));
}


FORCE_INLINE __m128i _mm_packus_epi32(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_u16(
        vcombine_u16(vqmovun_s32(vreinterpretq_s32_m128i(a)),
                     vqmovun_s32(vreinterpretq_s32_m128i(b))));
}


FORCE_INLINE __m128d _mm_round_pd(__m128d a, int rounding)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    switch (rounding) {
    case (_MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC):
        return vreinterpretq_m128d_f64(vrndnq_f64(vreinterpretq_f64_m128d(a)));
    case (_MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC):
        return _mm_floor_pd(a);
    case (_MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC):
        return _mm_ceil_pd(a);
    case (_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC):
        return vreinterpretq_m128d_f64(vrndq_f64(vreinterpretq_f64_m128d(a)));
    default:
        return vreinterpretq_m128d_f64(vrndiq_f64(vreinterpretq_f64_m128d(a)));
    }
#else
    double *v_double = (double *) &a;

    if (rounding == (_MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC) ||
        (rounding == _MM_FROUND_CUR_DIRECTION &&
         _MM_GET_ROUNDING_MODE() == _MM_ROUND_NEAREST)) {
        double res[2], tmp;
        for (int i = 0; i < 2; i++) {
            tmp = (v_double[i] < 0) ? -v_double[i] : v_double[i];
            double roundDown = floor(tmp);
            double roundUp = ceil(tmp);
            double diffDown = tmp - roundDown;
            double diffUp = roundUp - tmp;
            if (diffDown < diffUp) {

                res[i] = roundDown;
            } else if (diffDown > diffUp) {

                res[i] = roundUp;
            } else {


                double half = roundDown / 2;
                if (half != floor(half)) {


                    res[i] = roundUp;
                } else {


                    res[i] = roundDown;
                }
            }
            res[i] = (v_double[i] < 0) ? -res[i] : res[i];
        }
        return _mm_set_pd(res[1], res[0]);
    } else if (rounding == (_MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC) ||
               (rounding == _MM_FROUND_CUR_DIRECTION &&
                _MM_GET_ROUNDING_MODE() == _MM_ROUND_DOWN)) {
        return _mm_floor_pd(a);
    } else if (rounding == (_MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC) ||
               (rounding == _MM_FROUND_CUR_DIRECTION &&
                _MM_GET_ROUNDING_MODE() == _MM_ROUND_UP)) {
        return _mm_ceil_pd(a);
    }
    return _mm_set_pd(v_double[1] > 0 ? floor(v_double[1]) : ceil(v_double[1]),
                      v_double[0] > 0 ? floor(v_double[0]) : ceil(v_double[0]));
#endif
}


FORCE_INLINE __m128 _mm_round_ps(__m128 a, int rounding)
{
#if (defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)) || \
    defined(__ARM_FEATURE_DIRECTED_ROUNDING)
    switch (rounding) {
    case (_MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC):
        return vreinterpretq_m128_f32(vrndnq_f32(vreinterpretq_f32_m128(a)));
    case (_MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC):
        return _mm_floor_ps(a);
    case (_MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC):
        return _mm_ceil_ps(a);
    case (_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC):
        return vreinterpretq_m128_f32(vrndq_f32(vreinterpretq_f32_m128(a)));
    default:
        return vreinterpretq_m128_f32(vrndiq_f32(vreinterpretq_f32_m128(a)));
    }
#else
    float *v_float = (float *) &a;

    if (rounding == (_MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC) ||
        (rounding == _MM_FROUND_CUR_DIRECTION &&
         _MM_GET_ROUNDING_MODE() == _MM_ROUND_NEAREST)) {
        uint32x4_t signmask = vdupq_n_u32(0x80000000);
        float32x4_t half = vbslq_f32(signmask, vreinterpretq_f32_m128(a),
                                     vdupq_n_f32(0.5f));
        int32x4_t r_normal = vcvtq_s32_f32(vaddq_f32(
            vreinterpretq_f32_m128(a), half));
        int32x4_t r_trunc = vcvtq_s32_f32(
            vreinterpretq_f32_m128(a));
        int32x4_t plusone = vreinterpretq_s32_u32(vshrq_n_u32(
            vreinterpretq_u32_s32(vnegq_s32(r_trunc)), 31));
        int32x4_t r_even = vbicq_s32(vaddq_s32(r_trunc, plusone),
                                     vdupq_n_s32(1));
        float32x4_t delta = vsubq_f32(
            vreinterpretq_f32_m128(a),
            vcvtq_f32_s32(r_trunc));
        uint32x4_t is_delta_half =
            vceqq_f32(delta, half);
        return vreinterpretq_m128_f32(
            vcvtq_f32_s32(vbslq_s32(is_delta_half, r_even, r_normal)));
    } else if (rounding == (_MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC) ||
               (rounding == _MM_FROUND_CUR_DIRECTION &&
                _MM_GET_ROUNDING_MODE() == _MM_ROUND_DOWN)) {
        return _mm_floor_ps(a);
    } else if (rounding == (_MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC) ||
               (rounding == _MM_FROUND_CUR_DIRECTION &&
                _MM_GET_ROUNDING_MODE() == _MM_ROUND_UP)) {
        return _mm_ceil_ps(a);
    }
    return _mm_set_ps(v_float[3] > 0 ? floorf(v_float[3]) : ceilf(v_float[3]),
                      v_float[2] > 0 ? floorf(v_float[2]) : ceilf(v_float[2]),
                      v_float[1] > 0 ? floorf(v_float[1]) : ceilf(v_float[1]),
                      v_float[0] > 0 ? floorf(v_float[0]) : ceilf(v_float[0]));
#endif
}


FORCE_INLINE __m128d _mm_round_sd(__m128d a, __m128d b, int rounding)
{
    return _mm_move_sd(a, _mm_round_pd(b, rounding));
}


FORCE_INLINE __m128 _mm_round_ss(__m128 a, __m128 b, int rounding)
{
    return _mm_move_ss(a, _mm_round_ps(b, rounding));
}


FORCE_INLINE __m128i _mm_stream_load_si128(__m128i *p)
{
#if __has_builtin(__builtin_nontemporal_store)
    return __builtin_nontemporal_load(p);
#else
    return vreinterpretq_m128i_s64(vld1q_s64((int64_t *) p));
#endif
}


FORCE_INLINE int _mm_test_all_ones(__m128i a)
{
    return (uint64_t) (vgetq_lane_s64(a, 0) & vgetq_lane_s64(a, 1)) ==
           ~(uint64_t) 0;
}


FORCE_INLINE int _mm_test_all_zeros(__m128i a, __m128i mask)
{
    int64x2_t a_and_mask =
        vandq_s64(vreinterpretq_s64_m128i(a), vreinterpretq_s64_m128i(mask));
    return !(vgetq_lane_s64(a_and_mask, 0) | vgetq_lane_s64(a_and_mask, 1));
}


FORCE_INLINE int _mm_test_mix_ones_zeros(__m128i a, __m128i mask)
{
    uint64x2_t v = vreinterpretq_u64_m128i(a);
    uint64x2_t m = vreinterpretq_u64_m128i(mask);


    uint64x2_t ones = vandq_u64(m, v);
    uint64x2_t zeros = vbicq_u64(m, v);


    uint32x2_t reduced = vpmax_u32(vqmovn_u64(ones), vqmovn_u64(zeros));


    return (vget_lane_u32(vpmin_u32(reduced, reduced), 0) != 0);
}


FORCE_INLINE int _mm_testc_si128(__m128i a, __m128i b)
{
    int64x2_t s64_vec =
        vbicq_s64(vreinterpretq_s64_m128i(b), vreinterpretq_s64_m128i(a));
    return !(vgetq_lane_s64(s64_vec, 0) | vgetq_lane_s64(s64_vec, 1));
}


#define _mm_testnzc_si128(a, b) _mm_test_mix_ones_zeros(a, b)


FORCE_INLINE int _mm_testz_si128(__m128i a, __m128i b)
{
    int64x2_t s64_vec =
        vandq_s64(vreinterpretq_s64_m128i(a), vreinterpretq_s64_m128i(b));
    return !(vgetq_lane_s64(s64_vec, 0) | vgetq_lane_s64(s64_vec, 1));
}


static const uint16_t ALIGN_STRUCT(16) _sse2neon_cmpestr_mask16b[8] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
};
static const uint8_t ALIGN_STRUCT(16) _sse2neon_cmpestr_mask8b[16] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
};


#define _SIDD_UBYTE_OPS 0x00
#define _SIDD_UWORD_OPS 0x01
#define _SIDD_SBYTE_OPS 0x02
#define _SIDD_SWORD_OPS 0x03


#define _SIDD_CMP_EQUAL_ANY 0x00
#define _SIDD_CMP_RANGES 0x04
#define _SIDD_CMP_EQUAL_EACH 0x08
#define _SIDD_CMP_EQUAL_ORDERED 0x0C


#define _SIDD_POSITIVE_POLARITY 0x00
#define _SIDD_MASKED_POSITIVE_POLARITY 0x20
#define _SIDD_NEGATIVE_POLARITY 0x10
#define _SIDD_MASKED_NEGATIVE_POLARITY \
    0x30


#define _SIDD_LEAST_SIGNIFICANT 0x00
#define _SIDD_MOST_SIGNIFICANT 0x40


#define _SIDD_BIT_MASK 0x00
#define _SIDD_UNIT_MASK 0x40


#define SSE2NEON_PRIMITIVE_CAT(a, ...) a##__VA_ARGS__
#define SSE2NEON_CAT(a, b) SSE2NEON_PRIMITIVE_CAT(a, b)

#define SSE2NEON_IIF(c) SSE2NEON_PRIMITIVE_CAT(SSE2NEON_IIF_, c)

#define SSE2NEON_IIF_0(t, ...) __VA_ARGS__

#define SSE2NEON_IIF_1(t, ...) t

#define SSE2NEON_COMPL(b) SSE2NEON_PRIMITIVE_CAT(SSE2NEON_COMPL_, b)
#define SSE2NEON_COMPL_0 1
#define SSE2NEON_COMPL_1 0

#define SSE2NEON_DEC(x) SSE2NEON_PRIMITIVE_CAT(SSE2NEON_DEC_, x)
#define SSE2NEON_DEC_1 0
#define SSE2NEON_DEC_2 1
#define SSE2NEON_DEC_3 2
#define SSE2NEON_DEC_4 3
#define SSE2NEON_DEC_5 4
#define SSE2NEON_DEC_6 5
#define SSE2NEON_DEC_7 6
#define SSE2NEON_DEC_8 7
#define SSE2NEON_DEC_9 8
#define SSE2NEON_DEC_10 9
#define SSE2NEON_DEC_11 10
#define SSE2NEON_DEC_12 11
#define SSE2NEON_DEC_13 12
#define SSE2NEON_DEC_14 13
#define SSE2NEON_DEC_15 14
#define SSE2NEON_DEC_16 15


#define SSE2NEON_CHECK_N(x, n, ...) n
#define SSE2NEON_CHECK(...) SSE2NEON_CHECK_N(__VA_ARGS__, 0, )
#define SSE2NEON_PROBE(x) x, 1,

#define SSE2NEON_NOT(x) SSE2NEON_CHECK(SSE2NEON_PRIMITIVE_CAT(SSE2NEON_NOT_, x))
#define SSE2NEON_NOT_0 SSE2NEON_PROBE(~)

#define SSE2NEON_BOOL(x) SSE2NEON_COMPL(SSE2NEON_NOT(x))
#define SSE2NEON_IF(c) SSE2NEON_IIF(SSE2NEON_BOOL(c))

#define SSE2NEON_EAT(...)
#define SSE2NEON_EXPAND(...) __VA_ARGS__
#define SSE2NEON_WHEN(c) SSE2NEON_IF(c)(SSE2NEON_EXPAND, SSE2NEON_EAT)


#define SSE2NEON_EMPTY()
#define SSE2NEON_DEFER(id) id SSE2NEON_EMPTY()
#define SSE2NEON_OBSTRUCT(...) __VA_ARGS__ SSE2NEON_DEFER(SSE2NEON_EMPTY)()
#define SSE2NEON_EXPAND(...) __VA_ARGS__

#define SSE2NEON_EVAL(...) \
    SSE2NEON_EVAL1(SSE2NEON_EVAL1(SSE2NEON_EVAL1(__VA_ARGS__)))
#define SSE2NEON_EVAL1(...) \
    SSE2NEON_EVAL2(SSE2NEON_EVAL2(SSE2NEON_EVAL2(__VA_ARGS__)))
#define SSE2NEON_EVAL2(...) \
    SSE2NEON_EVAL3(SSE2NEON_EVAL3(SSE2NEON_EVAL3(__VA_ARGS__)))
#define SSE2NEON_EVAL3(...) __VA_ARGS__

#define SSE2NEON_REPEAT(count, macro, ...)                         \
    SSE2NEON_WHEN(count)                                           \
    (SSE2NEON_OBSTRUCT(SSE2NEON_REPEAT_INDIRECT)()(                \
        SSE2NEON_DEC(count), macro,                                \
        __VA_ARGS__) SSE2NEON_OBSTRUCT(macro)(SSE2NEON_DEC(count), \
                                              __VA_ARGS__))
#define SSE2NEON_REPEAT_INDIRECT() SSE2NEON_REPEAT

#define SSE2NEON_SIZE_OF_byte 8
#define SSE2NEON_NUMBER_OF_LANES_byte 16
#define SSE2NEON_SIZE_OF_word 16
#define SSE2NEON_NUMBER_OF_LANES_word 8

#define SSE2NEON_COMPARE_EQUAL_THEN_FILL_LANE(i, type)                         \
    mtx[i] = vreinterpretq_m128i_##type(vceqq_##type(                          \
        vdupq_n_##type(vgetq_lane_##type(vreinterpretq_##type##_m128i(b), i)), \
        vreinterpretq_##type##_m128i(a)));

#define SSE2NEON_FILL_LANE(i, type) \
    vec_b[i] =                      \
        vdupq_n_##type(vgetq_lane_##type(vreinterpretq_##type##_m128i(b), i));

#define PCMPSTR_RANGES(a, b, mtx, data_type_prefix, type_prefix, size,        \
                       number_of_lanes, byte_or_word)                         \
    do {                                                                      \
        SSE2NEON_CAT(                                                         \
            data_type_prefix,                                                 \
            SSE2NEON_CAT(size,                                                \
                         SSE2NEON_CAT(x, SSE2NEON_CAT(number_of_lanes, _t)))) \
        vec_b[number_of_lanes];                                               \
        __m128i mask = SSE2NEON_IIF(byte_or_word)(                            \
            vreinterpretq_m128i_u16(vdupq_n_u16(0xff)),                       \
            vreinterpretq_m128i_u32(vdupq_n_u32(0xffff)));                    \
        SSE2NEON_EVAL(SSE2NEON_REPEAT(number_of_lanes, SSE2NEON_FILL_LANE,    \
                                      SSE2NEON_CAT(type_prefix, size)))       \
        for (int i = 0; i < number_of_lanes; i++) {                           \
            mtx[i] = SSE2NEON_CAT(vreinterpretq_m128i_u,                      \
                                  size)(SSE2NEON_CAT(vbslq_u, size)(          \
                SSE2NEON_CAT(vreinterpretq_u,                                 \
                             SSE2NEON_CAT(size, _m128i))(mask),               \
                SSE2NEON_CAT(vcgeq_, SSE2NEON_CAT(type_prefix, size))(        \
                    vec_b[i],                                                 \
                    SSE2NEON_CAT(                                             \
                        vreinterpretq_,                                       \
                        SSE2NEON_CAT(type_prefix,                             \
                                     SSE2NEON_CAT(size, _m128i(a))))),        \
                SSE2NEON_CAT(vcleq_, SSE2NEON_CAT(type_prefix, size))(        \
                    vec_b[i],                                                 \
                    SSE2NEON_CAT(                                             \
                        vreinterpretq_,                                       \
                        SSE2NEON_CAT(type_prefix,                             \
                                     SSE2NEON_CAT(size, _m128i(a)))))));      \
        }                                                                     \
    } while (0)

#define PCMPSTR_EQ(a, b, mtx, size, number_of_lanes)                         \
    do {                                                                     \
        SSE2NEON_EVAL(SSE2NEON_REPEAT(number_of_lanes,                       \
                                      SSE2NEON_COMPARE_EQUAL_THEN_FILL_LANE, \
                                      SSE2NEON_CAT(u, size)))                \
    } while (0)

#define SSE2NEON_CMP_EQUAL_ANY_IMPL(type)                               \
    static uint16_t _sse2neon_cmp_##type##_equal_any(__m128i a, int la, \
                                                     __m128i b, int lb) \
    {                                                                   \
        __m128i mtx[16];                                                \
        PCMPSTR_EQ(a, b, mtx, SSE2NEON_CAT(SSE2NEON_SIZE_OF_, type),    \
                   SSE2NEON_CAT(SSE2NEON_NUMBER_OF_LANES_, type));      \
        return SSE2NEON_CAT(                                            \
            _sse2neon_aggregate_equal_any_,                             \
            SSE2NEON_CAT(                                               \
                SSE2NEON_CAT(SSE2NEON_SIZE_OF_, type),                  \
                SSE2NEON_CAT(x, SSE2NEON_CAT(SSE2NEON_NUMBER_OF_LANES_, \
                                             type))))(la, lb, mtx);     \
    }

#define SSE2NEON_CMP_RANGES_IMPL(type, data_type, us, byte_or_word)          \
    static uint16_t _sse2neon_cmp_##us##type##_ranges(__m128i a, int la,     \
                                                      __m128i b, int lb)     \
    {                                                                        \
        __m128i mtx[16];                                                     \
        PCMPSTR_RANGES(                                                      \
            a, b, mtx, data_type, us, SSE2NEON_CAT(SSE2NEON_SIZE_OF_, type), \
            SSE2NEON_CAT(SSE2NEON_NUMBER_OF_LANES_, type), byte_or_word);    \
        return SSE2NEON_CAT(                                                 \
            _sse2neon_aggregate_ranges_,                                     \
            SSE2NEON_CAT(                                                    \
                SSE2NEON_CAT(SSE2NEON_SIZE_OF_, type),                       \
                SSE2NEON_CAT(x, SSE2NEON_CAT(SSE2NEON_NUMBER_OF_LANES_,      \
                                             type))))(la, lb, mtx);          \
    }

#define SSE2NEON_CMP_EQUAL_ORDERED_IMPL(type)                                  \
    static uint16_t _sse2neon_cmp_##type##_equal_ordered(__m128i a, int la,    \
                                                         __m128i b, int lb)    \
    {                                                                          \
        __m128i mtx[16];                                                       \
        PCMPSTR_EQ(a, b, mtx, SSE2NEON_CAT(SSE2NEON_SIZE_OF_, type),           \
                   SSE2NEON_CAT(SSE2NEON_NUMBER_OF_LANES_, type));             \
        return SSE2NEON_CAT(                                                   \
            _sse2neon_aggregate_equal_ordered_,                                \
            SSE2NEON_CAT(                                                      \
                SSE2NEON_CAT(SSE2NEON_SIZE_OF_, type),                         \
                SSE2NEON_CAT(x,                                                \
                             SSE2NEON_CAT(SSE2NEON_NUMBER_OF_LANES_, type))))( \
            SSE2NEON_CAT(SSE2NEON_NUMBER_OF_LANES_, type), la, lb, mtx);       \
    }

static uint16_t _sse2neon_aggregate_equal_any_8x16(int la,
                                                   int lb,
                                                   __m128i mtx[16])
{
    uint16_t res = 0;
    int m = (1 << la) - 1;
    uint8x8_t vec_mask = vld1_u8(_sse2neon_cmpestr_mask8b);
    uint8x8_t t_lo = vtst_u8(vdup_n_u8((uint8_t) (m & 0xff)), vec_mask);
    uint8x8_t t_hi = vtst_u8(vdup_n_u8((uint8_t) (m >> 8)), vec_mask);
    uint8x16_t vec = vcombine_u8(t_lo, t_hi);
    for (int j = 0; j < lb; j++) {
        mtx[j] = vreinterpretq_m128i_u8(
            vandq_u8(vec, vreinterpretq_u8_m128i(mtx[j])));
        mtx[j] = vreinterpretq_m128i_u8(
            vshrq_n_u8(vreinterpretq_u8_m128i(mtx[j]), 7));
        uint16_t tmp =
            _sse2neon_vaddvq_u8(vreinterpretq_u8_m128i(mtx[j])) ? 1 : 0;
        res |= (tmp << j);
    }
    return res;
}

static uint16_t _sse2neon_aggregate_equal_any_16x8(int la,
                                                   int lb,
                                                   __m128i mtx[16])
{
    uint16_t res = 0;
    uint16_t m = (uint16_t) (1 << la) - 1;
    uint16x8_t vec =
        vtstq_u16(vdupq_n_u16(m), vld1q_u16(_sse2neon_cmpestr_mask16b));
    for (int j = 0; j < lb; j++) {
        mtx[j] = vreinterpretq_m128i_u16(
            vandq_u16(vec, vreinterpretq_u16_m128i(mtx[j])));
        mtx[j] = vreinterpretq_m128i_u16(
            vshrq_n_u16(vreinterpretq_u16_m128i(mtx[j]), 15));
        uint16_t tmp =
            _sse2neon_vaddvq_u16(vreinterpretq_u16_m128i(mtx[j])) ? 1 : 0;
        res |= (tmp << j);
    }
    return res;
}


#define SSE2NEON_GENERATE_CMP_EQUAL_ANY(prefix) \
    prefix##IMPL(byte) \
    prefix##IMPL(word)


SSE2NEON_GENERATE_CMP_EQUAL_ANY(SSE2NEON_CMP_EQUAL_ANY_)

static uint16_t _sse2neon_aggregate_ranges_16x8(int la, int lb, __m128i mtx[16])
{
    uint16_t res = 0;
    uint16_t m = (uint16_t) (1 << la) - 1;
    uint16x8_t vec =
        vtstq_u16(vdupq_n_u16(m), vld1q_u16(_sse2neon_cmpestr_mask16b));
    for (int j = 0; j < lb; j++) {
        mtx[j] = vreinterpretq_m128i_u16(
            vandq_u16(vec, vreinterpretq_u16_m128i(mtx[j])));
        mtx[j] = vreinterpretq_m128i_u16(
            vshrq_n_u16(vreinterpretq_u16_m128i(mtx[j]), 15));
        __m128i tmp = vreinterpretq_m128i_u32(
            vshrq_n_u32(vreinterpretq_u32_m128i(mtx[j]), 16));
        uint32x4_t vec_res = vandq_u32(vreinterpretq_u32_m128i(mtx[j]),
                                       vreinterpretq_u32_m128i(tmp));
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
        uint16_t t = vaddvq_u32(vec_res) ? 1 : 0;
#else
        uint64x2_t sumh = vpaddlq_u32(vec_res);
        uint16_t t = vgetq_lane_u64(sumh, 0) + vgetq_lane_u64(sumh, 1);
#endif
        res |= (t << j);
    }
    return res;
}

static uint16_t _sse2neon_aggregate_ranges_8x16(int la, int lb, __m128i mtx[16])
{
    uint16_t res = 0;
    uint16_t m = (uint16_t) ((1 << la) - 1);
    uint8x8_t vec_mask = vld1_u8(_sse2neon_cmpestr_mask8b);
    uint8x8_t t_lo = vtst_u8(vdup_n_u8((uint8_t) (m & 0xff)), vec_mask);
    uint8x8_t t_hi = vtst_u8(vdup_n_u8((uint8_t) (m >> 8)), vec_mask);
    uint8x16_t vec = vcombine_u8(t_lo, t_hi);
    for (int j = 0; j < lb; j++) {
        mtx[j] = vreinterpretq_m128i_u8(
            vandq_u8(vec, vreinterpretq_u8_m128i(mtx[j])));
        mtx[j] = vreinterpretq_m128i_u8(
            vshrq_n_u8(vreinterpretq_u8_m128i(mtx[j]), 7));
        __m128i tmp = vreinterpretq_m128i_u16(
            vshrq_n_u16(vreinterpretq_u16_m128i(mtx[j]), 8));
        uint16x8_t vec_res = vandq_u16(vreinterpretq_u16_m128i(mtx[j]),
                                       vreinterpretq_u16_m128i(tmp));
        uint16_t t = _sse2neon_vaddvq_u16(vec_res) ? 1 : 0;
        res |= (t << j);
    }
    return res;
}

#define SSE2NEON_CMP_RANGES_IS_BYTE 1
#define SSE2NEON_CMP_RANGES_IS_WORD 0


#define SSE2NEON_GENERATE_CMP_RANGES(prefix)             \
    prefix##IMPL(byte, uint, u, prefix##IS_BYTE)         \
    prefix##IMPL(byte, int, s, prefix##IS_BYTE)          \
    prefix##IMPL(word, uint, u, prefix##IS_WORD)         \
    prefix##IMPL(word, int, s, prefix##IS_WORD)


SSE2NEON_GENERATE_CMP_RANGES(SSE2NEON_CMP_RANGES_)

#undef SSE2NEON_CMP_RANGES_IS_BYTE
#undef SSE2NEON_CMP_RANGES_IS_WORD

static uint16_t _sse2neon_cmp_byte_equal_each(__m128i a,
                                              int la,
                                              __m128i b,
                                              int lb)
{
    uint8x16_t mtx =
        vceqq_u8(vreinterpretq_u8_m128i(a), vreinterpretq_u8_m128i(b));
    uint16_t m0 = (la < lb) ? 0 : (uint16_t) ((1 << la) - (1 << lb));
    uint16_t m1 = (uint16_t) (0x10000 - (1 << la));
    uint16_t tb = (uint16_t) (0x10000 - (1 << lb));
    uint8x8_t vec_mask, vec0_lo, vec0_hi, vec1_lo, vec1_hi;
    uint8x8_t tmp_lo, tmp_hi, res_lo, res_hi;
    vec_mask = vld1_u8(_sse2neon_cmpestr_mask8b);
    vec0_lo = vtst_u8(vdup_n_u8((uint8_t) m0), vec_mask);
    vec0_hi = vtst_u8(vdup_n_u8((uint8_t) (m0 >> 8)), vec_mask);
    vec1_lo = vtst_u8(vdup_n_u8((uint8_t) m1), vec_mask);
    vec1_hi = vtst_u8(vdup_n_u8((uint8_t) (m1 >> 8)), vec_mask);
    tmp_lo = vtst_u8(vdup_n_u8((uint8_t) tb), vec_mask);
    tmp_hi = vtst_u8(vdup_n_u8((uint8_t) (tb >> 8)), vec_mask);

    res_lo = vbsl_u8(vec0_lo, vdup_n_u8(0), vget_low_u8(mtx));
    res_hi = vbsl_u8(vec0_hi, vdup_n_u8(0), vget_high_u8(mtx));
    res_lo = vbsl_u8(vec1_lo, tmp_lo, res_lo);
    res_hi = vbsl_u8(vec1_hi, tmp_hi, res_hi);
    res_lo = vand_u8(res_lo, vec_mask);
    res_hi = vand_u8(res_hi, vec_mask);

    return _sse2neon_vaddv_u8(res_lo) +
           (uint16_t) (_sse2neon_vaddv_u8(res_hi) << 8);
}

static uint16_t _sse2neon_cmp_word_equal_each(__m128i a,
                                              int la,
                                              __m128i b,
                                              int lb)
{
    uint16x8_t mtx =
        vceqq_u16(vreinterpretq_u16_m128i(a), vreinterpretq_u16_m128i(b));
    uint16_t m0 = (uint16_t) ((la < lb) ? 0 : ((1 << la) - (1 << lb)));
    uint16_t m1 = (uint16_t) (0x100 - (1 << la));
    uint16_t tb = (uint16_t) (0x100 - (1 << lb));
    uint16x8_t vec_mask = vld1q_u16(_sse2neon_cmpestr_mask16b);
    uint16x8_t vec0 = vtstq_u16(vdupq_n_u16(m0), vec_mask);
    uint16x8_t vec1 = vtstq_u16(vdupq_n_u16(m1), vec_mask);
    uint16x8_t tmp = vtstq_u16(vdupq_n_u16(tb), vec_mask);
    mtx = vbslq_u16(vec0, vdupq_n_u16(0), mtx);
    mtx = vbslq_u16(vec1, tmp, mtx);
    mtx = vandq_u16(mtx, vec_mask);
    return _sse2neon_vaddvq_u16(mtx);
}

#define SSE2NEON_AGGREGATE_EQUAL_ORDER_IS_UBYTE 1
#define SSE2NEON_AGGREGATE_EQUAL_ORDER_IS_UWORD 0

#define SSE2NEON_AGGREGATE_EQUAL_ORDER_IMPL(size, number_of_lanes, data_type)  \
    static uint16_t                                                            \
        _sse2neon_aggregate_equal_ordered_##size##x##number_of_lanes(          \
            int bound, int la, int lb, __m128i mtx[16])                        \
    {                                                                          \
        uint16_t res = 0;                                                      \
        uint16_t m1 =                                                          \
            (uint16_t) (SSE2NEON_IIF(data_type)(0x10000, 0x100) - (1 << la));  \
        uint##size##x8_t vec_mask = SSE2NEON_IIF(data_type)(                   \
            vld1_u##size(_sse2neon_cmpestr_mask##size##b),                     \
            vld1q_u##size(_sse2neon_cmpestr_mask##size##b));                   \
        uint##size##x##number_of_lanes##_t vec1 = SSE2NEON_IIF(data_type)(     \
            vcombine_u##size(                                                  \
                vtst_u##size(vdup_n_u##size((uint##size##_t) m1), vec_mask),   \
                vtst_u##size(vdup_n_u##size((uint##size##_t)(m1 >> 8)),        \
                             vec_mask)),                                       \
            vtstq_u##size(vdupq_n_u##size((uint##size##_t) m1), vec_mask));    \
        uint##size##x##number_of_lanes##_t vec_minusone = vdupq_n_u##size(-1); \
        uint##size##x##number_of_lanes##_t vec_zero = vdupq_n_u##size(0);      \
        for (int j = 0; j < lb; j++) {                                         \
            mtx[j] = vreinterpretq_m128i_u##size(vbslq_u##size(                \
                vec1, vec_minusone, vreinterpretq_u##size##_m128i(mtx[j])));   \
        }                                                                      \
        for (int j = lb; j < bound; j++) {                                     \
            mtx[j] = vreinterpretq_m128i_u##size(                              \
                vbslq_u##size(vec1, vec_minusone, vec_zero));                  \
        }                                                                      \
        unsigned SSE2NEON_IIF(data_type)(char, short) *ptr =                   \
            (unsigned SSE2NEON_IIF(data_type)(char, short) *) mtx;             \
        for (int i = 0; i < bound; i++) {                                      \
            int val = 1;                                                       \
            for (int j = 0, k = i; j < bound - i && k < bound; j++, k++)       \
                val &= ptr[k * bound + j];                                     \
            res += (uint16_t) (val << i);                                      \
        }                                                                      \
        return res;                                                            \
    }


#define SSE2NEON_GENERATE_AGGREGATE_EQUAL_ORDER(prefix) \
    prefix##IMPL(8, 16, prefix##IS_UBYTE)               \
    prefix##IMPL(16, 8, prefix##IS_UWORD)


SSE2NEON_GENERATE_AGGREGATE_EQUAL_ORDER(SSE2NEON_AGGREGATE_EQUAL_ORDER_)

#undef SSE2NEON_AGGREGATE_EQUAL_ORDER_IS_UBYTE
#undef SSE2NEON_AGGREGATE_EQUAL_ORDER_IS_UWORD


#define SSE2NEON_GENERATE_CMP_EQUAL_ORDERED(prefix) \
    prefix##IMPL(byte)                              \
    prefix##IMPL(word)


SSE2NEON_GENERATE_CMP_EQUAL_ORDERED(SSE2NEON_CMP_EQUAL_ORDERED_)

#define SSE2NEON_CMPESTR_LIST                                  \
    _SSE2NEON(CMP_UBYTE_EQUAL_ANY, cmp_byte_equal_any)         \
    _SSE2NEON(CMP_UWORD_EQUAL_ANY, cmp_word_equal_any)         \
    _SSE2NEON(CMP_SBYTE_EQUAL_ANY, cmp_byte_equal_any)         \
    _SSE2NEON(CMP_SWORD_EQUAL_ANY, cmp_word_equal_any)         \
    _SSE2NEON(CMP_UBYTE_RANGES, cmp_ubyte_ranges)              \
    _SSE2NEON(CMP_UWORD_RANGES, cmp_uword_ranges)              \
    _SSE2NEON(CMP_SBYTE_RANGES, cmp_sbyte_ranges)              \
    _SSE2NEON(CMP_SWORD_RANGES, cmp_sword_ranges)              \
    _SSE2NEON(CMP_UBYTE_EQUAL_EACH, cmp_byte_equal_each)       \
    _SSE2NEON(CMP_UWORD_EQUAL_EACH, cmp_word_equal_each)       \
    _SSE2NEON(CMP_SBYTE_EQUAL_EACH, cmp_byte_equal_each)       \
    _SSE2NEON(CMP_SWORD_EQUAL_EACH, cmp_word_equal_each)       \
    _SSE2NEON(CMP_UBYTE_EQUAL_ORDERED, cmp_byte_equal_ordered) \
    _SSE2NEON(CMP_UWORD_EQUAL_ORDERED, cmp_word_equal_ordered) \
    _SSE2NEON(CMP_SBYTE_EQUAL_ORDERED, cmp_byte_equal_ordered) \
    _SSE2NEON(CMP_SWORD_EQUAL_ORDERED, cmp_word_equal_ordered)

enum {
#define _SSE2NEON(name, func_suffix) name,
    SSE2NEON_CMPESTR_LIST
#undef _SSE2NEON
};
typedef uint16_t (*cmpestr_func_t)(__m128i a, int la, __m128i b, int lb);
static cmpestr_func_t _sse2neon_cmpfunc_table[] = {
#define _SSE2NEON(name, func_suffix) _sse2neon_##func_suffix,
    SSE2NEON_CMPESTR_LIST
#undef _SSE2NEON
};

FORCE_INLINE uint16_t _sse2neon_sido_negative(int res,
                                              int lb,
                                              int imm8,
                                              int bound)
{
    switch (imm8 & 0x30) {
    case _SIDD_NEGATIVE_POLARITY:
        res ^= 0xffffffff;
        break;
    case _SIDD_MASKED_NEGATIVE_POLARITY:
        res ^= (1 << lb) - 1;
        break;
    default:
        break;
    }

    return (uint16_t) (res & ((bound == 8) ? 0xFF : 0xFFFF));
}

FORCE_INLINE int _sse2neon_clz(unsigned int x)
{
#if defined(_MSC_VER) && !defined(__clang__)
    unsigned long cnt = 0;
    if (_BitScanReverse(&cnt, x))
        return 31 - cnt;
    return 32;
#else
    return x != 0 ? __builtin_clz(x) : 32;
#endif
}

FORCE_INLINE int _sse2neon_ctz(unsigned int x)
{
#if defined(_MSC_VER) && !defined(__clang__)
    unsigned long cnt = 0;
    if (_BitScanForward(&cnt, x))
        return cnt;
    return 32;
#else
    return x != 0 ? __builtin_ctz(x) : 32;
#endif
}

FORCE_INLINE int _sse2neon_ctzll(unsigned long long x)
{
#ifdef _MSC_VER
    unsigned long cnt;
#if defined(SSE2NEON_HAS_BITSCAN64)
    if (_BitScanForward64(&cnt, x))
        return (int) (cnt);
#else
    if (_BitScanForward(&cnt, (unsigned long) (x)))
        return (int) cnt;
    if (_BitScanForward(&cnt, (unsigned long) (x >> 32)))
        return (int) (cnt + 32);
#endif
    return 64;
#else
    return x != 0 ? __builtin_ctzll(x) : 64;
#endif
}

#define SSE2NEON_MIN(x, y) (x) < (y) ? (x) : (y)

#define SSE2NEON_CMPSTR_SET_UPPER(var, imm) \
    const int var = ((imm) & 0x01) ? 8 : 16

#define SSE2NEON_CMPESTRX_LEN_PAIR(a, b, la, lb) \
    int tmp1 = la ^ (la >> 31);                  \
    la = tmp1 - (la >> 31);                      \
    int tmp2 = lb ^ (lb >> 31);                  \
    lb = tmp2 - (lb >> 31);                      \
    la = SSE2NEON_MIN(la, bound);                \
    lb = SSE2NEON_MIN(lb, bound)


#define SSE2NEON_COMP_AGG(a, b, la, lb, imm8, IE)                         \
    SSE2NEON_CMPSTR_SET_UPPER(bound, imm8);                               \
    SSE2NEON_##IE##_LEN_PAIR(a, b, la, lb);                               \
    uint16_t r2 = (_sse2neon_cmpfunc_table[(imm8) & 0x0f])(a, la, b, lb); \
    r2 = _sse2neon_sido_negative(r2, lb, imm8, bound)

#define SSE2NEON_CMPSTR_GENERATE_INDEX(r2, bound, imm8)            \
    return (r2 == 0) ? bound                                       \
                     : (((imm8) & 0x40) ? (31 - _sse2neon_clz(r2)) \
                                        : _sse2neon_ctz(r2))

#define SSE2NEON_CMPSTR_GENERATE_MASK(dst)                                     \
    __m128i dst = vreinterpretq_m128i_u8(vdupq_n_u8(0));                       \
    if ((imm8) & 0x40) {                                                       \
        if (bound == 8) {                                                      \
            uint16x8_t tmp = vtstq_u16(vdupq_n_u16(r2),                        \
                                       vld1q_u16(_sse2neon_cmpestr_mask16b));  \
            dst = vreinterpretq_m128i_u16(vbslq_u16(                           \
                tmp, vdupq_n_u16(-1), vreinterpretq_u16_m128i(dst)));          \
        } else {                                                               \
            uint8x16_t vec_r2 = vcombine_u8(vdup_n_u8((uint8_t) r2),           \
                                            vdup_n_u8((uint8_t) (r2 >> 8)));   \
            uint8x16_t tmp =                                                   \
                vtstq_u8(vec_r2, vld1q_u8(_sse2neon_cmpestr_mask8b));          \
            dst = vreinterpretq_m128i_u8(                                      \
                vbslq_u8(tmp, vdupq_n_u8(-1), vreinterpretq_u8_m128i(dst)));   \
        }                                                                      \
    } else {                                                                   \
        if (bound == 16) {                                                     \
            dst = vreinterpretq_m128i_u16(                                     \
                vsetq_lane_u16(r2 & 0xffff, vreinterpretq_u16_m128i(dst), 0)); \
        } else {                                                               \
            dst = vreinterpretq_m128i_u8(vsetq_lane_u8(                        \
                (uint8_t) (r2 & 0xff), vreinterpretq_u8_m128i(dst), 0));       \
        }                                                                      \
    }                                                                          \
    return dst


FORCE_INLINE int _mm_cmpestra(__m128i a,
                              int la,
                              __m128i b,
                              int lb,
                              const int imm8)
{
    int lb_cpy = lb;
    SSE2NEON_COMP_AGG(a, b, la, lb, imm8, CMPESTRX);
    return !r2 & (lb_cpy > bound);
}


FORCE_INLINE int _mm_cmpestrc(__m128i a,
                              int la,
                              __m128i b,
                              int lb,
                              const int imm8)
{
    SSE2NEON_COMP_AGG(a, b, la, lb, imm8, CMPESTRX);
    return r2 != 0;
}


FORCE_INLINE int _mm_cmpestri(__m128i a,
                              int la,
                              __m128i b,
                              int lb,
                              const int imm8)
{
    SSE2NEON_COMP_AGG(a, b, la, lb, imm8, CMPESTRX);
    SSE2NEON_CMPSTR_GENERATE_INDEX(r2, bound, imm8);
}


FORCE_INLINE __m128i
_mm_cmpestrm(__m128i a, int la, __m128i b, int lb, const int imm8)
{
    SSE2NEON_COMP_AGG(a, b, la, lb, imm8, CMPESTRX);
    SSE2NEON_CMPSTR_GENERATE_MASK(dst);
}


FORCE_INLINE int _mm_cmpestro(__m128i a,
                              int la,
                              __m128i b,
                              int lb,
                              const int imm8)
{
    SSE2NEON_COMP_AGG(a, b, la, lb, imm8, CMPESTRX);
    return r2 & 1;
}


FORCE_INLINE int _mm_cmpestrs(__m128i a,
                              int la,
                              __m128i b,
                              int lb,
                              const int imm8)
{
    (void) a;
    (void) b;
    (void) lb;
    SSE2NEON_CMPSTR_SET_UPPER(bound, imm8);
    return la <= (bound - 1);
}


FORCE_INLINE int _mm_cmpestrz(__m128i a,
                              int la,
                              __m128i b,
                              int lb,
                              const int imm8)
{
    (void) a;
    (void) b;
    (void) la;
    SSE2NEON_CMPSTR_SET_UPPER(bound, imm8);
    return lb <= (bound - 1);
}

#define SSE2NEON_CMPISTRX_LENGTH(str, len, imm8)                         \
    do {                                                                 \
        if ((imm8) & 0x01) {                                             \
            uint16x8_t equal_mask_##str =                                \
                vceqq_u16(vreinterpretq_u16_m128i(str), vdupq_n_u16(0)); \
            uint8x8_t res_##str = vshrn_n_u16(equal_mask_##str, 4);      \
            uint64_t matches_##str =                                     \
                vget_lane_u64(vreinterpret_u64_u8(res_##str), 0);        \
            len = _sse2neon_ctzll(matches_##str) >> 3;                   \
        } else {                                                         \
            uint16x8_t equal_mask_##str = vreinterpretq_u16_u8(          \
                vceqq_u8(vreinterpretq_u8_m128i(str), vdupq_n_u8(0)));   \
            uint8x8_t res_##str = vshrn_n_u16(equal_mask_##str, 4);      \
            uint64_t matches_##str =                                     \
                vget_lane_u64(vreinterpret_u64_u8(res_##str), 0);        \
            len = _sse2neon_ctzll(matches_##str) >> 2;                   \
        }                                                                \
    } while (0)

#define SSE2NEON_CMPISTRX_LEN_PAIR(a, b, la, lb) \
    int la, lb;                                  \
    do {                                         \
        SSE2NEON_CMPISTRX_LENGTH(a, la, imm8);   \
        SSE2NEON_CMPISTRX_LENGTH(b, lb, imm8);   \
    } while (0)


FORCE_INLINE int _mm_cmpistra(__m128i a, __m128i b, const int imm8)
{
    SSE2NEON_COMP_AGG(a, b, la, lb, imm8, CMPISTRX);
    return !r2 & (lb >= bound);
}


FORCE_INLINE int _mm_cmpistrc(__m128i a, __m128i b, const int imm8)
{
    SSE2NEON_COMP_AGG(a, b, la, lb, imm8, CMPISTRX);
    return r2 != 0;
}


FORCE_INLINE int _mm_cmpistri(__m128i a, __m128i b, const int imm8)
{
    SSE2NEON_COMP_AGG(a, b, la, lb, imm8, CMPISTRX);
    SSE2NEON_CMPSTR_GENERATE_INDEX(r2, bound, imm8);
}


FORCE_INLINE __m128i _mm_cmpistrm(__m128i a, __m128i b, const int imm8)
{
    SSE2NEON_COMP_AGG(a, b, la, lb, imm8, CMPISTRX);
    SSE2NEON_CMPSTR_GENERATE_MASK(dst);
}


FORCE_INLINE int _mm_cmpistro(__m128i a, __m128i b, const int imm8)
{
    SSE2NEON_COMP_AGG(a, b, la, lb, imm8, CMPISTRX);
    return r2 & 1;
}


FORCE_INLINE int _mm_cmpistrs(__m128i a, __m128i b, const int imm8)
{
    (void) b;
    SSE2NEON_CMPSTR_SET_UPPER(bound, imm8);
    int la;
    SSE2NEON_CMPISTRX_LENGTH(a, la, imm8);
    return la <= (bound - 1);
}


FORCE_INLINE int _mm_cmpistrz(__m128i a, __m128i b, const int imm8)
{
    (void) a;
    SSE2NEON_CMPSTR_SET_UPPER(bound, imm8);
    int lb;
    SSE2NEON_CMPISTRX_LENGTH(b, lb, imm8);
    return lb <= (bound - 1);
}


FORCE_INLINE __m128i _mm_cmpgt_epi64(__m128i a, __m128i b)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    return vreinterpretq_m128i_u64(
        vcgtq_s64(vreinterpretq_s64_m128i(a), vreinterpretq_s64_m128i(b)));
#else
    return vreinterpretq_m128i_s64(vshrq_n_s64(
        vqsubq_s64(vreinterpretq_s64_m128i(b), vreinterpretq_s64_m128i(a)),
        63));
#endif
}


FORCE_INLINE uint32_t _mm_crc32_u16(uint32_t crc, uint16_t v)
{
#if defined(__aarch64__) && defined(__ARM_FEATURE_CRC32)
    __asm__ __volatile__("crc32ch %w[c], %w[c], %w[v]\n\t"
                         : [c] "+r"(crc)
                         : [v] "r"(v));
#elif ((__ARM_ARCH == 8) && defined(__ARM_FEATURE_CRC32)) || \
    ((defined(_M_ARM64) || defined(_M_ARM64EC)) && !defined(__clang__))
    crc = __crc32ch(crc, v);
#else
    crc = _mm_crc32_u8(crc, (uint8_t) (v & 0xff));
    crc = _mm_crc32_u8(crc, (uint8_t) ((v >> 8) & 0xff));
#endif
    return crc;
}


FORCE_INLINE uint32_t _mm_crc32_u32(uint32_t crc, uint32_t v)
{
#if defined(__aarch64__) && defined(__ARM_FEATURE_CRC32)
    __asm__ __volatile__("crc32cw %w[c], %w[c], %w[v]\n\t"
                         : [c] "+r"(crc)
                         : [v] "r"(v));
#elif ((__ARM_ARCH == 8) && defined(__ARM_FEATURE_CRC32)) || \
    ((defined(_M_ARM64) || defined(_M_ARM64EC)) && !defined(__clang__))
    crc = __crc32cw(crc, v);
#else
    crc = _mm_crc32_u16(crc, (uint16_t) (v & 0xffff));
    crc = _mm_crc32_u16(crc, (uint16_t) ((v >> 16) & 0xffff));
#endif
    return crc;
}


FORCE_INLINE uint64_t _mm_crc32_u64(uint64_t crc, uint64_t v)
{
#if defined(__aarch64__) && defined(__ARM_FEATURE_CRC32)
    __asm__ __volatile__("crc32cx %w[c], %w[c], %x[v]\n\t"
                         : [c] "+r"(crc)
                         : [v] "r"(v));
#elif ((defined(_M_ARM64) || defined(_M_ARM64EC)) && !defined(__clang__))
    crc = __crc32cd((uint32_t) crc, v);
#else
    crc = _mm_crc32_u32((uint32_t) (crc), (uint32_t) (v & 0xffffffff));
    crc = _mm_crc32_u32((uint32_t) (crc), (uint32_t) ((v >> 32) & 0xffffffff));
#endif
    return crc;
}


FORCE_INLINE uint32_t _mm_crc32_u8(uint32_t crc, uint8_t v)
{
#if defined(__aarch64__) && defined(__ARM_FEATURE_CRC32)
    __asm__ __volatile__("crc32cb %w[c], %w[c], %w[v]\n\t"
                         : [c] "+r"(crc)
                         : [v] "r"(v));
#elif ((__ARM_ARCH == 8) && defined(__ARM_FEATURE_CRC32)) || \
    ((defined(_M_ARM64) || defined(_M_ARM64EC)) && !defined(__clang__))
    crc = __crc32cb(crc, v);
#else
    crc ^= v;
#if defined(__ARM_FEATURE_CRYPTO)


    uint64x2_t orig =
        vcombine_u64(vcreate_u64((uint64_t) (crc) << 24), vcreate_u64(0x0));
    uint64x2_t tmp = orig;


    uint64_t p = 0x105EC76F1;


    uint64_t mu = 0x1dea713f1;


    tmp = _sse2neon_vmull_p64(vget_low_u64(tmp), vcreate_u64(mu));

    tmp =
        vandq_u64(tmp, vcombine_u64(vcreate_u64(0xFFFFFFFF), vcreate_u64(0x0)));

    tmp = _sse2neon_vmull_p64(vget_low_u64(tmp), vcreate_u64(p));

    tmp = veorq_u64(tmp, orig);


    crc = vgetq_lane_u32(vreinterpretq_u32_u64(tmp), 1);
#else


    static const uint32_t crc32_half_byte_tbl[] = {
        0x00000000, 0x105ec76f, 0x20bd8ede, 0x30e349b1, 0x417b1dbc, 0x5125dad3,
        0x61c69362, 0x7198540d, 0x82f63b78, 0x92a8fc17, 0xa24bb5a6, 0xb21572c9,
        0xc38d26c4, 0xd3d3e1ab, 0xe330a81a, 0xf36e6f75,
    };

    crc = (crc >> 4) ^ crc32_half_byte_tbl[crc & 0x0F];
    crc = (crc >> 4) ^ crc32_half_byte_tbl[crc & 0x0F];
#endif
#endif
    return crc;
}


#if !defined(__ARM_FEATURE_CRYPTO) && \
    ((!defined(_M_ARM64) && !defined(_M_ARM64EC)) || defined(__clang__))

#define SSE2NEON_AES_SBOX(w)                                           \
    {                                                                  \
        w(0x63), w(0x7c), w(0x77), w(0x7b), w(0xf2), w(0x6b), w(0x6f), \
        w(0xc5), w(0x30), w(0x01), w(0x67), w(0x2b), w(0xfe), w(0xd7), \
        w(0xab), w(0x76), w(0xca), w(0x82), w(0xc9), w(0x7d), w(0xfa), \
        w(0x59), w(0x47), w(0xf0), w(0xad), w(0xd4), w(0xa2), w(0xaf), \
        w(0x9c), w(0xa4), w(0x72), w(0xc0), w(0xb7), w(0xfd), w(0x93), \
        w(0x26), w(0x36), w(0x3f), w(0xf7), w(0xcc), w(0x34), w(0xa5), \
        w(0xe5), w(0xf1), w(0x71), w(0xd8), w(0x31), w(0x15), w(0x04), \
        w(0xc7), w(0x23), w(0xc3), w(0x18), w(0x96), w(0x05), w(0x9a), \
        w(0x07), w(0x12), w(0x80), w(0xe2), w(0xeb), w(0x27), w(0xb2), \
        w(0x75), w(0x09), w(0x83), w(0x2c), w(0x1a), w(0x1b), w(0x6e), \
        w(0x5a), w(0xa0), w(0x52), w(0x3b), w(0xd6), w(0xb3), w(0x29), \
        w(0xe3), w(0x2f), w(0x84), w(0x53), w(0xd1), w(0x00), w(0xed), \
        w(0x20), w(0xfc), w(0xb1), w(0x5b), w(0x6a), w(0xcb), w(0xbe), \
        w(0x39), w(0x4a), w(0x4c), w(0x58), w(0xcf), w(0xd0), w(0xef), \
        w(0xaa), w(0xfb), w(0x43), w(0x4d), w(0x33), w(0x85), w(0x45), \
        w(0xf9), w(0x02), w(0x7f), w(0x50), w(0x3c), w(0x9f), w(0xa8), \
        w(0x51), w(0xa3), w(0x40), w(0x8f), w(0x92), w(0x9d), w(0x38), \
        w(0xf5), w(0xbc), w(0xb6), w(0xda), w(0x21), w(0x10), w(0xff), \
        w(0xf3), w(0xd2), w(0xcd), w(0x0c), w(0x13), w(0xec), w(0x5f), \
        w(0x97), w(0x44), w(0x17), w(0xc4), w(0xa7), w(0x7e), w(0x3d), \
        w(0x64), w(0x5d), w(0x19), w(0x73), w(0x60), w(0x81), w(0x4f), \
        w(0xdc), w(0x22), w(0x2a), w(0x90), w(0x88), w(0x46), w(0xee), \
        w(0xb8), w(0x14), w(0xde), w(0x5e), w(0x0b), w(0xdb), w(0xe0), \
        w(0x32), w(0x3a), w(0x0a), w(0x49), w(0x06), w(0x24), w(0x5c), \
        w(0xc2), w(0xd3), w(0xac), w(0x62), w(0x91), w(0x95), w(0xe4), \
        w(0x79), w(0xe7), w(0xc8), w(0x37), w(0x6d), w(0x8d), w(0xd5), \
        w(0x4e), w(0xa9), w(0x6c), w(0x56), w(0xf4), w(0xea), w(0x65), \
        w(0x7a), w(0xae), w(0x08), w(0xba), w(0x78), w(0x25), w(0x2e), \
        w(0x1c), w(0xa6), w(0xb4), w(0xc6), w(0xe8), w(0xdd), w(0x74), \
        w(0x1f), w(0x4b), w(0xbd), w(0x8b), w(0x8a), w(0x70), w(0x3e), \
        w(0xb5), w(0x66), w(0x48), w(0x03), w(0xf6), w(0x0e), w(0x61), \
        w(0x35), w(0x57), w(0xb9), w(0x86), w(0xc1), w(0x1d), w(0x9e), \
        w(0xe1), w(0xf8), w(0x98), w(0x11), w(0x69), w(0xd9), w(0x8e), \
        w(0x94), w(0x9b), w(0x1e), w(0x87), w(0xe9), w(0xce), w(0x55), \
        w(0x28), w(0xdf), w(0x8c), w(0xa1), w(0x89), w(0x0d), w(0xbf), \
        w(0xe6), w(0x42), w(0x68), w(0x41), w(0x99), w(0x2d), w(0x0f), \
        w(0xb0), w(0x54), w(0xbb), w(0x16)                             \
    }
#define SSE2NEON_AES_RSBOX(w)                                          \
    {                                                                  \
        w(0x52), w(0x09), w(0x6a), w(0xd5), w(0x30), w(0x36), w(0xa5), \
        w(0x38), w(0xbf), w(0x40), w(0xa3), w(0x9e), w(0x81), w(0xf3), \
        w(0xd7), w(0xfb), w(0x7c), w(0xe3), w(0x39), w(0x82), w(0x9b), \
        w(0x2f), w(0xff), w(0x87), w(0x34), w(0x8e), w(0x43), w(0x44), \
        w(0xc4), w(0xde), w(0xe9), w(0xcb), w(0x54), w(0x7b), w(0x94), \
        w(0x32), w(0xa6), w(0xc2), w(0x23), w(0x3d), w(0xee), w(0x4c), \
        w(0x95), w(0x0b), w(0x42), w(0xfa), w(0xc3), w(0x4e), w(0x08), \
        w(0x2e), w(0xa1), w(0x66), w(0x28), w(0xd9), w(0x24), w(0xb2), \
        w(0x76), w(0x5b), w(0xa2), w(0x49), w(0x6d), w(0x8b), w(0xd1), \
        w(0x25), w(0x72), w(0xf8), w(0xf6), w(0x64), w(0x86), w(0x68), \
        w(0x98), w(0x16), w(0xd4), w(0xa4), w(0x5c), w(0xcc), w(0x5d), \
        w(0x65), w(0xb6), w(0x92), w(0x6c), w(0x70), w(0x48), w(0x50), \
        w(0xfd), w(0xed), w(0xb9), w(0xda), w(0x5e), w(0x15), w(0x46), \
        w(0x57), w(0xa7), w(0x8d), w(0x9d), w(0x84), w(0x90), w(0xd8), \
        w(0xab), w(0x00), w(0x8c), w(0xbc), w(0xd3), w(0x0a), w(0xf7), \
        w(0xe4), w(0x58), w(0x05), w(0xb8), w(0xb3), w(0x45), w(0x06), \
        w(0xd0), w(0x2c), w(0x1e), w(0x8f), w(0xca), w(0x3f), w(0x0f), \
        w(0x02), w(0xc1), w(0xaf), w(0xbd), w(0x03), w(0x01), w(0x13), \
        w(0x8a), w(0x6b), w(0x3a), w(0x91), w(0x11), w(0x41), w(0x4f), \
        w(0x67), w(0xdc), w(0xea), w(0x97), w(0xf2), w(0xcf), w(0xce), \
        w(0xf0), w(0xb4), w(0xe6), w(0x73), w(0x96), w(0xac), w(0x74), \
        w(0x22), w(0xe7), w(0xad), w(0x35), w(0x85), w(0xe2), w(0xf9), \
        w(0x37), w(0xe8), w(0x1c), w(0x75), w(0xdf), w(0x6e), w(0x47), \
        w(0xf1), w(0x1a), w(0x71), w(0x1d), w(0x29), w(0xc5), w(0x89), \
        w(0x6f), w(0xb7), w(0x62), w(0x0e), w(0xaa), w(0x18), w(0xbe), \
        w(0x1b), w(0xfc), w(0x56), w(0x3e), w(0x4b), w(0xc6), w(0xd2), \
        w(0x79), w(0x20), w(0x9a), w(0xdb), w(0xc0), w(0xfe), w(0x78), \
        w(0xcd), w(0x5a), w(0xf4), w(0x1f), w(0xdd), w(0xa8), w(0x33), \
        w(0x88), w(0x07), w(0xc7), w(0x31), w(0xb1), w(0x12), w(0x10), \
        w(0x59), w(0x27), w(0x80), w(0xec), w(0x5f), w(0x60), w(0x51), \
        w(0x7f), w(0xa9), w(0x19), w(0xb5), w(0x4a), w(0x0d), w(0x2d), \
        w(0xe5), w(0x7a), w(0x9f), w(0x93), w(0xc9), w(0x9c), w(0xef), \
        w(0xa0), w(0xe0), w(0x3b), w(0x4d), w(0xae), w(0x2a), w(0xf5), \
        w(0xb0), w(0xc8), w(0xeb), w(0xbb), w(0x3c), w(0x83), w(0x53), \
        w(0x99), w(0x61), w(0x17), w(0x2b), w(0x04), w(0x7e), w(0xba), \
        w(0x77), w(0xd6), w(0x26), w(0xe1), w(0x69), w(0x14), w(0x63), \
        w(0x55), w(0x21), w(0x0c), w(0x7d)                             \
    }


#define SSE2NEON_AES_H0(x) (x)
static const uint8_t _sse2neon_sbox[256] = SSE2NEON_AES_SBOX(SSE2NEON_AES_H0);
static const uint8_t _sse2neon_rsbox[256] = SSE2NEON_AES_RSBOX(SSE2NEON_AES_H0);
#undef SSE2NEON_AES_H0


#if !defined(__aarch64__)
#define SSE2NEON_XT(x) (((x) << 1) ^ ((((x) >> 7) & 1) * 0x1b))
#define SSE2NEON_MULTIPLY(x, y)                                  \
    (((y & 1) * x) ^ ((y >> 1 & 1) * SSE2NEON_XT(x)) ^           \
     ((y >> 2 & 1) * SSE2NEON_XT(SSE2NEON_XT(x))) ^              \
     ((y >> 3 & 1) * SSE2NEON_XT(SSE2NEON_XT(SSE2NEON_XT(x)))) ^ \
     ((y >> 4 & 1) * SSE2NEON_XT(SSE2NEON_XT(SSE2NEON_XT(SSE2NEON_XT(x))))))
#endif


FORCE_INLINE __m128i _mm_aesenc_si128(__m128i a, __m128i RoundKey)
{
#if defined(__aarch64__)
    static const uint8_t shift_rows[] = {
        0x0, 0x5, 0xa, 0xf, 0x4, 0x9, 0xe, 0x3,
        0x8, 0xd, 0x2, 0x7, 0xc, 0x1, 0x6, 0xb,
    };
    static const uint8_t ror32by8[] = {
        0x1, 0x2, 0x3, 0x0, 0x5, 0x6, 0x7, 0x4,
        0x9, 0xa, 0xb, 0x8, 0xd, 0xe, 0xf, 0xc,
    };

    uint8x16_t v;
    uint8x16_t w = vreinterpretq_u8_m128i(a);


    w = vqtbl1q_u8(w, vld1q_u8(shift_rows));


    v = vqtbl4q_u8(_sse2neon_vld1q_u8_x4(_sse2neon_sbox), w);

    v = vqtbx4q_u8(v, _sse2neon_vld1q_u8_x4(_sse2neon_sbox + 0x40), w - 0x40);
    v = vqtbx4q_u8(v, _sse2neon_vld1q_u8_x4(_sse2neon_sbox + 0x80), w - 0x80);
    v = vqtbx4q_u8(v, _sse2neon_vld1q_u8_x4(_sse2neon_sbox + 0xc0), w - 0xc0);


    w = (v << 1) ^ (uint8x16_t) (((int8x16_t) v >> 7) & 0x1b);
    w ^= (uint8x16_t) vrev32q_u16((uint16x8_t) v);
    w ^= vqtbl1q_u8(v ^ w, vld1q_u8(ror32by8));


    return vreinterpretq_m128i_u8(w) ^ RoundKey;

#else
#define SSE2NEON_AES_B2W(b0, b1, b2, b3)                 \
    (((uint32_t) (b3) << 24) | ((uint32_t) (b2) << 16) | \
     ((uint32_t) (b1) << 8) | (uint32_t) (b0))

#define SSE2NEON_AES_F2(x) ((x << 1) ^ (((x >> 7) & 1) * 0x011b ))

#define SSE2NEON_AES_F3(x) (SSE2NEON_AES_F2(x) ^ x)
#define SSE2NEON_AES_U0(p) \
    SSE2NEON_AES_B2W(SSE2NEON_AES_F2(p), p, p, SSE2NEON_AES_F3(p))
#define SSE2NEON_AES_U1(p) \
    SSE2NEON_AES_B2W(SSE2NEON_AES_F3(p), SSE2NEON_AES_F2(p), p, p)
#define SSE2NEON_AES_U2(p) \
    SSE2NEON_AES_B2W(p, SSE2NEON_AES_F3(p), SSE2NEON_AES_F2(p), p)
#define SSE2NEON_AES_U3(p) \
    SSE2NEON_AES_B2W(p, p, SSE2NEON_AES_F3(p), SSE2NEON_AES_F2(p))


    static const uint32_t ALIGN_STRUCT(16) aes_table[4][256] = {
        SSE2NEON_AES_SBOX(SSE2NEON_AES_U0),
        SSE2NEON_AES_SBOX(SSE2NEON_AES_U1),
        SSE2NEON_AES_SBOX(SSE2NEON_AES_U2),
        SSE2NEON_AES_SBOX(SSE2NEON_AES_U3),
    };
#undef SSE2NEON_AES_B2W
#undef SSE2NEON_AES_F2
#undef SSE2NEON_AES_F3
#undef SSE2NEON_AES_U0
#undef SSE2NEON_AES_U1
#undef SSE2NEON_AES_U2
#undef SSE2NEON_AES_U3

    uint32_t x0 = _mm_cvtsi128_si32(a);
    uint32_t x1 =
        _mm_cvtsi128_si32(_mm_shuffle_epi32(a, 0x55));
    uint32_t x2 =
        _mm_cvtsi128_si32(_mm_shuffle_epi32(a, 0xAA));
    uint32_t x3 =
        _mm_cvtsi128_si32(_mm_shuffle_epi32(a, 0xFF));


    __m128i out = _mm_set_epi32(
        (aes_table[0][x3 & 0xff] ^ aes_table[1][(x0 >> 8) & 0xff] ^
         aes_table[2][(x1 >> 16) & 0xff] ^ aes_table[3][x2 >> 24]),
        (aes_table[0][x2 & 0xff] ^ aes_table[1][(x3 >> 8) & 0xff] ^
         aes_table[2][(x0 >> 16) & 0xff] ^ aes_table[3][x1 >> 24]),
        (aes_table[0][x1 & 0xff] ^ aes_table[1][(x2 >> 8) & 0xff] ^
         aes_table[2][(x3 >> 16) & 0xff] ^ aes_table[3][x0 >> 24]),
        (aes_table[0][x0 & 0xff] ^ aes_table[1][(x1 >> 8) & 0xff] ^
         aes_table[2][(x2 >> 16) & 0xff] ^ aes_table[3][x3 >> 24]));

    return _mm_xor_si128(out, RoundKey);
#endif
}


FORCE_INLINE __m128i _mm_aesdec_si128(__m128i a, __m128i RoundKey)
{
#if defined(__aarch64__)
    static const uint8_t inv_shift_rows[] = {
        0x0, 0xd, 0xa, 0x7, 0x4, 0x1, 0xe, 0xb,
        0x8, 0x5, 0x2, 0xf, 0xc, 0x9, 0x6, 0x3,
    };
    static const uint8_t ror32by8[] = {
        0x1, 0x2, 0x3, 0x0, 0x5, 0x6, 0x7, 0x4,
        0x9, 0xa, 0xb, 0x8, 0xd, 0xe, 0xf, 0xc,
    };

    uint8x16_t v;
    uint8x16_t w = vreinterpretq_u8_m128i(a);


    w = vqtbl1q_u8(w, vld1q_u8(inv_shift_rows));


    v = vqtbl4q_u8(_sse2neon_vld1q_u8_x4(_sse2neon_rsbox), w);
    v = vqtbx4q_u8(v, _sse2neon_vld1q_u8_x4(_sse2neon_rsbox + 0x40), w - 0x40);
    v = vqtbx4q_u8(v, _sse2neon_vld1q_u8_x4(_sse2neon_rsbox + 0x80), w - 0x80);
    v = vqtbx4q_u8(v, _sse2neon_vld1q_u8_x4(_sse2neon_rsbox + 0xc0), w - 0xc0);


    w = (v << 1) ^ (uint8x16_t) (((int8x16_t) v >> 7) & 0x1b);
    w = (w << 1) ^ (uint8x16_t) (((int8x16_t) w >> 7) & 0x1b);
    v ^= w;
    v ^= (uint8x16_t) vrev32q_u16((uint16x8_t) w);

    w = (v << 1) ^ (uint8x16_t) (((int8x16_t) v >> 7) &
                                 0x1b);
    w ^= (uint8x16_t) vrev32q_u16((uint16x8_t) v);
    w ^= vqtbl1q_u8(v ^ w, vld1q_u8(ror32by8));


    return vreinterpretq_m128i_u8(w) ^ RoundKey;

#else

    uint8_t i, e, f, g, h, v[4][4];
    uint8_t *_a = (uint8_t *) &a;
    for (i = 0; i < 16; ++i) {
        v[((i / 4) + (i % 4)) % 4][i % 4] = _sse2neon_rsbox[_a[i]];
    }


    for (i = 0; i < 4; ++i) {
        e = v[i][0];
        f = v[i][1];
        g = v[i][2];
        h = v[i][3];

        v[i][0] = SSE2NEON_MULTIPLY(e, 0x0e) ^ SSE2NEON_MULTIPLY(f, 0x0b) ^
                  SSE2NEON_MULTIPLY(g, 0x0d) ^ SSE2NEON_MULTIPLY(h, 0x09);
        v[i][1] = SSE2NEON_MULTIPLY(e, 0x09) ^ SSE2NEON_MULTIPLY(f, 0x0e) ^
                  SSE2NEON_MULTIPLY(g, 0x0b) ^ SSE2NEON_MULTIPLY(h, 0x0d);
        v[i][2] = SSE2NEON_MULTIPLY(e, 0x0d) ^ SSE2NEON_MULTIPLY(f, 0x09) ^
                  SSE2NEON_MULTIPLY(g, 0x0e) ^ SSE2NEON_MULTIPLY(h, 0x0b);
        v[i][3] = SSE2NEON_MULTIPLY(e, 0x0b) ^ SSE2NEON_MULTIPLY(f, 0x0d) ^
                  SSE2NEON_MULTIPLY(g, 0x09) ^ SSE2NEON_MULTIPLY(h, 0x0e);
    }

    return _mm_xor_si128(vreinterpretq_m128i_u8(vld1q_u8((uint8_t *) v)),
                         RoundKey);
#endif
}


FORCE_INLINE __m128i _mm_aesenclast_si128(__m128i a, __m128i RoundKey)
{
#if defined(__aarch64__)
    static const uint8_t shift_rows[] = {
        0x0, 0x5, 0xa, 0xf, 0x4, 0x9, 0xe, 0x3,
        0x8, 0xd, 0x2, 0x7, 0xc, 0x1, 0x6, 0xb,
    };

    uint8x16_t v;
    uint8x16_t w = vreinterpretq_u8_m128i(a);


    w = vqtbl1q_u8(w, vld1q_u8(shift_rows));


    v = vqtbl4q_u8(_sse2neon_vld1q_u8_x4(_sse2neon_sbox), w);
    v = vqtbx4q_u8(v, _sse2neon_vld1q_u8_x4(_sse2neon_sbox + 0x40), w - 0x40);
    v = vqtbx4q_u8(v, _sse2neon_vld1q_u8_x4(_sse2neon_sbox + 0x80), w - 0x80);
    v = vqtbx4q_u8(v, _sse2neon_vld1q_u8_x4(_sse2neon_sbox + 0xc0), w - 0xc0);


    return vreinterpretq_m128i_u8(v) ^ RoundKey;

#else
    uint8_t v[16] = {
        _sse2neon_sbox[vgetq_lane_u8(vreinterpretq_u8_m128i(a), 0)],
        _sse2neon_sbox[vgetq_lane_u8(vreinterpretq_u8_m128i(a), 5)],
        _sse2neon_sbox[vgetq_lane_u8(vreinterpretq_u8_m128i(a), 10)],
        _sse2neon_sbox[vgetq_lane_u8(vreinterpretq_u8_m128i(a), 15)],
        _sse2neon_sbox[vgetq_lane_u8(vreinterpretq_u8_m128i(a), 4)],
        _sse2neon_sbox[vgetq_lane_u8(vreinterpretq_u8_m128i(a), 9)],
        _sse2neon_sbox[vgetq_lane_u8(vreinterpretq_u8_m128i(a), 14)],
        _sse2neon_sbox[vgetq_lane_u8(vreinterpretq_u8_m128i(a), 3)],
        _sse2neon_sbox[vgetq_lane_u8(vreinterpretq_u8_m128i(a), 8)],
        _sse2neon_sbox[vgetq_lane_u8(vreinterpretq_u8_m128i(a), 13)],
        _sse2neon_sbox[vgetq_lane_u8(vreinterpretq_u8_m128i(a), 2)],
        _sse2neon_sbox[vgetq_lane_u8(vreinterpretq_u8_m128i(a), 7)],
        _sse2neon_sbox[vgetq_lane_u8(vreinterpretq_u8_m128i(a), 12)],
        _sse2neon_sbox[vgetq_lane_u8(vreinterpretq_u8_m128i(a), 1)],
        _sse2neon_sbox[vgetq_lane_u8(vreinterpretq_u8_m128i(a), 6)],
        _sse2neon_sbox[vgetq_lane_u8(vreinterpretq_u8_m128i(a), 11)],
    };

    return _mm_xor_si128(vreinterpretq_m128i_u8(vld1q_u8(v)), RoundKey);
#endif
}


FORCE_INLINE __m128i _mm_aesdeclast_si128(__m128i a, __m128i RoundKey)
{
#if defined(__aarch64__)
    static const uint8_t inv_shift_rows[] = {
        0x0, 0xd, 0xa, 0x7, 0x4, 0x1, 0xe, 0xb,
        0x8, 0x5, 0x2, 0xf, 0xc, 0x9, 0x6, 0x3,
    };

    uint8x16_t v;
    uint8x16_t w = vreinterpretq_u8_m128i(a);


    w = vqtbl1q_u8(w, vld1q_u8(inv_shift_rows));


    v = vqtbl4q_u8(_sse2neon_vld1q_u8_x4(_sse2neon_rsbox), w);
    v = vqtbx4q_u8(v, _sse2neon_vld1q_u8_x4(_sse2neon_rsbox + 0x40), w - 0x40);
    v = vqtbx4q_u8(v, _sse2neon_vld1q_u8_x4(_sse2neon_rsbox + 0x80), w - 0x80);
    v = vqtbx4q_u8(v, _sse2neon_vld1q_u8_x4(_sse2neon_rsbox + 0xc0), w - 0xc0);


    return vreinterpretq_m128i_u8(v) ^ RoundKey;

#else

    uint8_t v[4][4];
    uint8_t *_a = (uint8_t *) &a;
    for (int i = 0; i < 16; ++i) {
        v[((i / 4) + (i % 4)) % 4][i % 4] = _sse2neon_rsbox[_a[i]];
    }

    return _mm_xor_si128(vreinterpretq_m128i_u8(vld1q_u8((uint8_t *) v)),
                         RoundKey);
#endif
}


FORCE_INLINE __m128i _mm_aesimc_si128(__m128i a)
{
#if defined(__aarch64__)
    static const uint8_t ror32by8[] = {
        0x1, 0x2, 0x3, 0x0, 0x5, 0x6, 0x7, 0x4,
        0x9, 0xa, 0xb, 0x8, 0xd, 0xe, 0xf, 0xc,
    };
    uint8x16_t v = vreinterpretq_u8_m128i(a);
    uint8x16_t w;


    w = (v << 1) ^ (uint8x16_t) (((int8x16_t) v >> 7) & 0x1b);
    w = (w << 1) ^ (uint8x16_t) (((int8x16_t) w >> 7) & 0x1b);
    v ^= w;
    v ^= (uint8x16_t) vrev32q_u16((uint16x8_t) w);


    w = (v << 1) ^ (uint8x16_t) (((int8x16_t) v >> 7) & 0x1b);
    w ^= (uint8x16_t) vrev32q_u16((uint16x8_t) v);
    w ^= vqtbl1q_u8(v ^ w, vld1q_u8(ror32by8));
    return vreinterpretq_m128i_u8(w);

#else
    uint8_t i, e, f, g, h, v[4][4];
    vst1q_u8((uint8_t *) v, vreinterpretq_u8_m128i(a));
    for (i = 0; i < 4; ++i) {
        e = v[i][0];
        f = v[i][1];
        g = v[i][2];
        h = v[i][3];

        v[i][0] = SSE2NEON_MULTIPLY(e, 0x0e) ^ SSE2NEON_MULTIPLY(f, 0x0b) ^
                  SSE2NEON_MULTIPLY(g, 0x0d) ^ SSE2NEON_MULTIPLY(h, 0x09);
        v[i][1] = SSE2NEON_MULTIPLY(e, 0x09) ^ SSE2NEON_MULTIPLY(f, 0x0e) ^
                  SSE2NEON_MULTIPLY(g, 0x0b) ^ SSE2NEON_MULTIPLY(h, 0x0d);
        v[i][2] = SSE2NEON_MULTIPLY(e, 0x0d) ^ SSE2NEON_MULTIPLY(f, 0x09) ^
                  SSE2NEON_MULTIPLY(g, 0x0e) ^ SSE2NEON_MULTIPLY(h, 0x0b);
        v[i][3] = SSE2NEON_MULTIPLY(e, 0x0b) ^ SSE2NEON_MULTIPLY(f, 0x0d) ^
                  SSE2NEON_MULTIPLY(g, 0x09) ^ SSE2NEON_MULTIPLY(h, 0x0e);
    }

    return vreinterpretq_m128i_u8(vld1q_u8((uint8_t *) v));
#endif
}


FORCE_INLINE __m128i _mm_aeskeygenassist_si128(__m128i a, const int rcon)
{
#if defined(__aarch64__)
    uint8x16_t _a = vreinterpretq_u8_m128i(a);
    uint8x16_t v = vqtbl4q_u8(_sse2neon_vld1q_u8_x4(_sse2neon_sbox), _a);
    v = vqtbx4q_u8(v, _sse2neon_vld1q_u8_x4(_sse2neon_sbox + 0x40), _a - 0x40);
    v = vqtbx4q_u8(v, _sse2neon_vld1q_u8_x4(_sse2neon_sbox + 0x80), _a - 0x80);
    v = vqtbx4q_u8(v, _sse2neon_vld1q_u8_x4(_sse2neon_sbox + 0xc0), _a - 0xc0);

    uint32x4_t v_u32 = vreinterpretq_u32_u8(v);
    uint32x4_t ror_v = vorrq_u32(vshrq_n_u32(v_u32, 8), vshlq_n_u32(v_u32, 24));
    uint32x4_t ror_xor_v = veorq_u32(ror_v, vdupq_n_u32(rcon));

    return vreinterpretq_m128i_u32(vtrn2q_u32(v_u32, ror_xor_v));

#else
    uint32_t X1 = _mm_cvtsi128_si32(_mm_shuffle_epi32(a, 0x55));
    uint32_t X3 = _mm_cvtsi128_si32(_mm_shuffle_epi32(a, 0xFF));
    for (int i = 0; i < 4; ++i) {
        ((uint8_t *) &X1)[i] = _sse2neon_sbox[((uint8_t *) &X1)[i]];
        ((uint8_t *) &X3)[i] = _sse2neon_sbox[((uint8_t *) &X3)[i]];
    }
    return _mm_set_epi32(((X3 >> 8) | (X3 << 24)) ^ rcon, X3,
                         ((X1 >> 8) | (X1 << 24)) ^ rcon, X1);
#endif
}
#undef SSE2NEON_AES_SBOX
#undef SSE2NEON_AES_RSBOX

#if defined(__aarch64__)
#undef SSE2NEON_XT
#undef SSE2NEON_MULTIPLY
#endif

#else


FORCE_INLINE __m128i _mm_aesenc_si128(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_u8(veorq_u8(
        vaesmcq_u8(vaeseq_u8(vreinterpretq_u8_m128i(a), vdupq_n_u8(0))),
        vreinterpretq_u8_m128i(b)));
}


FORCE_INLINE __m128i _mm_aesdec_si128(__m128i a, __m128i RoundKey)
{
    return vreinterpretq_m128i_u8(veorq_u8(
        vaesimcq_u8(vaesdq_u8(vreinterpretq_u8_m128i(a), vdupq_n_u8(0))),
        vreinterpretq_u8_m128i(RoundKey)));
}


FORCE_INLINE __m128i _mm_aesenclast_si128(__m128i a, __m128i RoundKey)
{
    return _mm_xor_si128(vreinterpretq_m128i_u8(vaeseq_u8(
                             vreinterpretq_u8_m128i(a), vdupq_n_u8(0))),
                         RoundKey);
}


FORCE_INLINE __m128i _mm_aesdeclast_si128(__m128i a, __m128i RoundKey)
{
    return vreinterpretq_m128i_u8(
        veorq_u8(vaesdq_u8(vreinterpretq_u8_m128i(a), vdupq_n_u8(0)),
                 vreinterpretq_u8_m128i(RoundKey)));
}


FORCE_INLINE __m128i _mm_aesimc_si128(__m128i a)
{
    return vreinterpretq_m128i_u8(vaesimcq_u8(vreinterpretq_u8_m128i(a)));
}


FORCE_INLINE __m128i _mm_aeskeygenassist_si128(__m128i a, const int rcon)
{

    uint8x16_t u8 = vaeseq_u8(vreinterpretq_u8_m128i(a), vdupq_n_u8(0));

#if !defined(_MSC_VER) || defined(__clang__)
    uint8x16_t dest = {

        u8[0x4], u8[0x1], u8[0xE], u8[0xB],
        u8[0x1], u8[0xE], u8[0xB], u8[0x4],
        u8[0xC], u8[0x9], u8[0x6], u8[0x3],
        u8[0x9], u8[0x6], u8[0x3], u8[0xC],
    };
    uint32x4_t r = {0, (unsigned) rcon, 0, (unsigned) rcon};
    return vreinterpretq_m128i_u8(dest) ^ vreinterpretq_m128i_u32(r);
#else


    __n128 dest{
        ((uint64_t) u8.n128_u8[0x4] << 0) | ((uint64_t) u8.n128_u8[0x1] << 8) |
            ((uint64_t) u8.n128_u8[0xE] << 16) |
            ((uint64_t) u8.n128_u8[0xB] << 24) |
            ((uint64_t) u8.n128_u8[0x1] << 32) |
            ((uint64_t) u8.n128_u8[0xE] << 40) |
            ((uint64_t) u8.n128_u8[0xB] << 48) |
            ((uint64_t) u8.n128_u8[0x4] << 56),
        ((uint64_t) u8.n128_u8[0xC] << 0) | ((uint64_t) u8.n128_u8[0x9] << 8) |
            ((uint64_t) u8.n128_u8[0x6] << 16) |
            ((uint64_t) u8.n128_u8[0x3] << 24) |
            ((uint64_t) u8.n128_u8[0x9] << 32) |
            ((uint64_t) u8.n128_u8[0x6] << 40) |
            ((uint64_t) u8.n128_u8[0x3] << 48) |
            ((uint64_t) u8.n128_u8[0xC] << 56)};

    dest.n128_u32[1] = dest.n128_u32[1] ^ rcon;
    dest.n128_u32[3] = dest.n128_u32[3] ^ rcon;

    return dest;
#endif
}
#endif


FORCE_INLINE __m128i _mm_clmulepi64_si128(__m128i _a, __m128i _b, const int imm)
{
    uint64x2_t a = vreinterpretq_u64_m128i(_a);
    uint64x2_t b = vreinterpretq_u64_m128i(_b);
    switch (imm & 0x11) {
    case 0x00:
        return vreinterpretq_m128i_u64(
            _sse2neon_vmull_p64(vget_low_u64(a), vget_low_u64(b)));
    case 0x01:
        return vreinterpretq_m128i_u64(
            _sse2neon_vmull_p64(vget_high_u64(a), vget_low_u64(b)));
    case 0x10:
        return vreinterpretq_m128i_u64(
            _sse2neon_vmull_p64(vget_low_u64(a), vget_high_u64(b)));
    case 0x11:
        return vreinterpretq_m128i_u64(
            _sse2neon_vmull_p64(vget_high_u64(a), vget_high_u64(b)));
    default:
        abort();
    }
}

FORCE_INLINE unsigned int _sse2neon_mm_get_denormals_zero_mode(void)
{
    union {
        fpcr_bitfield field;
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
        uint64_t value;
#else
        uint32_t value;
#endif
    } r;

#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    r.value = _sse2neon_get_fpcr();
#else
    __asm__ __volatile__("vmrs %0, FPSCR" : "=r"(r.value));
#endif

    return r.field.bit24 ? _MM_DENORMALS_ZERO_ON : _MM_DENORMALS_ZERO_OFF;
}


FORCE_INLINE int _mm_popcnt_u32(unsigned int a)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
#if __has_builtin(__builtin_popcount)
    return __builtin_popcount(a);
#elif defined(_MSC_VER)
    return _CountOneBits(a);
#else
    return (int) vaddlv_u8(vcnt_u8(vcreate_u8((uint64_t) a)));
#endif
#else
    uint32_t count = 0;
    uint8x8_t input_val, count8x8_val;
    uint16x4_t count16x4_val;
    uint32x2_t count32x2_val;

    input_val = vld1_u8((uint8_t *) &a);
    count8x8_val = vcnt_u8(input_val);
    count16x4_val = vpaddl_u8(count8x8_val);
    count32x2_val = vpaddl_u16(count16x4_val);

    vst1_u32(&count, count32x2_val);
    return count;
#endif
}


FORCE_INLINE int64_t _mm_popcnt_u64(uint64_t a)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
#if __has_builtin(__builtin_popcountll)
    return __builtin_popcountll(a);
#elif defined(_MSC_VER)
    return _CountOneBits64(a);
#else
    return (int64_t) vaddlv_u8(vcnt_u8(vcreate_u8(a)));
#endif
#else
    uint64_t count = 0;
    uint8x8_t input_val, count8x8_val;
    uint16x4_t count16x4_val;
    uint32x2_t count32x2_val;
    uint64x1_t count64x1_val;

    input_val = vld1_u8((uint8_t *) &a);
    count8x8_val = vcnt_u8(input_val);
    count16x4_val = vpaddl_u8(count8x8_val);
    count32x2_val = vpaddl_u16(count16x4_val);
    count64x1_val = vpaddl_u32(count32x2_val);
    vst1_u64(&count, count64x1_val);
    return count;
#endif
}

FORCE_INLINE void _sse2neon_mm_set_denormals_zero_mode(unsigned int flag)
{


    union {
        fpcr_bitfield field;
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
        uint64_t value;
#else
        uint32_t value;
#endif
    } r;

#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    r.value = _sse2neon_get_fpcr();
#else
    __asm__ __volatile__("vmrs %0, FPSCR" : "=r"(r.value));
#endif

    r.field.bit24 = (flag & _MM_DENORMALS_ZERO_MASK) == _MM_DENORMALS_ZERO_ON;

#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    _sse2neon_set_fpcr(r.value);
#else
    __asm__ __volatile__("vmsr FPSCR, %0" ::"r"(r));
#endif
}


FORCE_INLINE uint64_t _rdtsc(void)
{
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    uint64_t val;


#if defined(_MSC_VER) && !defined(__clang__)
    val = _ReadStatusReg(ARM64_SYSREG(3, 3, 14, 0, 2));
#else
    __asm__ __volatile__("mrs %0, cntvct_el0" : "=r"(val));
#endif

    return val;
#else
    uint32_t pmccntr, pmuseren, pmcntenset;


    __asm__ __volatile__("mrc p15, 0, %0, c9, c14, 0" : "=r"(pmuseren));
    if (pmuseren & 1) {
        __asm__ __volatile__("mrc p15, 0, %0, c9, c12, 1" : "=r"(pmcntenset));
        if (pmcntenset & 0x80000000UL) {
            __asm__ __volatile__("mrc p15, 0, %0, c9, c13, 0" : "=r"(pmccntr));

            return (uint64_t) (pmccntr) << 6;
        }
    }


    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t) (tv.tv_sec) * 1000000 + tv.tv_usec;
#endif
}

#if defined(__GNUC__) || defined(__clang__)
#pragma pop_macro("ALIGN_STRUCT")
#pragma pop_macro("FORCE_INLINE")
#endif

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC pop_options
#endif

#endif
