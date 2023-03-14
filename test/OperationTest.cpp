#include "TestUtils.hpp"

TEST_CASE("Test rx Gate Matrix") {
    auto test = qc::StandardOperation();

    auto theta = GENERATE(take(10, random(0.0, 2 * PI)));

    CAPTURE(theta);
    CAPTURE(std::cos(theta / 2));
    CAPTURE(Complex(0, -std::sin(theta / 2)));

    test.setGate(qc::RX);
    test.setParameter({theta, 0, 0});

    std::array<Complex, 4> matrix = getMatrix(test);

    CAPTURE(matrix[0]);
    CAPTURE(matrix[1]);
    CAPTURE(matrix[2]);
    CAPTURE(matrix[3]);

    approx(std::cos(theta / 2), matrix[0]);
    approx(Complex(0, -std::sin(theta / 2)), matrix[1]);
    approx(Complex(0, -std::sin(theta / 2)), matrix[2]);
    approx(std::cos(theta / 2), matrix[3]);
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