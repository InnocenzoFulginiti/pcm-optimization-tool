#include "TestUtils.hpp"

TEST_CASE("Test BitSet not constant methods", "[BitSet]") {
    int a = GENERATE(take(20, random(0, 100000)));
    int b = GENERATE(take(20, random(0, 100000)));
    int length = 5;
    a &= (1 << length) - 1;
    b &= (1 << length) - 1;

    auto size = static_cast<size_t>(length);

    BitSet bitSet(size, static_cast<size_t>(a));

    BitSet copy(size, bitSet);

    REQUIRE(copy == bitSet);

    bitSet &= BitSet(size, static_cast<size_t>(b));

    REQUIRE(bitSet == BitSet(size, static_cast<size_t>(a & b)));
}

TEST_CASE("Test Bitset operator <", "[BitSet]") {
    size_t a = static_cast<size_t>(GENERATE(take(5, random(0, 100000))));
    size_t b = static_cast<size_t>(GENERATE(take(5, random(0, 100000))));

    size_t la = static_cast<size_t>(GENERATE(take(3, random(0, 255))));
    size_t lb = static_cast<size_t>(GENERATE(take(3, random(0, 255))));

    a &= (static_cast<size_t>(1) << la) - 1;
    b &= (static_cast<size_t>(1) << lb) - 1;

    BitSet bitSetA(la, a);
    BitSet bitSetB(lb, b);

    CAPTURE(a, b, a < b, a > b, bitSetA, bitSetB, la, lb);

    REQUIRE((bitSetA < bitSetB) == (a < b));
    REQUIRE((bitSetA > bitSetB) == (a > b));
}

TEST_CASE("Test BitSet operator- same length", "[BitSet]") {
    size_t b = static_cast<size_t>(GENERATE(take(20, random(0, 10000))));

    size_t a = b | static_cast<size_t>(GENERATE(take(20, random(0, 10000))));

    CAPTURE(a, b, a - b);

    size_t length = 8 * sizeof(size_t);

    BitSet expected(length, a - b);
    BitSet aBitSet(length, a);
    BitSet bBitSet(length, b);

    CAPTURE(aBitSet, bBitSet, expected);

    BitSet result = aBitSet - bBitSet;

    CAPTURE(aBitSet, bBitSet, result, expected);

    REQUIRE(expected == result);
}

TEST_CASE("Test BitSet operator- different length", "[BitSet]") {
    size_t a = static_cast<size_t>(GENERATE(take(10, random(0, 10000))));
    size_t b = static_cast<size_t>(GENERATE(take(10, random(0, 10000))));


    size_t la = static_cast<size_t>(GENERATE(take(3, random(0, 256)))) % 8 * sizeof(size_t);
    size_t lb = static_cast<size_t>(GENERATE(take(3, random(0, 256)))) % 8 * sizeof(size_t);

    a &= (static_cast<size_t>(1) << la) - 1;
    b &= (static_cast<size_t>(1) << lb) - 1;


    size_t bigger = std::max(a, b);
    size_t smaller = std::min(a, b);

    CAPTURE(bigger, smaller, bigger - smaller);


    BitSet expected(bigger - smaller);


}

TEST_CASE("Random s1&~s1 == 0", "[BitSet]") {
    size_t s1 = static_cast<size_t>(GENERATE(take(10, random(1, 4))));
    size_t l = 3;

    CAPTURE(s1, ~s1, s1 & ~s1);

    BitSet bs1(l, s1);

    CAPTURE(bs1);

    BitSet nbs1 = ~bs1;

    CAPTURE(nbs1);

    BitSet andbs1 = bs1 & nbs1;

    CAPTURE(andbs1);

    REQUIRE(andbs1 == BitSet(l, 0));
}

TEST_CASE("BitSet - Basic Tests", "[BitSet]") {
    BitSet a(32, 2);
    BitSet b(32, 2);

    CAPTURE(a, b);
    REQUIRE(a == b);

    BitSet c = a & b;
    REQUIRE(c == a);

    BitSet d = a | b;
    REQUIRE(d == a);
}

TEST_CASE("BitSet - Random Relationships", "[BitSet]") {
    size_t A = static_cast<size_t>(GENERATE(take(5, random(1, 7))));
    size_t B = static_cast<size_t>(GENERATE(take(5, random(1, 7))));

    auto length = GENERATE(take(3, random(1, 5)));


    A &= (static_cast<size_t>(1) << length) - 1;
    B &= (static_cast<size_t>(1) << length) - 1;

    auto size = static_cast<size_t>(length);

    BitSet bsA(size, A);
    BitSet bsB(size, B);

    CAPTURE(A, B, length, bsA, bsB);

    SECTION("A ^ 0 = A") {
        BitSet res = bsA ^ BitSet(size, 0);
        REQUIRE(bsA == res);
    }

    SECTION("~(A & B) == ~A | ~B") {
        BitSet ret1 = ~(bsA & bsB);
        BitSet ret2 = ~bsA | ~bsB;
        REQUIRE(ret1 == ret2);
    }

    SECTION("~(~A) == A") {
        BitSet ret1 = ~(~bsA);
        REQUIRE(ret1 == bsA);
    }

    SECTION("A | 0 == A") {
        REQUIRE((bsA | BitSet(size, 0)) == bsA);
    }

    SECTION("A & 0 == 0") {
        REQUIRE((bsA & BitSet(size, 0)) == BitSet(size, 0));
    }

    SECTION("A & ~0 == A") {
        REQUIRE((bsA & ~BitSet(size, 0)) == bsA);
    }

    SECTION("A | ~0 == ~0") {
        REQUIRE((bsA | ~BitSet(size, 0)) == ~BitSet(size, 0));
    }
}

TEST_CASE("BitSet Random equals/neq Test", "[BitSet]") {
    size_t A = static_cast<size_t>(GENERATE(take(6, random(1, 10000))));
    size_t B = static_cast<size_t>(GENERATE(take(6, random(1, 10000))));

    size_t la = static_cast<size_t>(GENERATE(take(2, random(0, 256)))) % 8 * sizeof(size_t);
    size_t lb = static_cast<size_t>(GENERATE(take(2, random(0, 256)))) % 8 * sizeof(size_t);


    A &= (static_cast<size_t>(1) << la) - 1;
    B &= (static_cast<size_t>(1) << lb) - 1;

    CAPTURE(A, B, la, lb, A == B, A != B);

    BitSet bsA(la, A);
    BitSet bsB(lb, B);

    CAPTURE(bsA, bsB);

    REQUIRE(bsA == bsA);
    REQUIRE((bsA == bsB) == (A == B));
    REQUIRE((bsA != bsB) == (A != B));
}

TEST_CASE("BitSet Constructor", "[BitSet]") {
    unsigned int a = static_cast<unsigned int>(GENERATE(take(10, random(0, 10000))));

    BitSet bs(32, a);

    auto s = bs.to_string();

    CAPTURE(a, bs, s);

    REQUIRE(s == std::bitset<32>(a).to_string());
}

TEST_CASE("BitSet int Constructor", "[BitSet]") {
    int a = GENERATE(take(10, random(0, 10000)));

    BitSet bs(a);

    auto s = bs.to_string();
    auto expected = std::bitset<sizeof(int)*8>(static_cast<unsigned int>(a)).to_string();

    CAPTURE(a, bs, s, expected);

    REQUIRE(s == expected);
}

TEST_CASE("BitSet - Should Throw", "[BitSet]") {
    size_t a = static_cast<size_t>(GENERATE(take(5, random(0, 10000))));
    size_t b = static_cast<size_t>(GENERATE(take(5, random(0, 10000))));

    size_t la = static_cast<size_t>(GENERATE(take(3, random(0, 255)))) % 8 * sizeof(size_t);
    size_t lb = la == 0 ? 1 : la - 1;

    a &= (static_cast<size_t>(1) << la) - 1;
    b &= (static_cast<size_t>(1) << lb) - 1;

    CAPTURE(a, b, la, lb);

    //Throw if int is negative

    REQUIRE_THROWS_AS(BitSet(-1), std::invalid_argument);
}

TEST_CASE("BitSet shift", "[BitSet]") {
    size_t a = static_cast<size_t>(GENERATE(take(10, random(0, 10))));
    size_t la = static_cast<size_t>(GENERATE(take(3, random(0, 255)))) % (8 * sizeof(size_t));

    a &= (static_cast<size_t>(1) << la) - 1;

    CAPTURE(a, la);

    BitSet as(la, a);

    CAPTURE(as);

    size_t shiftLeft = static_cast<size_t>(GENERATE(take(5, random(0, 255)))) % (8 * sizeof(size_t) - la);
    size_t shiftRight = static_cast<size_t>(GENERATE(take(5, random(0, 50))));

    CAPTURE(shiftLeft, shiftRight);

    BitSet left = as << shiftLeft;
    BitSet right = as >> shiftRight;

    size_t leftA = a << shiftLeft;
    size_t rightA = a >> shiftRight;

    CAPTURE(left, right, leftA, rightA);

    REQUIRE(left == BitSet(leftA));
    REQUIRE(right == BitSet(rightA));
}

TEST_CASE("BitSet setSize", "[BitSet]") {
    size_t a = static_cast<size_t>(GENERATE(take(10, random(0, 10000))));
    size_t la = static_cast<size_t>(GENERATE(take(3, random(0, 255)))) % (8 * sizeof(size_t)) ;

    a &= (static_cast<size_t>(1) << la) - 1;

    CAPTURE(a, la);

    BitSet as(la, a);

    size_t newSize = static_cast<size_t>(GENERATE(take(10, random(0, 10000)))) % (8 * sizeof(size_t) - la);

    CAPTURE(newSize);

    as.setSize(newSize);
    a &= (static_cast<size_t>(1) << newSize) - 1;

    REQUIRE(as == BitSet(newSize, a));
}