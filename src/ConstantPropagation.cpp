//
// Created by Jakob on 25/01/2023.
//

#include "../include/ConstantPropagation.hpp"
#include "../include/UnionTable.hpp"

qc::QuantumComputation ConstantPropagation::optimize(qc::QuantumComputation &qc) const {
    UnionTable table(3);
    //Define array of complex numbers

    qc.print(std::cout);

    auto* testTable = new QubitStateOrTop[3]{
            std::make_shared<QubitState>(2),
            std::make_shared<QubitState>(1),
            TOP::T
    };

    std::get<std::shared_ptr<QubitState>>(testTable[0])->operator[](BitSet(2, 0)) = Complex(0.70710678118, 0);
    std::get<std::shared_ptr<QubitState>>(testTable[0])->operator[](BitSet(2, 3)) = Complex(0.70710678118, 0);

    std::get<std::shared_ptr<QubitState>>(testTable[1])->operator[](BitSet(1, 0)) = Complex(0.5, 0);
    std::get<std::shared_ptr<QubitState>>(testTable[1])->operator[](BitSet(1, 1)) = Complex(0.86602540378, 0);

    testTable[2] = std::get<std::shared_ptr<QubitState>>(testTable[0]);


    table.setTable(testTable);

    table.print(std::cout);

    table.combine(0, 1);

    table.print(std::cout);

    return {};
}
