//
// Created by zuchna on 2/17/23.
//

#pragma once

#include "CircuitOptimization.hpp"

class CompoundGateInliner : public CircuitOptimization {
public:
    qc::QuantumComputation optimize(qc::QuantumComputation &qc) const override;

    static void addOperation(qc::QuantumComputation &qc, const std::unique_ptr<qc::Operation> &op,
                      const std::vector<qc::Control> &controls);
};
