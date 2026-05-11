

#ifndef HARAKA_H_
#define HARAKA_H_

#if defined(__arm__) || defined(__aarch64__) || defined(_M_ARM) || defined(_M_ARM64) || defined(_M_ARM) || defined(_M_ARM64) || defined(_M_ARM) || defined(_M_ARM64)
#if !defined(__clang__) && defined(__GNUC__) && __GNUC__ < 10
#include "crypto/compat/sse2neon.h"
#else
#include "crypto/sse2neon.h"
#endif
#else
#include "immintrin.h"
#endif

#define NUMROUNDS 5

#ifdef _WIN32
typedef unsigned long long u64;
#else
typedef unsigned long u64;
#endif
typedef __m128i u128;

extern u128 rc[40];

#define LOAD(src) _mm_load_si128((u128 *)(src))
#define STORE(dest,src) _mm_storeu_si128((u128 *)(dest),src)

#define AES2(s0, s1, rci) \
  s0 = _mm_aesenc_si128(s0, rc[rci]); \
  s1 = _mm_aesenc_si128(s1, rc[rci + 1]); \
  s0 = _mm_aesenc_si128(s0, rc[rci + 2]); \
  s1 = _mm_aesenc_si128(s1, rc[rci + 3]);

#define AES2_4x(s0, s1, s2, s3, rci) \
  AES2(s0[0], s0[1], rci); \
  AES2(s1[0], s1[1], rci); \
  AES2(s2[0], s2[1], rci); \
  AES2(s3[0], s3[1], rci);

#define AES2_8x(s0, s1, s2, s3, s4, s5, s6, s7, rci) \
  AES2_4x(s0, s1, s2, s3, rci); \
  AES2_4x(s4, s5, s6, s7, rci);

#define AES4(s0, s1, s2, s3, rci) \
  s0 = _mm_aesenc_si128(s0, rc[rci]); \
  s1 = _mm_aesenc_si128(s1, rc[rci + 1]); \
  s2 = _mm_aesenc_si128(s2, rc[rci + 2]); \
  s3 = _mm_aesenc_si128(s3, rc[rci + 3]); \
  s0 = _mm_aesenc_si128(s0, rc[rci + 4]); \
  s1 = _mm_aesenc_si128(s1, rc[rci + 5]); \
  s2 = _mm_aesenc_si128(s2, rc[rci + 6]); \
  s3 = _mm_aesenc_si128(s3, rc[rci + 7]); \

#define AES4_zero(s0, s1, s2, s3, rci) \
  s0 = _mm_aesenc_si128(s0, rc0[rci]); \
  s1 = _mm_aesenc_si128(s1, rc0[rci + 1]); \
  s2 = _mm_aesenc_si128(s2, rc0[rci + 2]); \
  s3 = _mm_aesenc_si128(s3, rc0[rci + 3]); \
  s0 = _mm_aesenc_si128(s0, rc0[rci + 4]); \
  s1 = _mm_aesenc_si128(s1, rc0[rci + 5]); \
  s2 = _mm_aesenc_si128(s2, rc0[rci + 6]); \
  s3 = _mm_aesenc_si128(s3, rc0[rci + 7]); \

#define AES4_4x(s0, s1, s2, s3, rci) \
  AES4(s0[0], s0[1], s0[2], s0[3], rci); \
  AES4(s1[0], s1[1], s1[2], s1[3], rci); \
  AES4(s2[0], s2[1], s2[2], s2[3], rci); \
  AES4(s3[0], s3[1], s3[2], s3[3], rci);

#define AES4_8x(s0, s1, s2, s3, s4, s5, s6, s7, rci) \
  AES4_4x(s0, s1, s2, s3, rci); \
  AES4_4x(s4, s5, s6, s7, rci);

#define MIX2(s0, s1) \
  tmp = _mm_unpacklo_epi32(s0, s1); \
  s1 = _mm_unpackhi_epi32(s0, s1); \
  s0 = tmp;

#define MIX4(s0, s1, s2, s3) \
  tmp  = _mm_unpacklo_epi32(s0, s1); \
  s0 = _mm_unpackhi_epi32(s0, s1); \
  s1 = _mm_unpacklo_epi32(s2, s3); \
  s2 = _mm_unpackhi_epi32(s2, s3); \
  s3 = _mm_unpacklo_epi32(s0, s2); \
  s0 = _mm_unpackhi_epi32(s0, s2); \
  s2 = _mm_unpackhi_epi32(s1, tmp); \
  s1 = _mm_unpacklo_epi32(s1, tmp);

#define TRUNCSTORE(out, s0, s1, s2, s3) \
  *(u64*)(out) = *(((u64*)&s0 + 1)); \
  *(u64*)(out + 8) = *(((u64*)&s1 + 1)); \
  *(u64*)(out + 16) = *(((u64*)&s2 + 0)); \
  *(u64*)(out + 24) = *(((u64*)&s3 + 0));

void load_constants();
void test_implementations();

void load_constants();

void haraka256(unsigned char *out, const unsigned char *in);
void haraka256_keyed(unsigned char *out, const unsigned char *in, const u128 *rc);
void haraka256_4x(unsigned char *out, const unsigned char *in);
void haraka256_8x(unsigned char *out, const unsigned char *in);

void haraka512(unsigned char *out, const unsigned char *in);
void haraka512_zero(unsigned char *out, const unsigned char *in);
void haraka512_keyed(unsigned char *out, const unsigned char *in, const u128 *rc);
void haraka512_4x(unsigned char *out, const unsigned char *in);
void haraka512_8x(unsigned char *out, const unsigned char *in);

#endif
