//
// Created by Jakob on 25/01/2023.
//

#include "../include/ConstantPropagation.hpp"

#define SHOW_DEBUG_MSG false

ActivationState
ConstantPropagation::applyGate(const std::shared_ptr<UnionTable> &table,
                               const std::unique_ptr<qc::Operation> &op,
                               const size_t maxAmplitudes,
                               std::map<size_t, double> &measurementResults) {

    /**
     * Barrier: https://qiskit.org/documentation/stubs/qiskit.circuit.library.Barrier.html
     *          Just indicator, not relevant for qcprop
     */
    if (op->getType() == qc::Barrier) {
        return UNKNOWN;
    }

    if (op->isClassicControlledOperation()) {
        auto cco = dynamic_cast<qc::ClassicControlledOperation *>(op.get());

        //Don't do anything if target is TOP anyways
        if ((*table)[cco->getOperation()->getTargets()[0]].isTop())
            return UNKNOWN;

        auto classics = cco->getControlRegister();
        size_t classicsStart = classics.first;
        size_t classicsLength = classics.second;
        size_t expected = cco->getExpectedValue();

        size_t target = cco->getOperation()->getTargets()[0];

        double prob = 1.0;
        for (size_t i = 0; i < classicsLength; i++) {
            if (measurementResults[classicsStart + i] < 0) {
                table->setTop(target);
                return UNKNOWN;
            }

            if (expected & (1 << i)) {
                prob *= measurementResults[classicsStart + i];
            } else {
                prob *= 1 - measurementResults[classicsStart + i];
            }
        }

        //Use complex so same "Approximate" zero method is used
        if (Complex(prob).isZero()) {
            return NEVER;
        }

        if (Complex(prob) == 1) {
            applyGate(table, cco->getOperation()->clone(), maxAmplitudes, measurementResults);
            return ALWAYS;
        }

        //Weighed average of activated/not activated
        std::shared_ptr<QubitState> targetState = (*table)[target].getQubitState();
        QubitState stateAfterGate = targetState->clone();
        stateAfterGate.applyGate(table->indexInState(target), cco->getOperation()->getMatrix());

        stateAfterGate *= prob;
        (*targetState) *= (1 - prob);

        (*targetState) += stateAfterGate;

        return SOMETIMES;
    }

    if (op->getType() == qc::SWAP) {
        //Handle SWAP as compound Gate
        qc::Qubit i = op->getTargets()[0];
        qc::Qubit j = op->getTargets()[1];

        if (!op->getControls().empty()) {
            //Decompose cswap a, b, c as
            //  cx c,b;
            //  ccx a,b,c;
            //  cx c,b;
            //TODO: cswap
            std::cout << "Constant Propagation does not support cswap, is skipped!" << std::endl;
            return UNKNOWN;
        } else {
            table->swap(i, j);
        }
    }

    //Reset qubit to |0>
    if (op->getType() == qc::Reset) {
        for (const size_t t: op->getTargets()) {
            (*table)[t] = std::make_shared<QubitState>(1);
        }
        //TODO: Optimize this? What about entangled qubits?
        return ALWAYS;
    }

    if (op->isCompoundOperation()) {
        std::cout << "Constant Propagation does not support Compound Gates, is skipped!" << std::endl;
        table->setTop(op->getTargets()[0]);
        return UNKNOWN;
    }

    if (op->getType() == qc::Measure) {
        auto nonUni = dynamic_cast<qc::NonUnitaryOperation *>(op.get());

        measure(table,
                nonUni->getTargets(),
                nonUni->getClassics(),
                measurementResults);

        return ALWAYS;
    }

    //Not a "special" Gate

    if (SHOW_DEBUG_MSG) {
        std::cout << table->to_string() << std::endl;
        std::cout << "Applying Gate: " << op->getName();
        std::cout << " with Target: " << op->getTargets().begin()[0];
        std::cout << " and Controls: ";

        for (auto c: op->getControls()) {
            std::cout << c.qubit << " ";
        }
        std::cout << std::endl;
    }

    size_t target = op->getTargets().begin()[0];
    if (table->isTop(target)) {
        return UNKNOWN;
    }

    auto G = op->getMatrix();

    //Get Target State
    auto targetState = ((*table)[target]).getQubitState();
    std::vector<size_t> controls{};
    auto ctr = op->getControls();
    bool controlsAreTop = false;
    for (auto c: ctr) {
        controls.emplace_back(c.qubit);
        if (table->isTop(c.qubit)) {
            controlsAreTop = true;
        }
    }
    if (controlsAreTop) {
        return UNKNOWN;
    }


    std::pair counts = table->countActivations(controls);
    size_t notActivated = counts.first;
    size_t activated = counts.second;

    if (activated == 0 && !controls.empty()) {
        //Gate can never be activated --> Do nothing
        if (SHOW_DEBUG_MSG) {
            std::cout << "Found Gate that can never be activated: " << qc::toString(op->getType()) << ", T: "
                      << target << ", Ctrl: ";
            for (auto c: controls) {
                std::cout << c << " ";
            }
            std::cout << std::endl;
        }
        return NEVER;
    } else if (notActivated == 0 || controls.empty()) {
        if (SHOW_DEBUG_MSG) {
            std::cout << "Found Gate that is always activated: " << qc::toString(op->getType()) << ", T: "
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
        return ALWAYS;
    } else {
        //Gate is sometimes applied --> Apply
        table->combine(target, controls);
        //State may have changed
        targetState = ((*table)[target]).getQubitState();

        if (targetState->size() > maxAmplitudes) {
            table->setTop(target);
            return UNKNOWN;
        }

        //Find indices in state
        size_t targetIndex = table->indexInState(target);
        std::vector<size_t> controlIndices{};
        for (auto c: controls) {
            controlIndices.emplace_back(table->indexInState(c));
        }

        targetState->applyGate(targetIndex, controlIndices, G);

        if (targetState->size() > maxAmplitudes) {
            table->setTop(target);
        }

        return SOMETIMES;
    }
}


std::pair<std::vector<ActivationState>, std::shared_ptr<UnionTable>> ConstantPropagation::propagate(
        const qc::QuantumComputation &qc, size_t maxAmplitudes) {


    std::shared_ptr<UnionTable> table = std::make_shared<UnionTable>(qc.getNqubits());
    std::vector<ActivationState> activationStates{};
    std::map<size_t, double> measurementResults{};

    for (auto &gate: qc) {
        activationStates.emplace_back(applyGate(table, gate, maxAmplitudes, measurementResults));
    }

    return {activationStates, table};
}

qc::QuantumComputation ConstantPropagation::optimize(qc::QuantumComputation &qc) const {
    return optimize(qc, MAX_AMPLITUDES);
}

qc::QuantumComputation ConstantPropagation::optimize(qc::QuantumComputation &qc, int maxAmplitudes) const {
    auto ret = propagate(qc, maxAmplitudes);

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

void ConstantPropagation::measure(const std::shared_ptr<UnionTable> &table, std::vector<qc::Qubit> qubits,
                                  std::vector<size_t> classics, std::map<size_t, double> &results) {
    for (size_t i = 0; i < qubits.size(); ++i) {
        size_t target = qubits[i];

        if (table->isTop(target)) {
            results[classics[i]] = -1;
            continue;
        }

        (*table)[target].getQubitState()->setMeasured();

        auto targetState = ((*table)[target]).getQubitState();
        size_t indexInState = table->indexInState(target);
        double prob = 0.0;
        for (auto &[key, val]: *targetState) {
            if ((key & (1 << indexInState)) > 0) {
                prob += val.norm();
            }
        }

        results[classics[i]] = prob;
    }
}
