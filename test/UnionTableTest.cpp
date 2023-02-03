//
// Created by Jakob on 02/02/2023.
//

#include <catch2/catch_test_macros.hpp>

#include "UnionTable.hpp"

SCENARIO("Make a Union Table", "[qcpropLib]")
{
    UnionTable ut(3);

    REQUIRE(ut.size() == 3);
}