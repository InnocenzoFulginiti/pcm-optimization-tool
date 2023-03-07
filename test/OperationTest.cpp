#include "TestUtils.hpp"

TEST_CASE("Test rx Gate Matrix") {
    auto test = qc::StandardOperation();

    auto theta = GENERATE(take(10, random(0.0, 2 * PI)));

    test.setGate(qc::RX);
    test.setParameter({theta, 0, 0});

    std::array<Complex, 4> matrix = getMatrix(test);

    CHECK(matrix[0] == std::cos(theta / 2));
    CHECK(matrix[1] == Complex(0, -std::sin(theta / 2)));
    CHECK(matrix[2] == Complex(0, -std::sin(theta / 2)));
    CHECK(matrix[3] == std::cos(theta / 2));
}

TEST_CASE("Test ry Gate Matrix") {
    auto test = qc::StandardOperation();

    auto theta = GENERATE(take(10, random(0.0, 2 * PI)));

    test.setGate(qc::RY);
    test.setParameter({theta, 0, 0});

    std::array<Complex, 4> matrix = getMatrix(test);

    CHECK(matrix[0] == std::cos(theta/2));
    CHECK(matrix[1] == -std::sin(theta/2));
    CHECK(matrix[2] == std::sin(theta/2));
    CHECK(matrix[3] == std::cos(theta/2));
}