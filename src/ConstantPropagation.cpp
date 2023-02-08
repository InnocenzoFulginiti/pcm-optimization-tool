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

    qc::QuantumComputation ret(qc.getNqubits());

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

        auto G = gate->getMatrix();

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
            std::cout << "Found Gate that can never be activated: " << qc::toString(gate->getType()) << ", T: "
                      << target << ", Ctrl: ";
            for (auto c: controls) {
                std::cout << c << " ";
            }
            std::cout << std::endl;
            continue;
        } else if (notActivated == 0 || controls.empty()) {
            std::cout << "Found Gate that is always activated: " << qc::toString(gate->getType()) << ", T: " << target
                      << ", Ctrl: ";
            for (auto c: controls) {
                std::cout << c << " ";
            }
            std::cout << std::endl;

            targetState->applyGate(table.indexInState(target), G);
            gate->setControls(qc::Controls{});
            ret.emplace_back(gate);
            //TODO: Check size of amplitudes
        } else {
            //Gate is sometimes applied --> Apply
            table.combine(target, controls);
            //State may have changed
            targetState = std::get<std::shared_ptr<QubitState>>(table[target]);

            //Find indices in state
            size_t targetIndex = table.indexInState(target);
            std::vector<size_t> controlIndices{};
            for (auto c: controls) {
                controlIndices.emplace_back(table.indexInState(c));
            }

            targetState->applyGate(targetIndex, controlIndices, G);
            ret.emplace_back(gate);
        }

    }

    return ret;
}
