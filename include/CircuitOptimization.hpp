//
// Created by Jakob on 24/01/2023.
//

#ifndef QCPROP_CIRCUITOPTIMIZATION_HPP
#define QCPROP_CIRCUITOPTIMIZATION_HPP

#include <vector>
#include "../extern/qfr/include/QuantumComputation.hpp"

class CircuitOptimization {
public:
    virtual ~CircuitOptimization() = default;
    virtual qc::QuantumComputation optimize(qc::QuantumComputation &qc) const = 0;
};

#endif //QCPROP_CIRCUITOPTIMIZATION_HPP
