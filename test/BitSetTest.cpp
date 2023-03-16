#include "TestUtils.hpp"

TEST_CASE("Test BitSet minus") {
    SECTION("Test Random minus") {
        size_t a = static_cast<size_t>(GENERATE(take(20, random(0, 100000))));
        size_t b = static_cast<size_t>(GENERATE(take(20, random(0, 100000))));

        CAPTURE(a, b, a - b);

        size_t length = 8 * sizeof(size_t);

        BitSet expected(length, a - b);
        BitSet aBitSet(length, a);
        BitSet bBitSet(length, b);

        BitSet result = aBitSet - bBitSet;

        REQUIRE(expected == result);
    }

    SECTION("Test Random minus with different length") {
        size_t a = static_cast<size_t>(GENERATE(take(20, random(0, 100000))));
        size_t b = static_cast<size_t>(GENERATE(take(20, random(0, 100000))));

        CAPTURE(a, b, a - b);

        size_t length = 8 * sizeof(size_t);

        BitSet expected(length, a - b);
        BitSet aBitSet(length, a);

        size_t lDiff = static_cast<size_t>(GENERATE(take(5, random(0, 100000)))) % length;

        BitSet bBitSet(length - lDiff, b);

        BitSet result = aBitSet - bBitSet;

        REQUIRE(expected == result);
    }
}
