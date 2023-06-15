//
// Created by Jakob on 25/01/2023.
//

#ifndef QCPROP_CONSTANTPROPAGATION_HPP
#define QCPROP_CONSTANTPROPAGATION_HPP


#include "ConstantPropagation.hpp"
#include "QuantumComputation.hpp"
#include "UnionTable.hpp"
#include "ActivationState.hpp"

#define MAX_AMPLITUDES 1024

class ConstantPropagation {
public:
    static void optimize(qc::QuantumComputation &qc);

    static void optimize(qc::QuantumComputation &qc, size_t maxAmplitudes);

    static std::shared_ptr<UnionTable>
    propagate(qc::QuantumComputation &qc, size_t maxAmplitudes);

    static void
    propagate(qc::QuantumComputation &qc, size_t maxAmplitudes, const std::shared_ptr<UnionTable> &table);

private:
    static bool checkAmplitude(const std::shared_ptr<UnionTable> &table, size_t maxAmplitudes, size_t index);

    static void checkAmplitudes(const std::shared_ptr<UnionTable> &table, size_t maxAmplitudes);
};

#endif //QCPROP_CONSTANTPROPAGATION_HPP
