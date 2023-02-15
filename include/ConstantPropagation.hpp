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
    qc::QuantumComputation optimize(qc::QuantumComputation &qc, int maxAmplitudes) const;

    [[nodiscard]] static std::pair<std::vector<ActivationState>, std::shared_ptr<UnionTable>>
    propagate(const qc::QuantumComputation &qc, size_t maxAmplitudes) ;
private:
    static ActivationState
    applyGate(const std::shared_ptr<UnionTable> &table, const std::unique_ptr<qc::Operation> &op, size_t maxAmplitudes,
              std::map<size_t, double> &measurementResults);

    static ActivationState
    applyCompoundGate(const std::shared_ptr<UnionTable> &table, const std::vector<std::unique_ptr<qc::Operation>>& ops,
                      size_t maxAmplitudes, std::map<size_t, double> &measurementResults);

    static void
    measure(const std::shared_ptr<UnionTable> &sharedPtr, std::vector<qc::Qubit> qubits, std::vector<size_t> classics,
            std::map<size_t, double> &map);

    static ActivationState
    applyCompoundGate(const std::shared_ptr<UnionTable> &table, const std::vector<std::unique_ptr<qc::Operation>> &ops,
                      const std::vector<size_t> &controls, const size_t maxAmplitudes,
                      std::map<size_t, double> &measurementResults);
};

#endif //QCPROP_CONSTANTPROPAGATION_HPP
