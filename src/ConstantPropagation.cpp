//
// Created by Jakob on 25/01/2023.
//

#include "../include/ConstantPropagation.hpp"
#include "../include/UnionTable.hpp"

qc::QuantumComputation ConstantPropagation::optimize(qc::QuantumComputation &qc) const {
    UnionTable table(qc.getNqubits());

    //Iterate over

    table.combine(0, 1);

    table.print(std::cout);

    return {};
}
