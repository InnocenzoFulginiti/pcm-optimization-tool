//
// Created by zuchna on 2/4/23.
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/generators/catch_generators_random.hpp>
#include <catch2/catch_approx.hpp>

#include "UnionTable.hpp"

TEST_CASE("QubitState normalization example") {
    Complex c1(1, 0);
    Complex c2(1, 0);

    QubitState qs(1);
    qs[BitSet(1, 0)] = c1;
    qs[BitSet(1, 1)] = c2;

    qs.normalize();
    REQUIRE(qs.norm() == Catch::Approx(1.0));
}

TEST_CASE("QubitState normalization random test") {
    //Generate 4*4*4*4 = 256 doubles
    auto p1 = GENERATE(take(4, random(-10.0, 10.0)));
    auto p2 = GENERATE(take(4, random(-10.0, 10.0)));
    auto p3 = GENERATE(take(4, random(-10.0, 10.0)));
    auto p4 = GENERATE(take(4, random(-10.0, 10.0)));

    auto c1 = Complex(p1, p2);
    auto c2 = Complex(p3, p4);


    QubitState qs(1);
    qs[BitSet(1, 0)] = c1;
    qs[BitSet(1, 1)] = c2;

    std::cout << qs << std::endl;

    qs.normalize();

    REQUIRE(qs.norm() == Catch::Approx(1.0));
}

SCENARIO("QubitState Merge Example") {
    //Build Register for testing
    /*
     * 0&2: |00> = 1/sqrt(2), |11> = 1/sqrt(2)
     * 1: |0> = 0.5, |1> = sqrt(3)/2
     */

    auto *testQReg = new QubitStateOrTop[3]{
            std::make_shared<QubitState>(2),
            std::make_shared<QubitState>(1),
            TOP::T
    };

    std::get<std::shared_ptr<QubitState>>(testQReg[0])->operator[](BitSet(2, 0)) = Complex(0.70710678118, 0);
    std::get<std::shared_ptr<QubitState>>(testQReg[0])->operator[](BitSet(2, 3)) = Complex(0.70710678118, 0);
    std::get<std::shared_ptr<QubitState>>(testQReg[1])->operator[](BitSet(1, 0)) = Complex(0.5, 0);
    std::get<std::shared_ptr<QubitState>>(testQReg[1])->operator[](BitSet(1, 1)) = Complex(0.86602540378, 0);

    testQReg[2] = std::get<std::shared_ptr<QubitState>>(testQReg[0]);

    std::shared_ptr<QubitState> q0_2 = std::get<std::shared_ptr<QubitState>>(testQReg[0]);
    std::shared_ptr<QubitState> q1 = std::get<std::shared_ptr<QubitState>>(testQReg[1]);


    std::shared_ptr<QubitState> result = QubitState::combine(q0_2, std::vector{0, 2}, q1, std::vector{1});

    Complex expected0 = Complex(qc::SQRT_2_2 / 2.0, 0);
    Complex expected2 = Complex(qc::SQRT_3 * qc::SQRT_2_2 / 2.0, 0);
    Complex expected5 = expected0;
    Complex expected7 = expected2;

    //Make sure my solution is at least normed
    double sum = expected0.norm() + expected2.norm() + expected5.norm() + expected7.norm();
    REQUIRE(Complex(sum) == Complex(1, 0));

    //Check if the result is correct
    REQUIRE(result->size() == 4);

    REQUIRE((*result)[BitSet(3, 0)] == expected0);

    REQUIRE((*result)[BitSet(3, 2)] == expected2);

    REQUIRE((*result)[BitSet(3, 5)] == expected5);

    REQUIRE((*result)[BitSet(3, 7)] == expected7);
}