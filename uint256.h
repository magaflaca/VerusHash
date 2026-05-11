

#ifndef VERUSHASH_STANDALONE_UINT256_H
#define VERUSHASH_STANDALONE_UINT256_H

#include <cstdint>
#include <cstring>

class uint256
{
public:
    alignas(uint32_t) uint8_t data[32];

    uint8_t *begin() { return data; }
    uint8_t *end() { return data + 32; }
    const uint8_t *begin() const { return data; }
    const uint8_t *end() const { return data + 32; }
    unsigned int size() const { return 32; }

    void SetNull() { std::memset(data, 0, sizeof(data)); }
    bool IsNull() const
    {
        for (uint8_t b : data)
        {
            if (b != 0) return false;
        }
        return true;
    }

    friend inline bool operator==(const uint256 &a, const uint256 &b)
    {
        return std::memcmp(a.data, b.data, sizeof(a.data)) == 0;
    }

    friend inline bool operator!=(const uint256 &a, const uint256 &b)
    {
        return !(a == b);
    }
};

#endif
