//
// Created by Jakob on 02/02/2023.
//

#include "TestUtils.hpp"

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

TEST_CASE("Test UnionTable::minimizeControls Example 1") {
    UnionTable ut(5);
    //Entangle All
    ut[4] = ut[3] = ut[2] = ut[1] = ut[0];

    auto qs = ut[0].getQubitState();
    qs->clear();
    //Values so that 4 => 3, 2, 0
    (*qs)[0b00000] = 1;
    (*qs)[0b00001] = 1;
    (*qs)[0b00100] = 1;
    (*qs)[0b00101] = 1;
    (*qs)[0b01101] = 1;
    (*qs)[0b11101] = 1;

    auto ret = ut.minimizeControls({0, 2, 3, 4});
    auto min = ret.second;
    //Only "4" should be left
    CHECK(ret.second.size() == 1);
    CHECK(std::find(min.begin(), min.end(), 4) != min.end());
}

TEST_CASE("Test UnionTable::minimizeControls Example 2") {
    UnionTable ut(5);
    //Entangle All
    ut[4] = ut[3] = ut[2] = ut[1] = ut[0];

    auto qs = ut[0].getQubitState();
    qs->clear();
    //Values so that 4 => 3, 2 => 0, no other implication
    (*qs)[1] = 1;
    (*qs)[2] = 1;
    (*qs)[5] = 1;
    (*qs)[8] = 1;
    (*qs)[24] = 1;

    auto ret = ut.minimizeControls({0, 2, 3, 4});
    auto min = ret.second;

    //Only "4 & 2" should be left
    CHECK(ret.second.size() == 2);
    CHECK(std::find(min.begin(), min.end(), 4) != min.end());
    CHECK(std::find(min.begin(), min.end(), 2) != min.end());
}

TEST_CASE("Test UnionTable::minimizeControls <=>") {
    UnionTable ut(5);
    //Entangle All
    ut[4] = ut[3] = ut[2] = ut[1] = ut[0];

    auto qs = ut[0].getQubitState();
    qs->clear();
    //Values so that 4 => 3, 4 <= 3,0 => 2, no others
    (*qs)[0b00000] = 1;
    (*qs)[0b00010] = 1;
    (*qs)[0b00100] = 1;
    (*qs)[0b00110] = 1;
    (*qs)[0b00111] = 1;
    (*qs)[0b11000] = 1;
    (*qs)[0b11010] = 1;
    (*qs)[0b11100] = 1;
    (*qs)[0b11101] = 1;
    (*qs)[0b11110] = 1;
    (*qs)[0b11111] = 1;

    auto [act, min] = ut.minimizeControls({0, 2, 3, 4});

    //Only "4/3, 0" should be left
    CHECK(min.size() == 2);

    CHECK(std::find(min.begin(), min.end(), 0) != min.end());

    bool has3 = std::find(min.begin(), min.end(), 3) != min.end();
    bool has4 = std::find(min.begin(), min.end(), 4) != min.end();

    CHECK((has3 || has4));
}

TEST_CASE("Test UnionTable::minimizeControls") {
    UnionTable ut(6);
    ut[0] = TOP::T;
    ut[1].getQubitState()->clear();
    (*ut[1].getQubitState())[0] = qc::SQRT_2_2;
    (*ut[1].getQubitState())[1] = qc::SQRT_2_2;

    ut[3] = ut[2];
    ut[3].getQubitState()->clear();
    (*ut[3].getQubitState())[2] = qc::SQRT_2_2;
    (*ut[3].getQubitState())[3] = qc::SQRT_2_2;

    ut[4] = ut[5];
    ut[4].getQubitState()->clear();
    (*ut[4].getQubitState())[0] = qc::SQRT_2_2;
    (*ut[4].getQubitState())[3] = qc::SQRT_2_2;

    auto res = ut.minimizeControls({0, 1, 2, 3, 4, 5});

    CHECK(res.second.size() == 4);
}