//
// Created by Jakob on 25/01/2023.
//

#include "../include/ConstantPropagation.hpp"
#include "MatrixGenerator.hpp"

#define SHOW_DEBUG_MSG false

bool ConstantPropagation::checkAmplitude(const std::shared_ptr<UnionTable> &table, size_t maxAmplitudes, size_t index) {
    if ((*table)[index].isQubitState()) {
        if ((*table)[index].getQubitState()->size() > maxAmplitudes) {
            table->setTop(index);
            return true;
        }
    }

    return false;
}

bool ConstantPropagation::checkAmplitudes(const std::shared_ptr<UnionTable> &table, size_t maxAmplitudes) {
    return std::any_of(table->begin(), table->end(), [&](auto const &u) {
        if (u.isQubitState()) {
            if (u.getQubitState()->size() > maxAmplitudes) {
                return true;
            }
        }
        return false;
    });
}

std::pair<qc::QuantumComputation, std::shared_ptr<UnionTable>> ConstantPropagation::propagate(
        const qc::QuantumComputation &qc, size_t maxAmplitudes, const std::shared_ptr<UnionTable> &table) {
    qc::QuantumComputation newQc = qc.clone();
    newQc.clear();

    for (auto const &op: qc) {
        if (op->getType() == qc::Barrier) {
            newQc.emplace_back(op->clone());
            continue;
        }

        if (op->isClassicControlledOperation()) {
            std::cout << "Classical Controlled Operations not supported, is skipped!" << std::endl;
            //TODO: Set Targets to T
            newQc.emplace_back(op->clone());
            continue;
        }

        if (op->getType() == qc::SWAP) {
            //Handle SWAP as compound Gate
            qc::Qubit i = op->getTargets()[0];
            qc::Qubit j = op->getTargets()[1];

            //Decompose cswap a, b
            std::vector<size_t> controls{};
            for (qc::Control c: op->getControls())
                controls.emplace_back(c.qubit);

            auto min = table->minimizeControls(controls);
            std::unique_ptr<qc::Operation> x = std::make_unique<qc::StandardOperation>();
            x->setGate(qc::X);

            auto newOp = op->clone();
            op->setControls({});
            for (size_t c: min.second)
                op->getControls().insert({static_cast<unsigned int>(c)});

            switch (min.first) {
                case ALWAYS:
                    table->swap(i, j);
                    newQc.emplace_back(newOp);
                    continue;
                case NEVER:
                    continue;
                case UNKNOWN:
                case SOMETIMES:
                    table->combine(j, i);
                    table->combine(i, min.second);
                    min.second.emplace_back(i);
                    if ((*table)[i].isTop()) {
                        continue;
                    } else {
                        (*table)[i].getQubitState()->applyGate(i, {j}, {0, 1, 1, 0}); //TODO: x->getMatrix());
                        (*table)[i].getQubitState()->applyGate(j, min.second, {0, 1, 1, 0}); //TODO: x->getMatrix());
                        (*table)[i].getQubitState()->applyGate(i, {j}, {0, 1, 1, 0}); //TODO: x->getMatrix());
                        checkAmplitude(table, maxAmplitudes, i);
                    }
                    continue;
            }
        }

        //Reset qubit to |0>
        if (op->getType() == qc::Reset) {
            for (const size_t t: op->getTargets()) {
                if ((*table)[t].isQubitState()) {
                    (*table)[t].getQubitState()->removeBit(table->indexInState(t));
                }

                (*table)[t] = std::make_shared<QubitState>(1);
            }

            newQc.emplace_back(op->clone());
            continue;
        }

        if (op->isCompoundOperation()) {
            std::cout << "Constant Propagation does not support Compound Gates, is skipped!" << std::endl;
            auto comp = dynamic_cast<qc::CompoundOperation *>(op.get());
            for (auto &o: *comp) {
                for (size_t t: o->getTargets()) {
                    table->setTop(t);
                }
            }

            newQc.emplace_back(op->clone());
            continue;
        }

        if (op->getType() == qc::Measure) {
            std::cout << "Constant Propagation does not support Measurement, is skipped! (qubits: ";

            auto nonUni = dynamic_cast<qc::NonUnitaryOperation *>(op.get());

            for (auto const t: nonUni->getTargets()) {
                std::cout << t << ", ";
            }

            std::cout << ")" << std::endl;

            continue;
        }

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

        //"Ordinary" Gate
        size_t target = op->getTargets().begin()[0];
        if (table->isTop(target)) {
            newQc.emplace_back(op->clone());
            continue;
        }

        std::array<Complex, 4> G = getMatrix(*op);

        //Get Target State
        std::vector<size_t> controls{};
        for (auto c: op->getControls())
            controls.emplace_back(c.qubit);

        auto [act, min] = table->minimizeControls(controls);
        auto newOp = op->clone();
        newOp->setControls({});

        switch (act) {
            case NEVER:
                continue;
            case ALWAYS:
                newQc.emplace_back(newOp);
                (*table)[target].getQubitState()->applyGate(table->indexInState(target), G);
                checkAmplitude(table, maxAmplitudes, target);
                continue;
            case UNKNOWN:
            case SOMETIMES:
                for (auto c: min)
                    newOp->getControls().insert({static_cast<unsigned int>(c)});

                table->combine(target, min);

                newQc.emplace_back(newOp);
                if (table->isTop(target)) {
                    continue;
                } else {
                    (*table)[target].getQubitState()->applyGate(table->indexInState(target),
                                                                table->indexInState(min),
                                                                G);
                    checkAmplitude(table, maxAmplitudes, target);
                }
        }
    } //Loop over Gates

    return {newQc.clone(), table};
}

std::pair<qc::QuantumComputation, std::shared_ptr<UnionTable>> ConstantPropagation::propagate(
        const qc::QuantumComputation &qc, size_t maxAmplitudes) {
    auto table = std::make_shared<UnionTable>(qc.getNqubits());
    return propagate(qc, maxAmplitudes, table);
}

qc::QuantumComputation ConstantPropagation::optimize(qc::QuantumComputation &qc) const {
    return optimize(qc, MAX_AMPLITUDES);
}

qc::QuantumComputation ConstantPropagation::optimize(qc::QuantumComputation &qc, int maxAmplitudes) {
    auto [optQc, table] = propagate(qc, static_cast<size_t>(maxAmplitudes));
    return optQc.clone();
}
