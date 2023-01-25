//
// Created by Jakob on 24/01/2023.
//

#ifndef QCPROP_CIRCUITOPTIMIZATION_HPP
#define QCPROP_CIRCUITOPTIMIZATION_HPP

#include <vector>
#include "operations/Operation.hpp"

class CircuitOptimization {
public:
    [[nodiscard]] virtual std::vector<std::unique_ptr<qc::Operation>> optimize(std::vector<std::unique_ptr<qc::Operation>> ops) const = 0;
};


#endif //QCPROP_CIRCUITOPTIMIZATION_HPP
