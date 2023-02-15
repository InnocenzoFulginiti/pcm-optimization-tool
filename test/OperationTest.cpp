//
// Created by zuchna on 2/9/23.
//

#include "TestUtils.hpp"

TEST_CASE("Test rx Gate Matrix") {
    auto test = qc::StandardOperation();

    auto theta = GENERATE(take(10, random(0.0, 2 * qc::PI)));

    test.setGate(qc::RX);
    test.setParameter({theta});

    auto matrix = test.getMatrix();

    CHECK(matrix[0] == std::cos(theta / 2));
    CHECK(matrix[1] == Complex(0, -std::sin(theta / 2)));
    CHECK(matrix[2] == Complex(0, -std::sin(theta / 2)));
    CHECK(matrix[3] == std::cos(theta / 2));
}

TEST_CASE("Test ry Gate Matrix") {
    auto test = qc::StandardOperation();

    auto theta = GENERATE(take(10, random(0.0, 2 * qc::PI)));

    test.setGate(qc::RY);
    test.setParameter({theta});

    auto matrix = test.getMatrix();

    CHECK(matrix[0] == std::cos(theta/2));
    CHECK(matrix[1] == -std::sin(theta/2));
    CHECK(matrix[2] == std::sin(theta/2));
    CHECK(matrix[3] == std::cos(theta/2));
}