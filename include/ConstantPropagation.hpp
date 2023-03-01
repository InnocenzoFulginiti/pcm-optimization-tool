//
// Created by Jakob on 25/01/2023.
//

#ifndef QCPROP_CONSTANTPROPAGATION_HPP
#define QCPROP_CONSTANTPROPAGATION_HPP


#include "CircuitOptimization.hpp"
#include "UnionTable.hpp"
#include "ActivationState.hpp"

#define MAX_AMPLITUDES 1024

class ConstantPropagation : public CircuitOptimization {
public:
    [[nodiscard]] qc::QuantumComputation optimize(qc::QuantumComputation& qc) const override;
    qc::QuantumComputation optimize(qc::QuantumComputation &qc, int maxAmplitudes) const;

    [[nodiscard]] static std::pair<qc::QuantumComputation, std::shared_ptr<UnionTable>>
    propagate(const qc::QuantumComputation &qc, size_t maxAmplitudes) ;

    static std::pair<qc::QuantumComputation, std::shared_ptr<UnionTable>>
    propagate(const qc::QuantumComputation &qc, size_t maxAmplitudes, const std::shared_ptr<UnionTable> &table);
private:
    static void
    measure(const std::shared_ptr<UnionTable> &sharedPtr, std::vector<qc::Qubit> qubits, std::vector<size_t> classics,
            std::map<size_t, double> &map);

    static bool checkAmplitude(const std::shared_ptr<UnionTable> &table, size_t maxAmplitudes, size_t index);

    static bool checkAmplitudes(const std::shared_ptr<UnionTable> &table, size_t maxAmplitudes);
};

#endif //QCPROP_CONSTANTPROPAGATION_HPP
