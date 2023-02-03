//
// Created by Jakob on 25/01/2023.
//

#include "../include/ConstantPropagation.hpp"
#include "../include/UnionTable.hpp"

qc::QuantumComputation ConstantPropagation::optimize(qc::QuantumComputation &qc) const {
    UnionTable table(3);
    //Define array of complex numbers

    qc.print(std::cout);



    table.print(std::cout);

    table.combine(0, 1);

    table.print(std::cout);

    return {};
}
