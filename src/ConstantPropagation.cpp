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

void ConstantPropagation::propagate(qc::QuantumComputation &qc, size_t maxAmplitudes,
                                    const std::shared_ptr<UnionTable> &table) {
    //Use qfr to flatten compound gates
    qc::CircuitOptimizer::flattenOperations(qc);

    qc::QuantumComputation newQc(qc.getNqubits());

    auto it = qc.begin();
    while (it != qc.end()) {
        auto op = (*it).get();
        it++;
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
            || op->getType() == qc::Reset
                ) {
            for (auto t: op->getTargets()) {
                table->setTop(t);
            }
            newQc.emplace_back(op->clone());
            continue;
        }

        if (op->isCompoundOperation()) {
            std::cout << "Constant Propagation does not support Compound Gates, is skipped!" << std::endl;
            auto comp = dynamic_cast<qc::CompoundOperation *>(op);
            for (auto &o: *comp) {
                for (size_t t: o->getTargets()) {
                    table->setTop(t);
                }
            }
            continue;
        }

        std::vector<size_t> controls{};
        for (qc::Control c: op->getControls())
            controls.emplace_back(c.qubit);

        auto [act, min] = table->minimizeControls(controls);

        op->getControls().clear();
        for (auto c: min) {
            op->getControls().insert({static_cast<unsigned int>(c)});
        }

        if (act == NEVER) {
            continue;
        } else {
            newQc.emplace_back(op->clone());
        }

        if (op->getType() == qc::SWAP) {
            //Handle SWAP as compound Gate
            qc::Qubit i = op->getTargets()[0];
            qc::Qubit j = op->getTargets()[1];

            //Decompose cswap a, b
            std::unique_ptr<qc::Operation> x = std::make_unique<qc::StandardOperation>();
            x->setGate(qc::X);

            op->setControls({});
            for (size_t c: min)
                op->getControls().insert({static_cast<unsigned int>(c)});

            if (act == ALWAYS) {
                table->swap(i, j);
            } else {
                table->combine(j, i);
                table->combine(i, min);
                min.emplace_back(i);
                if ((*table)[i].isTop()) {
                    continue;
                } else {
                    (*table)[i].getQubitState()->applyGate(i, {j}, {0, 1, 1, 0});
                    (*table)[i].getQubitState()->applyGate(j, min, {0, 1, 1, 0});
                    (*table)[i].getQubitState()->applyGate(i, {j}, {0, 1, 1, 0});
                    checkAmplitude(table, maxAmplitudes, i);
                }
            }
            continue;
        }

        if (op->getType() == qc::Measure) {
            auto nonUni = dynamic_cast<qc::NonUnitaryOperation *>(op);

            for (auto const t: nonUni->getTargets()) {
                table->setTop(t);
            }

            continue;
        }

        //Two qubit gates
        if (qc::isTwoQubitGate(op->getType())) {
            size_t t1 = op->getTargets()[0];
            size_t t2 = op->getTargets()[1];
            auto twoQubitMat = getTwoQubitMatrix(*op);
            table->combine(t1, t2);

            checkAmplitude(table, maxAmplitudes, t1);

            if (table->isTop(t1)) {
                continue;
            }

            table->combine(t1, min);
            checkAmplitude(table, maxAmplitudes, t1);

            if (table->isTop(t1)) {
                continue;
            }


            (*table)[t1].getQubitState()->applyTwoQubitGate(table->indexInState(t1),
                                                            table->indexInState(t2),
                                                            table->indexInState(min),
                                                            twoQubitMat);
            continue;
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
                continue;
            }
        }
    } //Loop over Gates
}

std::shared_ptr<UnionTable> ConstantPropagation::propagate(qc::QuantumComputation &qc, size_t maxAmplitudes) {
    auto table = std::make_shared<UnionTable>(qc.getNqubits());
    propagate(qc, maxAmplitudes, table);
    return table;
}

void ConstantPropagation::optimize(qc::QuantumComputation &qc) {
    optimize(qc, MAX_AMPLITUDES);
}

void ConstantPropagation::optimize(qc::QuantumComputation &qc, size_t maxAmplitudes) {
    propagate(qc, maxAmplitudes);
}
