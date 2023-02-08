//
// Created by Jakob on 25/01/2023.
//

#include "../include/ConstantPropagation.hpp"
#include "../include/UnionTable.hpp"
#include "../include/Definitions.hpp"

qc::QuantumComputation ConstantPropagation::optimize(qc::QuantumComputation &qc) const {
    Complex X[4] = {0, 1, 1, 0};
    Complex H[4] = {qc::SQRT_2_2, qc::SQRT_2_2, qc::SQRT_2_2, -qc::SQRT_2_2};


    UnionTable table(qc.getNqubits());

    for (auto &gate: qc) {
        size_t target = gate->getTargets().begin()[0];
        if (table.isTop(target))
            continue;

        //TODO: Actual Solution for any Gate Type
        Complex *G;
        switch (gate->getType()) {
            case qc::X:
                G = X;break;
            case qc::H:
                G = H;break;
            default:
                std::cerr << "Not Implemented!";

        }

        //Get Target State
        auto targetState = std::get<std::shared_ptr<QubitState>>(table[target]);
        std::vector<size_t> controls{};
        auto ctr = gate->getControls();
        for (auto c: ctr) {
            controls.emplace_back(c.qubit);
        }

        if (table.canActivate(controls)) {
            table.combine(target, controls);
            targetState = std::get<std::shared_ptr<QubitState>>(table[target]);
            targetState->applyGate(table.indexInState(target), controls, G);
        } else {
            std::cout << "Found Gate That can not be activated: " << gate->getName() << "Controls: ";
            //Print Controls
            for (auto c: controls) {
                std::cout << c << ", ";
            }
            std::cout << "Target: " << target << std::endl;
        }

    }

    return {};
}
