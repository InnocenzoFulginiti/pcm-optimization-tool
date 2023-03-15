//
// Created by Jakob on 25/01/2023.
//

#include "../include/ConstantPropagation.hpp"
#include "MatrixGenerator.hpp"
#include "CircuitOptimizer.hpp"


bool ConstantPropagation::checkAmplitude(const std::shared_ptr<UnionTable> &table, size_t maxAmplitudes, size_t index) {
    if ((*table)[index].isQubitState()) {
        if ((*table)[index].getQubitState()->size() > maxAmplitudes) {
            table->setTop(index);
            return true;
        }
    }

    return false;
}

[[maybe_unused]] bool
ConstantPropagation::checkAmplitudes(const std::shared_ptr<UnionTable> &table, size_t maxAmplitudes) {
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
    qc::QuantumComputation copyQc = qc.clone();

    //Use qfr to flatten compound gates
    qc::CircuitOptimizer::flattenOperations(copyQc);
    qc::QuantumComputation newQc = copyQc.clone();
    newQc.clear();

    for (auto const &op: copyQc) {
        //Gates that can be ignored
        if (op->getType() == qc::Barrier ||
            op->getType() == qc::Snapshot ||
            op->getType() == qc::ShowProbabilities ||
            op->getType() == qc::Teleportation ||
            op->getType() == qc::OpCount
                ) {
            newQc.emplace_back(op->clone());
            continue;
        }

        //Currently unsupported gates -> Targets set to top
        if (op->isClassicControlledOperation()
            || op->getType() == qc::Peres
            || op->getType() == qc::Peresdag
            || op->getType() == qc::ATrue
            || op->getType() == qc::AFalse
            || op->getType() == qc::MultiATrue
            || op->getType() == qc::MultiAFalse
                ) {
            for (auto t: op->getTargets()) {
                table->setTop(t);
            }
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
                        (*table)[i].getQubitState()->applyGate(i, {j}, {0, 1, 1, 0});
                        (*table)[i].getQubitState()->applyGate(j, min.second, {0, 1, 1, 0});
                        (*table)[i].getQubitState()->applyGate(i, {j}, {0, 1, 1, 0});
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
            auto nonUni = dynamic_cast<qc::NonUnitaryOperation *>(op.get());

            for (auto const t: nonUni->getTargets()) {
                table->setTop(t);
            }

            continue;
        }

        std::vector<size_t> controls{};
        for (auto c: op->getControls())
            controls.emplace_back(c.qubit);

        auto [act, min] = table->minimizeControls(controls);
        if (act == NEVER) {
            continue;
        }

        auto newOp = op->clone();
        newOp->setControls({});

        for (auto c: min)
            newOp->getControls().insert({static_cast<unsigned int>(c)});

        newQc.emplace_back(newOp);

        //Two qubit gates
        if (qc::isTwoQubitGate(op->getType())) {
            size_t t1 = op->getTargets()[0];
            size_t t2 = op->getTargets()[1];
            auto twoQubitMat = getTwoQubitMatrix(*op);
            table->combine(t1, t2);

            checkAmplitude(table, maxAmplitudes, t1);

            if (table->isTop(t1))
                continue;

            table->combine(t1, min);
            checkAmplitude(table, maxAmplitudes, t1);

            if (table->isTop(t1))
                continue;

            (*table)[t1].getQubitState()->applyTwoQubitGate(table->indexInState(t1),
                                                            table->indexInState(t2),
                                                            table->indexInState(min),
                                                            twoQubitMat);
        } else {
            //Single Qubit Gate
            size_t target = op->getTargets().begin()[0];
            auto singleQubitMat = getMatrix(*op);

            if (table->isTop(target)) {
                continue;
            }

            table->combine(target, min);

            checkAmplitude(table, maxAmplitudes, target);

            if (table->isTop(target)) {
                continue;
            } else {
                (*table)[target].getQubitState()->applyGate(table->indexInState(target),
                                                            table->indexInState(min),
                                                            singleQubitMat);
                checkAmplitude(table, maxAmplitudes, target);
            }
        }
    } //Loop over Gates

    return {newQc.clone(), table};
}

std::pair<qc::QuantumComputation, std::shared_ptr<UnionTable>> ConstantPropagation::propagate(
        const qc::QuantumComputation &qc, size_t maxAmplitudes) {
    auto copyQc = qc.clone();
    //Use qfr to try to replace classic controlled operations
    qc::CircuitOptimizer::eliminateResets(copyQc);

    try {
        qc::CircuitOptimizer::deferMeasurements(copyQc);
    } catch (qc::QFRException &e) {
        //If Classic controlled gates target multiple bits, deferMeasurements will fail
        //In this case, Classic controlled gates are skipped
        //And their targets set to TOP
    }

    auto table = std::make_shared<UnionTable>(copyQc.getNqubits());
    return propagate(copyQc, maxAmplitudes, table);
}

qc::QuantumComputation ConstantPropagation::optimize(qc::QuantumComputation &qc) {
    return optimize(qc, MAX_AMPLITUDES);
}

qc::QuantumComputation ConstantPropagation::optimize(qc::QuantumComputation &qc, size_t maxAmplitudes) {
    auto [optQc, table] = propagate(qc, maxAmplitudes);
    return optQc.clone();
}
