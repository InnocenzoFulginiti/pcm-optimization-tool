//
// Created by Jakob on 24/01/2023.
//

#ifndef QCPROP_CIRCUITOPTIMIZATION_HPP
#define QCPROP_CIRCUITOPTIMIZATION_HPP

#include <vector>
#include "QuantumComputation.hpp"
#include "operations/Operation.hpp"

class CircuitOptimization {
public:
    virtual qc::QuantumComputation optimize(qc::QuantumComputation &qc) const = 0;
};

#endif //QCPROP_CIRCUITOPTIMIZATION_HPP
