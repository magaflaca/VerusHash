


#ifndef INCLUDE_VERUS_CLHASH_H
#define INCLUDE_VERUS_CLHASH_H


#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <errno.h>

#ifdef _WIN32
#undef __cpuid
#include <intrin.h>
#include <malloc.h>
#endif

#if defined(__arm__) || defined(__aarch64__) || defined(_M_ARM) || defined(_M_ARM64) || defined(_M_ARM) || defined(_M_ARM64) || defined(_M_ARM) || defined(_M_ARM64)
#if !defined(__clang__) && defined(__GNUC__) && __GNUC__ < 10
#include "crypto/compat/sse2neon.h"
#else
#include "crypto/sse2neon.h"
#endif
#if !defined(__APPLE__) && !defined(_WIN32)
#include <sys/auxv.h>
#include <asm/hwcap.h>
#endif
#else
#if defined(_MSC_VER)
#include <immintrin.h>
#ifndef bit_PCLMUL
#define bit_PCLMUL (1 << 1)
#endif
#ifndef bit_AES
#define bit_AES (1 << 25)
#endif
#ifndef bit_AVX
#define bit_AVX (1 << 28)
#endif
#else
#include <cpuid.h>
#include <x86intrin.h>
#endif
#endif


#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define posix_memalign(p, a, s) (((*(p)) = _aligned_malloc((s), (a))), *(p) ?0 :errno)
typedef unsigned char u_char;
#endif

enum {


    VERUSKEYSIZE=1024 * 8 + (40 * 16),
    SOLUTION_VERUSHHASH_V2 = 1,
    SOLUTION_VERUSHHASH_V2_1 = 3,
    SOLUTION_VERUSHHASH_V2_2 = 4
};

struct verusclhash_descr
{
    uint256 seed;
    uint32_t keySizeInBytes;
    uint32_t solutionVersion;
    uint32_t seedInitialized;
};

struct thread_specific_ptr {
    void *ptr;
    thread_specific_ptr() { ptr = nullptr; }
    void reset(void *newptr = nullptr)
    {
        if (ptr && ptr != newptr)
        {
#if defined(_WIN32)
            _aligned_free(ptr);
#else
            std::free(ptr);
#endif
        }
        ptr = newptr;

    }
    void *get() { return ptr; }
#if defined(__APPLE__) || defined(_WIN32)

    ~thread_specific_ptr();
#else
    ~thread_specific_ptr() {
        this->reset();
    }
#endif
};

extern thread_local thread_specific_ptr verusclhasher_key;
extern thread_local thread_specific_ptr verusclhasher_descr;

extern int __cpuverusoptimized;

__m128i __verusclmulwithoutreduction64alignedrepeat(__m128i *randomsource, const __m128i buf[4], uint64_t keyMask, __m128i **pMoveScratch);
__m128i __verusclmulwithoutreduction64alignedrepeat_sv2_1(__m128i *randomsource, const __m128i buf[4], uint64_t keyMask, __m128i **pMoveScratch);
__m128i __verusclmulwithoutreduction64alignedrepeat_sv2_2(__m128i *randomsource, const __m128i buf[4], uint64_t keyMask, __m128i **pMoveScratch);
__m128i __verusclmulwithoutreduction64alignedrepeat_port(__m128i *randomsource, const __m128i buf[4], uint64_t keyMask, __m128i **pMoveScratch);
__m128i __verusclmulwithoutreduction64alignedrepeat_sv2_1_port(__m128i *randomsource, const __m128i buf[4], uint64_t keyMask, __m128i **pMoveScratch);
__m128i __verusclmulwithoutreduction64alignedrepeat_sv2_2_port(__m128i *randomsource, const __m128i buf[4], uint64_t keyMask, __m128i **pMoveScratch);

inline bool IsCPUVerusOptimized()
{
#if defined(VERUSHASH_PORTABLE_ONLY)
    __cpuverusoptimized = false;
#else
    #if defined(__arm__) || defined(__aarch64__) || defined(_M_ARM) || defined(_M_ARM64) || defined(_M_ARM) || defined(_M_ARM64) || defined(_M_ARM) || defined(_M_ARM64)
    #if defined(__APPLE__)
    __cpuverusoptimized = true;
#elif defined(_WIN32)
    __cpuverusoptimized = false;
#else
    long hwcaps= getauxval(AT_HWCAP);

    if((hwcaps & HWCAP_AES) && (hwcaps & HWCAP_PMULL))
        __cpuverusoptimized = true;
    else
        __cpuverusoptimized = false;
#endif
    #else
    if (__cpuverusoptimized & 0x80)
    {
#if defined(_MSC_VER)
        int cpuInfo[4] = {0, 0, 0, 0};
        __cpuid(cpuInfo, 1);
        unsigned int ecx = (unsigned int)cpuInfo[2];
        __cpuverusoptimized = ((ecx & (bit_AVX | bit_AES | bit_PCLMUL)) == (bit_AVX | bit_AES | bit_PCLMUL));
#else
        unsigned int eax,ebx,ecx,edx;
        if (!__get_cpuid(1,&eax,&ebx,&ecx,&edx))
        {
            __cpuverusoptimized = false;
        }
        else
        {
            __cpuverusoptimized = ((ecx & (bit_AVX | bit_AES | bit_PCLMUL)) == (bit_AVX | bit_AES | bit_PCLMUL));
        }
#endif
    }
    #endif
#endif
    return __cpuverusoptimized;
};

inline void ForceCPUVerusOptimized(bool trueorfalse)
{
    __cpuverusoptimized = trueorfalse;
};

uint64_t verusclhash(void * random, const unsigned char buf[64], uint64_t keyMask, __m128i **pMoveScratch);
uint64_t verusclhash_port(void * random, const unsigned char buf[64], uint64_t keyMask, __m128i **pMoveScratch);
uint64_t verusclhash_sv2_1(void * random, const unsigned char buf[64], uint64_t keyMask, __m128i **pMoveScratch);
uint64_t verusclhash_sv2_2(void * random, const unsigned char buf[64], uint64_t keyMask, __m128i **pMoveScratch);
uint64_t verusclhash_sv2_1_port(void * random, const unsigned char buf[64], uint64_t keyMask, __m128i **pMoveScratch);
uint64_t verusclhash_sv2_2_port(void * random, const unsigned char buf[64], uint64_t keyMask, __m128i **pMoveScratch);
void *alloc_aligned_buffer(uint64_t bufSize);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include <vector>
#include <string>
#include <iostream>


struct verusclhasher {
    uint64_t keySizeInBytes;
    uint64_t keyMask;
    uint64_t (*verusclhashfunction)(void * random, const unsigned char buf[64], uint64_t keyMask, __m128i **pMoveScratch);
    __m128i (*verusinternalclhashfunction)(__m128i *randomsource, const __m128i buf[4], uint64_t keyMask, __m128i **pMoveScratch);

    static inline uint64_t keymask(uint64_t keysize)
    {
        int i = 0;
        while (keysize >>= 1)
        {
            i++;
        }
        return i ? (((uint64_t)1) << i) - 1 : 0;
    }


    verusclhasher(uint64_t keysize=VERUSKEYSIZE, int solutionVersion=SOLUTION_VERUSHHASH_V2) : keySizeInBytes((keysize >> 5) << 5)
    {
#if !defined(VERUSHASH_PORTABLE_ONLY)
        if (IsCPUVerusOptimized())
        {
            if (solutionVersion >= SOLUTION_VERUSHHASH_V2_1)
            {
                if (solutionVersion >= SOLUTION_VERUSHHASH_V2_2)
                {
                    verusclhashfunction = &verusclhash_sv2_2;
                    verusinternalclhashfunction = &__verusclmulwithoutreduction64alignedrepeat_sv2_2;
                }
                else
                {
                    verusclhashfunction = &verusclhash_sv2_1;
                    verusinternalclhashfunction = &__verusclmulwithoutreduction64alignedrepeat_sv2_1;
                }
            }
            else
            {
                verusclhashfunction = &verusclhash;
                verusinternalclhashfunction = &__verusclmulwithoutreduction64alignedrepeat;
            }
        }
        else
#endif
        {
            if (solutionVersion >= SOLUTION_VERUSHHASH_V2_1)
            {
                if (solutionVersion >= SOLUTION_VERUSHHASH_V2_2)
                {
                    verusclhashfunction = &verusclhash_sv2_2_port;
                    verusinternalclhashfunction = &__verusclmulwithoutreduction64alignedrepeat_sv2_2_port;
                }
                else
                {
                    verusclhashfunction = &verusclhash_sv2_1_port;
                    verusinternalclhashfunction = &__verusclmulwithoutreduction64alignedrepeat_sv2_1_port;
                }
            }
            else
            {
                verusclhashfunction = &verusclhash_port;
                verusinternalclhashfunction = &__verusclmulwithoutreduction64alignedrepeat_port;
            }
        }


        if (verusclhasher_key.get())
        {
            verusclhash_descr *existingDesc = (verusclhash_descr *)verusclhasher_descr.get();
            if (keySizeInBytes != existingDesc->keySizeInBytes ||
                (uint32_t)solutionVersion != existingDesc->solutionVersion)
            {
                verusclhasher_key.reset();
                verusclhasher_descr.reset();
            }
        }

        void *key = NULL;
        if (!(key = verusclhasher_key.get()) &&
            (verusclhasher_key.reset((unsigned char *)alloc_aligned_buffer(keySizeInBytes << 1)), key = verusclhasher_key.get()))
        {
            verusclhash_descr *pdesc;
            if (verusclhasher_descr.reset((unsigned char *)alloc_aligned_buffer(sizeof(verusclhash_descr))), pdesc = (verusclhash_descr *)verusclhasher_descr.get())
            {
                pdesc->keySizeInBytes = keySizeInBytes;
                pdesc->solutionVersion = (uint32_t)solutionVersion;
                pdesc->seedInitialized = 0;
                pdesc->seed.SetNull();
            }
            else
            {
                verusclhasher_key.reset();
                key = NULL;
            }
        }
        if (key)
        {
            keyMask = keymask(keySizeInBytes);
        }
        else
        {
            keyMask = 0;
            keySizeInBytes = 0;
        }
#ifdef VERUSHASHDEBUG
        printf("New hasher, keyMask: %lx, newKeySize: %lx\n", keyMask, keySizeInBytes);
#endif
    }

    inline void *gethasherrefresh()
    {
        verusclhash_descr *pdesc = (verusclhash_descr *)verusclhasher_descr.get();
        return (unsigned char *)verusclhasher_key.get() + pdesc->keySizeInBytes;
    }


    inline __m128i **getpmovescratch(void *hasherrefresh)
    {
        return (__m128i **)((unsigned char *)hasherrefresh + keyrefreshsize());
    }

    inline verusclhash_descr *gethasherdescription() const
    {
        return (verusclhash_descr *)verusclhasher_descr.get();
    }

    inline uint64_t keyrefreshsize() const
    {
        return keyMask + 1;
    }

    inline void *fixupkey(void *hashKey, verusclhash_descr &desc)
    {
        unsigned char *ret = (unsigned char *)hashKey;
        uint32_t ofs = desc.keySizeInBytes >> 4;
        __m128i **ppfixup = getpmovescratch(ret + desc.keySizeInBytes);
        for (__m128i *pfixup = *ppfixup; pfixup; pfixup = *++ppfixup)
        {
            *pfixup = *(pfixup + ofs);
        }
        return hashKey;
    }


    inline void *gethashkey()
    {
        unsigned char *ret = (unsigned char *)verusclhasher_key.get();
        return fixupkey(ret, *(verusclhash_descr *)verusclhasher_descr.get());
    }

    inline uint64_t operator()(const unsigned char buf[64]) const {
        unsigned char *pkey = (unsigned char *)verusclhasher_key.get();
        verusclhash_descr *pdesc = (verusclhash_descr *)verusclhasher_descr.get();
        return (*verusclhashfunction)(pkey, buf, keyMask, (__m128i **)(pkey + (pdesc->keySizeInBytes + keyrefreshsize())));
    }

    inline uint64_t operator()(const unsigned char buf[64], void *pkey) const {
        verusclhash_descr *pdesc = (verusclhash_descr *)verusclhasher_descr.get();
        return (*verusclhashfunction)(pkey, buf, keyMask, (__m128i **)((unsigned char *)pkey + (pdesc->keySizeInBytes + keyrefreshsize())));
    }

    inline uint64_t operator()(const unsigned char buf[64], void *pkey, __m128i **pMoveScratch) const {
        return (*verusclhashfunction)((unsigned char *)pkey, buf, keyMask, pMoveScratch);
    }
};

#endif

#endif
