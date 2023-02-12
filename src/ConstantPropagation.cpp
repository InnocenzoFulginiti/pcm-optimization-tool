//
// Created by Jakob on 25/01/2023.
//

#include "../include/ConstantPropagation.hpp"

#define SHOW_DEBUG_MSG false

std::pair<std::vector<ActivationState>, std::shared_ptr<UnionTable>> ConstantPropagation::propagate(
        const qc::QuantumComputation &qc, size_t maxAmplitudes) {

    std::shared_ptr<UnionTable> table = std::make_shared<UnionTable>(qc.getNqubits());
    std::vector<ActivationState> activationStates{};

    for (auto &gate: qc) {
        if (gate->getType() == qc::Compound) {
            activationStates.emplace_back(UNKNOWN);
            continue;
        }

        if (SHOW_DEBUG_MSG) {
            std::cout << table->to_string() << std::endl;
            std::cout << "Applying Gate: " << gate->getName();
            std::cout << " with Target: " << gate->getTargets().begin()[0];
            std::cout << " and Controls: ";

            for (auto c: gate->getControls()) {
                std::cout << c.qubit << " ";
            }
            std::cout << std::endl;
        }
        size_t target = gate->getTargets().begin()[0];
        if (table->isTop(target)) {
            activationStates.emplace_back(UNKNOWN);
            continue;
        }

        auto G = gate->getMatrix();

        //Get Target State
        auto targetState = ((*table)[target]).getQubitState();
        std::vector<size_t> controls{};
        auto ctr = gate->getControls();
        bool controlsAreTop = false;
        for (auto c: ctr) {
            controls.emplace_back(c.qubit);
            if (table->isTop(c.qubit)) {
                controlsAreTop = true;
            }
        }
        if(controlsAreTop) {
            activationStates.emplace_back(UNKNOWN);
            continue;
        }


        std::pair counts = table->countActivations(controls);
        size_t notActivated = counts.first;
        size_t activated = counts.second;

        if (activated == 0 && !controls.empty()) {
            //Gate can never be activated --> Do nothing
            if (SHOW_DEBUG_MSG) {
                std::cout << "Found Gate that can never be activated: " << qc::toString(gate->getType()) << ", T: "
                          << target << ", Ctrl: ";
                for (auto c: controls) {
                    std::cout << c << " ";
                }
                std::cout << std::endl;
            }
            activationStates.emplace_back(NEVER);
            continue;
        } else if (notActivated == 0 || controls.empty()) {
            if (SHOW_DEBUG_MSG) {
                std::cout << "Found Gate that is always activated: " << qc::toString(gate->getType()) << ", T: "
                          << target
                          << ", Ctrl: ";
                for (auto c: controls) {
                    std::cout << c << " ";
                }
                std::cout << std::endl;
            }
            targetState->applyGate(table->indexInState(target), G);
            if (targetState->size() > maxAmplitudes) {
                table->setTop(target);
            }
            activationStates.emplace_back(ALWAYS);
        } else {
            //Gate is sometimes applied --> Apply
            table->combine(target, controls);
            //State may have changed
            targetState = ((*table)[target]).getQubitState();

            if (targetState->size() > maxAmplitudes) {
                table->setTop(target);
                continue;
            }

            //Find indices in state
            size_t targetIndex = table->indexInState(target);
            std::vector<size_t> controlIndices{};
            for (auto c: controls) {
                controlIndices.emplace_back(table->indexInState(c));
            }

            targetState->applyGate(targetIndex, controlIndices, G);
            activationStates.emplace_back(SOMETIMES);
            if (targetState->size() > maxAmplitudes) {
                table->setTop(target);
                continue;
            }
        }

    }

    return {activationStates, table};
}

qc::QuantumComputation ConstantPropagation::optimize(qc::QuantumComputation &qc) const {
    auto ret = propagate(qc, 3);

    std::vector<ActivationState> ops = ret.first;

    qc::QuantumComputation optimized(qc.getNqubits());
    for (auto &gate: qc) {
        switch (ops[0]) {
            case ALWAYS:
                gate->setControls({});
                optimized.emplace_back(gate);
                break;
            case NEVER:
                break;
            case SOMETIMES:
            case UNKNOWN:
                optimized.emplace_back(gate);
                break;

        }
        ops.erase(ops.begin());
    }


    return optimized;
}
