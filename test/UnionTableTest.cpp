//
// Created by Jakob on 02/02/2023.
//

#include <catch2/catch_test_macros.hpp>

#include "UnionTable.hpp"
#include "Definitions.hpp"

SCENARIO("Make a Union Table") {
    UnionTable ut(3);
    REQUIRE(ut.size() == 3);
}

SCENARIO("Test combine Method") {
    UnionTable table(3);

    table.combine(0, 1);

    REQUIRE(table[0] == table[1]);
    REQUIRE(table[1] != table[2]);

    auto state0 = std::get<std::shared_ptr<QubitState>>(table[0]);
    auto state1 = std::get<std::shared_ptr<QubitState>>(table[1]);
    auto state2 = std::get<std::shared_ptr<QubitState>>(table[2]);


}