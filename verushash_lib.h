#ifndef VERUSHASH_LIB_H
#define VERUSHASH_LIB_H

#include <stddef.h>

#ifdef _WIN32
#ifdef VERUSHASH_LIB_BUILD
#define VERUSHASH_API __declspec(dllexport)
#else
#define VERUSHASH_API __declspec(dllimport)
#endif
#else
#if defined(__GNUC__) || defined(__clang__)
#define VERUSHASH_API __attribute__((visibility("default")))
#else
#define VERUSHASH_API
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define VERUSHASH_OUTPUT_SIZE 32

enum verushash_solution_version
{
    VERUSHASH_SOLUTION_V2 = 1,
    VERUSHASH_SOLUTION_V2_1 = 3,
    VERUSHASH_SOLUTION_V2_2 = 4
};

enum verushash_status
{
    VERUSHASH_SUCCESS = 0,
    VERUSHASH_ERROR_INVALID_ARGUMENT = 1,
    VERUSHASH_ERROR_INPUT_TOO_LARGE = 2,
    VERUSHASH_ERROR_UNSUPPORTED_VERSION = 3,
    VERUSHASH_ERROR_INTERNAL = 4
};

VERUSHASH_API size_t verushash_output_size(void);
VERUSHASH_API int verushash_latest_solution_version(void);
VERUSHASH_API int verushash_hash(const unsigned char *input, size_t input_len, unsigned char *output32);
VERUSHASH_API int verushash_hash_v2_2(const unsigned char *input, size_t input_len, unsigned char *output32);
VERUSHASH_API int verushash_hash_v2_1(const unsigned char *input, size_t input_len, unsigned char *output32);
VERUSHASH_API int verushash_hash_v2b(const unsigned char *input, size_t input_len, unsigned char *output32);
VERUSHASH_API int verushash_hash_v2(const unsigned char *input, size_t input_len, unsigned char *output32);
VERUSHASH_API int verushash_hash_v1(const unsigned char *input, size_t input_len, unsigned char *output32);
VERUSHASH_API int verushash_hash_v2b_version(const unsigned char *input, size_t input_len, unsigned char *output32, int solution_version);

#ifdef __cplusplus
}
#endif

#endif
