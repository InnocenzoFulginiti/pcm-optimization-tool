//
// Created by Jakob on 02/02/2023.
//

#include <catch2/catch_test_macros.hpp>

#include "UnionTable.hpp"
#include "Definitions.hpp"

TEST_CASE("Make a Union Table") {
    UnionTable ut(3);
    REQUIRE(ut.size() == 3);
}

TEST_CASE("Test combine Method") {
    UnionTable table(3);

    table.combine(0, 1);

    REQUIRE(table[0] == table[1]);
    REQUIRE(table[1] != table[2]);
}

TEST_CASE("Test UnionTable::canActivate with example") {
    //See if controls {1, 2, 7, 4} are correctly checked in union table with
    //{1, 3, 4} and {2, 5, 7} as entangled groups

    UnionTable ut(8);

    ut[0] = TOP::T;
    ut[6] = TOP::T;

    QubitState state1(3);
    state1.clear();
    //Is able to activate:
    state1[5] = 1;

    QubitState state2(3);
    state2.clear();
    state2[5] = 1;

    ut[1] = ut[3] = ut[4] = std::make_shared<QubitState>(state1);
    ut[2] = ut[5] = ut[7] = std::make_shared<QubitState>(state2);

    CHECK(ut.canActivate(std::vector<size_t>{1, 2, 7, 4}));
}