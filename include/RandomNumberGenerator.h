#ifndef RANDOMNUMBERGENERATOR_H
#define RANDOMNUMBERGENERATOR_H

#include <array>
#include <cstdint>
#include <vector>

class RandomNumberGenerator {
   public:
    static int getRandomNumberBelow(int maximum) {
        return xoshiro128p() % maximum;
    }

    static float getRandomFloatBetweenZeroAndOne() {
        return xoshiro128p() / 4294967295.0f;
    }

   private:
    static uint32_t xoshiro128p() {
        static std::array<uint32_t, 4> s = {2, 1, 1, 1};  // Example seed, should be non-zero
        const uint32_t result = s[0] + s[3];
        const uint32_t t = s[1] << 9;

        s[2] ^= s[0];
        s[3] ^= s[1];
        s[1] ^= s[2];
        s[0] ^= s[3];

        s[2] ^= t;
        s[3] = rotl(s[3], 11);

        return result >> 1;
    }

    static constexpr uint32_t rotl(const uint32_t x, int k) {
        return (x << k) | (x >> (32 - k));
    }
};

#endif  // RANDOMNUMBERGENERATOR_H
