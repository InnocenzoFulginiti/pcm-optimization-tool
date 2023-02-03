//
// Created by Jakob on 25/01/2023.
//

#ifndef QCPROP_CONSTANTPROPAGATION_HPP
#define QCPROP_CONSTANTPROPAGATION_HPP


#include "CircuitOptimization.hpp"

class ConstantPropagation : public CircuitOptimization {
public:
    [[nodiscard]] qc::QuantumComputation optimize(qc::QuantumComputation& qc) const override;
};


#endif //QCPROP_CONSTANTPROPAGATION_HPP
