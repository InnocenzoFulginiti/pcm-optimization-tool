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
        //DEBUG: Print Table
        std::cout << table.to_string() << std::endl;
        std::cout << "Applying Gate: " << gate->getName();
        std::cout << " with Target: " << gate->getTargets().begin()[0];
        std::cout << " and Controls: ";
        for (auto c: gate->getControls()) {
            std::cout << c.qubit << " ";
        }
        std::cout << std::endl;

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


        std::pair counts = table.countActivations(controls);
        size_t notActivated = counts.first;
        size_t activated = counts.second;

        if (activated == 0 && !controls.empty()) {
            //Gate can never be activated --> Do nothing
            continue;
        } else if (notActivated == 0 || controls.empty()) {
            //Gate is always applied --> Apply without controls

            targetState->applyGate(table.indexInState(target), G);
            //TODO: Add to new qc
            //TODO: Check size of amplitudes
        } else {
            //Gate is sometimes applied --> Apply
            table.combine(target, controls);
            //State may have changed
            targetState = std::get<std::shared_ptr<QubitState>>(table[target]);
            targetState->applyGate(table.indexInState(target), controls, G);
            //Also add to qc
            //Also check amplitudes and set Top if necessary
        }

    }

    return {};
}
