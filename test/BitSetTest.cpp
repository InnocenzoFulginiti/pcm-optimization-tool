#include "TestUtils.hpp"

TEST_CASE("Test BitSet not constant methods", "[BitSet]") {
    int a = GENERATE(take(20, random(0, 100000)));
    int b = GENERATE(take(20, random(0, 100000)));
    int length = 5;
    a &= (1 << length) - 1;
    b &= (1 << length) - 1;

    BitSet bitSet(static_cast<size_t>(length), static_cast<size_t>(a));

    BitSet copy(bitSet);

    REQUIRE(copy == bitSet);

    bitSet &= BitSet(b);

    REQUIRE(bitSet == BitSet(static_cast<size_t>(length), static_cast<size_t>(a & b)));
}

TEST_CASE("Test BitSet minus", "[BitSet]") {
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

TEST_CASE("Random s1&~s1 == 0", "[BitSet]") {
    size_t s1 = static_cast<size_t>(GENERATE(take(10, random(1, 4))));

    CAPTURE(s1, ~s1, s1 & ~s1);

    BitSet bs1(s1);

    CAPTURE(bs1);

    BitSet nbs1 = ~bs1;

    CAPTURE(nbs1);

    BitSet andbs1 = bs1 & nbs1;

    CAPTURE(andbs1);

    REQUIRE(andbs1 == BitSet(0));
}

TEST_CASE("Random s1 ^ 0 = s1", "[BitSet]") {
    size_t A = static_cast<size_t>(GENERATE(take(10, random(1, 4))));
    size_t B = static_cast<size_t>(GENERATE(take(10, random(1, 4))));

    auto length = GENERATE(take(10, random(static_cast<long long>(1), static_cast<long long>(8 * sizeof(size_t)))));


    A &= (static_cast<size_t>(1) << length) - 1;
    B &= (static_cast<size_t>(1) << length) - 1;

    BitSet bsA(A);
    BitSet bsB(B);

    CAPTURE(A, B, length, bsA, bsB);

    SECTION("A ^ 0 = A") {
        BitSet res = bsA ^ 0;
        REQUIRE(bsA == res);
    }

    SECTION("~(A & B) == ~A | ~B") {
        REQUIRE((~(bsA & bsB)) == (~bsA | ~bsB));
    }

    SECTION("~(~A) == A") {
        REQUIRE((~(~bsA)) == bsA);
    }

    SECTION("A | 0 == A") {
        REQUIRE((bsA | 0) == bsA);
    }

    SECTION("A & 0 == 0") {
        REQUIRE((bsA & 0) == 0);
    }

    SECTION("A & ~0 == A") {
        REQUIRE((bsA & ~0) == bsA);
    }

    SECTION("A | ~0 == ~0") {
        REQUIRE((bsA | ~0) == ~0);
    }
}
