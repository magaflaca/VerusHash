#include "verushash_lib.h"

#include <climits>
#include <cstddef>
#include <exception>
#include <mutex>

#include "crypto/verus_hash.h"

namespace
{
std::once_flag g_init_flag;

void initialize_verushash_core()
{
    CVerusHash::init();
    CVerusHashV2::init();
}

int validate_args(const unsigned char *input, size_t input_len, unsigned char *output32)
{
    if (output32 == nullptr)
    {
        return VERUSHASH_ERROR_INVALID_ARGUMENT;
    }
    if (input_len != 0 && input == nullptr)
    {
        return VERUSHASH_ERROR_INVALID_ARGUMENT;
    }
    if (input_len > static_cast<size_t>(INT_MAX))
    {

        return VERUSHASH_ERROR_INPUT_TOO_LARGE;
    }
    return VERUSHASH_SUCCESS;
}

int hash_v2b_impl(const unsigned char *input,
                  size_t input_len,
                  unsigned char *output32,
                  int solution_version)
{
    const int arg_status = validate_args(input, input_len, output32);
    if (arg_status != VERUSHASH_SUCCESS)
    {
        return arg_status;
    }

    if (solution_version != VERUSHASH_SOLUTION_V2 &&
        solution_version != VERUSHASH_SOLUTION_V2_1 &&
        solution_version != VERUSHASH_SOLUTION_V2_2)
    {
        return VERUSHASH_ERROR_UNSUPPORTED_VERSION;
    }

    try
    {
        std::call_once(g_init_flag, initialize_verushash_core);
        CVerusHashV2 hasher(solution_version);
        hasher.Write(input, input_len);
        hasher.Finalize2b(output32);
        return VERUSHASH_SUCCESS;
    }
    catch (...)
    {
        return VERUSHASH_ERROR_INTERNAL;
    }
}
}

extern "C"
{

VERUSHASH_API size_t verushash_output_size(void)
{
    return VERUSHASH_OUTPUT_SIZE;
}

VERUSHASH_API int verushash_latest_solution_version(void)
{
    return VERUSHASH_SOLUTION_V2_2;
}

VERUSHASH_API int verushash_hash(const unsigned char *input,
                                 size_t input_len,
                                 unsigned char *output32)
{
    return hash_v2b_impl(input, input_len, output32, VERUSHASH_SOLUTION_V2_2);
}

VERUSHASH_API int verushash_hash_v2_2(const unsigned char *input,
                                      size_t input_len,
                                      unsigned char *output32)
{
    return hash_v2b_impl(input, input_len, output32, VERUSHASH_SOLUTION_V2_2);
}

VERUSHASH_API int verushash_hash_v2_1(const unsigned char *input,
                                      size_t input_len,
                                      unsigned char *output32)
{
    return hash_v2b_impl(input, input_len, output32, VERUSHASH_SOLUTION_V2_1);
}

VERUSHASH_API int verushash_hash_v2b(const unsigned char *input,
                                     size_t input_len,
                                     unsigned char *output32)
{
    return hash_v2b_impl(input, input_len, output32, VERUSHASH_SOLUTION_V2);
}

VERUSHASH_API int verushash_hash_v2(const unsigned char *input,
                                    size_t input_len,
                                    unsigned char *output32)
{
    const int arg_status = validate_args(input, input_len, output32);
    if (arg_status != VERUSHASH_SUCCESS)
    {
        return arg_status;
    }

    try
    {
        std::call_once(g_init_flag, initialize_verushash_core);
        CVerusHashV2 hasher;
        hasher.Write(input, input_len);
        hasher.Finalize(output32);
        return VERUSHASH_SUCCESS;
    }
    catch (...)
    {
        return VERUSHASH_ERROR_INTERNAL;
    }
}

VERUSHASH_API int verushash_hash_v1(const unsigned char *input,
                                    size_t input_len,
                                    unsigned char *output32)
{
    const int arg_status = validate_args(input, input_len, output32);
    if (arg_status != VERUSHASH_SUCCESS)
    {
        return arg_status;
    }

    try
    {
        std::call_once(g_init_flag, initialize_verushash_core);
        CVerusHash hasher;
        hasher.Write(input, input_len);
        hasher.Finalize(output32);
        return VERUSHASH_SUCCESS;
    }
    catch (...)
    {
        return VERUSHASH_ERROR_INTERNAL;
    }
}

VERUSHASH_API int verushash_hash_v2b_version(const unsigned char *input,
                                             size_t input_len,
                                             unsigned char *output32,
                                             int solution_version)
{
    return hash_v2b_impl(input, input_len, output32, solution_version);
}

}
