//
// Created by Jakob on 02/02/2023.
//

#include <catch2/catch_test_macros.hpp>

#include "UnionTable.hpp"
#include "Definitions.hpp"

SCENARIO("Make a Union Table")
{
    UnionTable ut(3);
    REQUIRE(ut.size() == 3);
}

SCENARIO("Test combine Method") {
    UnionTable table(3);

    //Build Register for testing
    /*
     * 0&2: |00> = 1/sqrt(2), |11> = 1/sqrt(2)
     * 1: |0> = 0.5, |1> = sqrt(3)/2
     */

    auto* testQReg = new QubitStateOrTop[3]{
            std::make_shared<QubitState>(2),
            std::make_shared<QubitState>(1),
            TOP::T
    };
    std::get<std::shared_ptr<QubitState>>(testQReg[0])->operator[](BitSet(2, 0)) = Complex(0.70710678118, 0);
    std::get<std::shared_ptr<QubitState>>(testQReg[0])->operator[](BitSet(2, 3)) = Complex(0.70710678118, 0);
    std::get<std::shared_ptr<QubitState>>(testQReg[1])->operator[](BitSet(1, 0)) = Complex(0.5, 0);
    std::get<std::shared_ptr<QubitState>>(testQReg[1])->operator[](BitSet(1, 1)) = Complex(0.86602540378, 0);
    testQReg[2] = std::get<std::shared_ptr<QubitState>>(testQReg[0]);
    table.setTable(testQReg);


    table.combine(0, 1);

    QubitStateOrTop* result = table.getTable();

    //All should be combined (point to same address)
    REQUIRE(result[0] == result[1]);
    REQUIRE(result[0] == result[2]);

    //Check if the result is correct
    REQUIRE(std::get<std::shared_ptr<QubitState>>(result[0])->size() == 4);

    REQUIRE(std::get<std::shared_ptr<QubitState>>(result[0])->operator[](BitSet(3, 0)) == Complex(qc::SQRT_2_2/2.0, 0));
    REQUIRE(std::get<std::shared_ptr<QubitState>>(result[0])->operator[](BitSet(3, 2)) == Complex(qc::SQRT_3*qc::SQRT_2_2/2.0, 0));
    REQUIRE(std::get<std::shared_ptr<QubitState>>(result[0])->operator[](BitSet(3, 5)) == Complex(qc::SQRT_2_2/2.0, 0));
    REQUIRE(std::get<std::shared_ptr<QubitState>>(result[0])->operator[](BitSet(3, 7)) == Complex(qc::SQRT_3*qc::SQRT_2_2/2.0, 0));
}