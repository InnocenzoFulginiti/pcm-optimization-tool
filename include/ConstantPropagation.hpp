//
// Created by Jakob on 25/01/2023.
//

#ifndef QCPROP_CONSTANTPROPAGATION_HPP
#define QCPROP_CONSTANTPROPAGATION_HPP


#include "CircuitOptimization.hpp"
#include "UnionTable.hpp"

#define MAX_AMPLITUDES 1024

enum ActivationState {
    ALWAYS, NEVER, SOMETIMES, UNKNOWN
};

class ConstantPropagation : public CircuitOptimization {
public:
    [[nodiscard]] qc::QuantumComputation optimize(qc::QuantumComputation& qc) const override;

    [[nodiscard]] static std::pair<std::vector<ActivationState>, std::shared_ptr<UnionTable>>
    propagate(const qc::QuantumComputation &qc, size_t maxAmplitudes) ;
private:

};

#endif //QCPROP_CONSTANTPROPAGATION_HPP
