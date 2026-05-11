#include "crypto/verus_hash.h"

int __cpuverusoptimized = 0x80;

thread_local thread_specific_ptr verusclhasher_key;
thread_local thread_specific_ptr verusclhasher_descr;

#if defined(__APPLE__) || defined(_WIN32)
thread_specific_ptr::~thread_specific_ptr()
{
    if (verusclhasher_key.ptr)
    {
        verusclhasher_key.reset();
    }
    if (verusclhasher_descr.ptr)
    {
        verusclhasher_descr.reset();
    }
}
#endif

void *alloc_aligned_buffer(uint64_t bufSize)
{
    void *answer = NULL;
    if (posix_memalign(&answer, sizeof(__m128i) * 2, bufSize))
    {
        return NULL;
    }
    return answer;
}
