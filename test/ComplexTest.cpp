//
// Created by zuchna on 2/3/23.
//

#include <catch2/catch_test_macros.hpp>

#include "Complex.hpp"

SCENARIO("Create a Complex Number")
{
    Complex a = 1;
    REQUIRE(a.real() == 1.0);
    REQUIRE(a.imag() == 0.0);
}