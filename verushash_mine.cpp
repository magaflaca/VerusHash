#include "verushash_lib.h"

#include <cstdint>
#include <cstring>
#include <vector>

#include "uint256.h"
extern "C" {
#include "crypto/haraka_portable.h"
}
#include "crypto/verus_clhash.h"
#include "crypto/verus_hash.h"

namespace
{
constexpr size_t kEquihashHeaderSize = 140;
constexpr size_t kMinSolutionSize = 33;
constexpr size_t kMaxSolutionSize = 4096;

bool le256_less_or_equal(const unsigned char *value, const unsigned char *target)
{
    for (int i = 31; i >= 0; --i)
    {
        if (value[i] < target[i])
            return true;
        if (value[i] > target[i])
            return false;
    }
    return true;
}

size_t compact_size_len(size_t value)
{
    if (value < 253)
        return 1;
    if (value <= 0xffff)
        return 3;
    return 5;
}

void write_compact_size(unsigned char *out, size_t value)
{
    if (value < 253)
    {
        out[0] = static_cast<unsigned char>(value);
        return;
    }
    if (value <= 0xffff)
    {
        out[0] = 0xfd;
        out[1] = static_cast<unsigned char>(value & 0xff);
        out[2] = static_cast<unsigned char>((value >> 8) & 0xff);
        return;
    }
    out[0] = 0xfe;
    out[1] = static_cast<unsigned char>(value & 0xff);
    out[2] = static_cast<unsigned char>((value >> 8) & 0xff);
    out[3] = static_cast<unsigned char>((value >> 16) & 0xff);
    out[4] = static_cast<unsigned char>((value >> 24) & 0xff);
}

size_t solution_extra_offset(size_t solution_len)
{
    return solution_len - ((solution_len % 32) + 15);
}

void write_i64_le(unsigned char *out, uint64_t value)
{
    for (int i = 0; i < 8; ++i)
        out[i] = static_cast<unsigned char>((value >> (i * 8)) & 0xff);
}

int validate_scan_args(const unsigned char *header140,
                       size_t header_len,
                       const unsigned char *solution,
                       size_t solution_len,
                       const unsigned char *target32_le,
                       unsigned char *output_solution,
                       unsigned char *output_hash32,
                       uint64_t *found_nonce)
{
    if (header140 == nullptr || solution == nullptr || target32_le == nullptr || output_solution == nullptr || output_hash32 == nullptr || found_nonce == nullptr)
        return VERUSHASH_ERROR_INVALID_ARGUMENT;
    if (header_len != kEquihashHeaderSize)
        return VERUSHASH_ERROR_INVALID_ARGUMENT;
    if (solution_len < kMinSolutionSize || solution_len > kMaxSolutionSize)
        return VERUSHASH_ERROR_INVALID_ARGUMENT;
    if (solution_extra_offset(solution_len) + 8 > solution_len)
        return VERUSHASH_ERROR_INVALID_ARGUMENT;
    return VERUSHASH_SUCCESS;
}

int scan_impl(const unsigned char *header140,
              size_t header_len,
              const unsigned char *solution,
              size_t solution_len,
              const unsigned char *target32_le,
              uint64_t start_nonce,
              uint64_t max_nonce_count,
              uint64_t *hashes_done,
              uint64_t *found_nonce,
              unsigned char *output_solution,
              unsigned char *output_hash32,
              int solution_version)
{
    const int arg_status = validate_scan_args(header140, header_len, solution, solution_len, target32_le, output_solution, output_hash32, found_nonce);
    if (arg_status != VERUSHASH_SUCCESS)
        return arg_status;

    if (hashes_done != nullptr)
        *hashes_done = 0;

    if (max_nonce_count == 0)
        return VERUSHASH_SCAN_NOT_FOUND;

    if (solution_version != VERUSHASH_SOLUTION_V2 && solution_version != VERUSHASH_SOLUTION_V2_1 && solution_version != VERUSHASH_SOLUTION_V2_2)
        return VERUSHASH_ERROR_UNSUPPORTED_VERSION;

    try
    {
        CVerusHash::init();
        CVerusHashV2::init();

        const size_t compact_len = compact_size_len(solution_len);
        std::vector<unsigned char> serialized(kEquihashHeaderSize + compact_len + solution_len);
        std::memcpy(serialized.data(), header140, kEquihashHeaderSize);
        write_compact_size(serialized.data() + kEquihashHeaderSize, solution_len);
        std::memcpy(serialized.data() + kEquihashHeaderSize + compact_len, solution, solution_len);

        CVerusHashV2 vh(solution_version);
        verusclhasher &vclh = vh.vclh;

        vh.Reset();
        vh.Write(serialized.data(), serialized.size());

        int64_t *extra_ptr = vh.ExtraI64Ptr();
        unsigned char *cur_buf = vh.CurBuffer();

        u128 *hash_key = reinterpret_cast<u128 *>(verusclhasher_key.get());
        verusclhash_descr *desc = reinterpret_cast<verusclhash_descr *>(verusclhasher_descr.get());
        if (hash_key == nullptr || desc == nullptr)
            return VERUSHASH_ERROR_INTERNAL;

        const uint32_t key_size = desc->keySizeInBytes;
        void *hasher_refresh = reinterpret_cast<unsigned char *>(hash_key) + key_size;
        __m128i **move_scratch = vclh.getpmovescratch(hasher_refresh);
        const int key_refresh_size = static_cast<int>(vclh.keyrefreshsize());

        if (desc->seed != *reinterpret_cast<uint256 *>(cur_buf))
        {
            const int blocks = static_cast<int>(key_size >> 5);
            unsigned char *key_out = reinterpret_cast<unsigned char *>(hash_key);
            unsigned char *source = cur_buf;
            for (int i = 0; i < blocks; ++i)
            {
                haraka256_port(key_out, source);
                source = key_out;
                key_out += 32;
            }
            desc->seed = *reinterpret_cast<uint256 *>(cur_buf);
            std::memcpy(hasher_refresh, hash_key, static_cast<size_t>(key_refresh_size));
            std::memset(reinterpret_cast<unsigned char *>(hasher_refresh) + key_refresh_size, 0, key_size - static_cast<uint32_t>(key_refresh_size));
        }
        else
        {
            vclh.gethashkey();
        }

        unsigned char hash[32];
        uint64_t done = 0;
        const size_t extra_offset = solution_extra_offset(solution_len);

        for (uint64_t i = 0; i < max_nonce_count; ++i)
        {
            const uint64_t nonce = start_nonce + i;
            *extra_ptr = static_cast<int64_t>(nonce);

            vh.FillExtra(reinterpret_cast<u128 *>(cur_buf));
            const uint64_t intermediate = vclh(cur_buf, hash_key, move_scratch);
            vh.FillExtra(&intermediate);
            (*vh.haraka512KeyedFunction)(hash, cur_buf, hash_key + vh.IntermediateTo128Offset(intermediate));

            ++done;

            if (le256_less_or_equal(hash, target32_le))
            {
                std::memcpy(output_solution, solution, solution_len);
                write_i64_le(output_solution + extra_offset, nonce);
                std::memcpy(output_hash32, hash, 32);
                *found_nonce = nonce;
                if (hashes_done != nullptr)
                    *hashes_done = done;
                return VERUSHASH_SCAN_FOUND;
            }

            vclh.fixupkey(hash_key, *desc);
        }

        if (hashes_done != nullptr)
            *hashes_done = done;
        return VERUSHASH_SCAN_NOT_FOUND;
    }
    catch (...)
    {
        return VERUSHASH_ERROR_INTERNAL;
    }
}
}

extern "C"
{

VERUSHASH_API size_t verushash_equihash_header_size(void)
{
    return kEquihashHeaderSize;
}

VERUSHASH_API size_t verushash_min_solution_size(void)
{
    return kMinSolutionSize;
}

VERUSHASH_API size_t verushash_max_solution_size(void)
{
    return kMaxSolutionSize;
}

VERUSHASH_API size_t verushash_solution_nonce_offset(size_t solution_len)
{
    if (solution_len < kMinSolutionSize || solution_len > kMaxSolutionSize)
        return 0;
    return solution_extra_offset(solution_len);
}

VERUSHASH_API int verushash_scan_v2_2(const unsigned char *header140,
                                      size_t header_len,
                                      const unsigned char *solution,
                                      size_t solution_len,
                                      const unsigned char *target32_le,
                                      uint64_t start_nonce,
                                      uint64_t max_nonce_count,
                                      uint64_t *hashes_done,
                                      uint64_t *found_nonce,
                                      unsigned char *output_solution,
                                      unsigned char *output_hash32)
{
    return scan_impl(header140, header_len, solution, solution_len, target32_le, start_nonce, max_nonce_count, hashes_done, found_nonce, output_solution, output_hash32, VERUSHASH_SOLUTION_V2_2);
}

VERUSHASH_API int verushash_scan_v2b_version(const unsigned char *header140,
                                             size_t header_len,
                                             const unsigned char *solution,
                                             size_t solution_len,
                                             const unsigned char *target32_le,
                                             uint64_t start_nonce,
                                             uint64_t max_nonce_count,
                                             uint64_t *hashes_done,
                                             uint64_t *found_nonce,
                                             unsigned char *output_solution,
                                             unsigned char *output_hash32,
                                             int solution_version)
{
    return scan_impl(header140, header_len, solution, solution_len, target32_le, start_nonce, max_nonce_count, hashes_done, found_nonce, output_solution, output_hash32, solution_version);
}

}
