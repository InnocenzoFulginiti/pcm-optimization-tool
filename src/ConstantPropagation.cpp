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

void
ConstantPropagation::checkAmplitudes(const std::shared_ptr<UnionTable> &table, size_t maxAmplitudes) {
    for (size_t i = 0; i < table->size(); i++) {
        checkAmplitude(table, maxAmplitudes, i);
    }
}

void ConstantPropagation::propagate(qc::QuantumComputation &qc, size_t maxAmplitudes,
                                    const std::shared_ptr<UnionTable> &table) {
    std::vector<ClassicalRegisterValue> classicControlBits(qc.getNqubits(), NOT_KNOWN);
    //Use qfr to flatten compound gates
    qc::CircuitOptimizer::flattenOperations(qc);

    qc::QuantumComputation newQc(qc.getNqubits());

    auto it = qc.begin();
    while (it != qc.end()) {
        auto op = (*it).get();
        it++;

        checkAmplitudes(table, maxAmplitudes);

        if (table->allTop()) {
            newQc.emplace_back(op->clone());
            continue;
        }

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
        if (op->getType() == qc::Peres
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

            newQc.emplace_back(op->clone());
            continue;
        }

        if (op->getType() == qc::Measure) {
            auto nonUni = dynamic_cast<qc::NonUnitaryOperation *>(op);

            for (auto const t: nonUni->getTargets()) {
                if (table->purityTest(t)) {

                    double _probabilityMeasureZero = (*table)[t].getQubitState()->probabilityMeasureZero(table->indexInState(t));
                    double _probabilityMeasureOne = (*table)[t].getQubitState()->probabilityMeasureOne(table->indexInState(t));

                    if (_probabilityMeasureZero != 1.0 && 
                        _probabilityMeasureOne != 1.0) {
                        Complex alpha_tmp((*table)[t].getQubitState()->amplitudeStateZero(table->indexInState(t)));
                        double alpha = alpha_tmp.real();

                        newQc.emplace_back(std::make_unique<qc::StandardOperation>(op->getNqubits(), op->getControls(), op->getTargets(), qc::OpType::RY, std::vector<qc::fp>(1, 2.0 * acos(alpha)), op->getStartingQubit()));
                        
                        auto tmp = std::make_unique<qc::StandardOperation>(op->getNqubits(), op->getControls(), op->getTargets(), qc::OpType::X, std::vector<qc::fp>(), op->getStartingQubit())->clone();
                        auto probabilisticOp = std::make_unique<qc::ProbabilisticOperation>(tmp, _probabilityMeasureZero);
                        newQc.emplace_back(probabilisticOp);
                        classicControlBits[t] = NOT_KNOWN;
                        table->setTop(t);
                    }
                    else if (_probabilityMeasureZero == 1.0) {
                        classicControlBits[t] = ZERO;
                    }
                    else {
                        classicControlBits[t] = ONE;
                    }
                    // else if probability is 0 or 1, apply nothing and and leave the qubit state unchanged
                }
                else {
                    table->setTop(t);
                    newQc.emplace_back(op->clone());
                }
            }
            continue;
        }

        if (op->isClassicControlledOperation()) {
            std::unique_ptr<qc::Operation> clonedOp(op->clone());

            // Performing the dynamic cast
            try {
                qc::ClassicControlledOperation& ccop = dynamic_cast<qc::ClassicControlledOperation&>(*clonedOp);
                qc::Operation* ccop_in = ccop.getOperation();
                
                auto control = ccop.getControlRegister().first;

                switch (classicControlBits[control]) {
                    case ZERO:
                        // Add nothing
                        continue;
                        break;
                    case ONE:
                        // Replace current operation with the ClassicControlledOperation "internal" operation
                        op = ccop_in;
                        break;
                    default: // Case NOT_KNOWN
                        // Replace classical controlled with controlled
                        ccop_in->getControls().insert({static_cast<unsigned int>(control)});
                        
                        auto new_op = std::make_unique<qc::StandardOperation>(ccop_in->getNqubits(), ccop_in->getControls(), ccop_in->getTargets(), ccop_in->getType(), std::vector<qc::fp>(), ccop_in->getStartingQubit());
                        qc.emplace_back(new_op);
                        continue;
                        break;
                }
                
                
            } catch (const std::bad_cast& e) {
                std::cerr << "Bad cast: " << e.what() << std::endl;
            }
        }

        std::vector<size_t> controls{};
        for (qc::Control c: op->getControls())
            controls.emplace_back(c.qubit);

        auto [act, min] = table->minimizeControls(controls);

        if (act == NEVER) {
            continue;
        }

        op->getControls().clear();
        for (auto c: min) {
            op->getControls().insert({static_cast<unsigned int>(c)});
        }

        newQc.emplace_back(op->clone());

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
                    auto inStateI = table->indexInState(i);
                    auto inStateJ = table->indexInState(j);
                    auto inStateMin = table->indexInState(min);
                    (*table)[i].getQubitState()->applyGate(inStateI, {inStateJ}, {0, 1, 1, 0});
                    (*table)[i].getQubitState()->applyGate(inStateJ, inStateMin, {0, 1, 1, 0});
                    (*table)[i].getQubitState()->applyGate(inStateI, {inStateJ}, {0, 1, 1, 0});
                    checkAmplitude(table, maxAmplitudes, i);
                    if ((*table)[i].isQubitState() && (*table)[i].getQubitState()->size() == 0) {
                        table->setTop(i);
                    }
                }
            }
            continue;
        }

        //Entangle Control Bits and Targets
        table->combine(op->getTargets()[0], min);
        checkAmplitude(table, maxAmplitudes, op->getTargets()[0]);

        if (qc::isTwoQubitGate(op->getType())) {
            //Two qubit gate
            size_t t1 = op->getTargets()[0];
            size_t t2 = op->getTargets()[1];
            auto twoQubitMat = getTwoQubitMatrix(*op);

            //Entangle second Target only present for twoQubitGates
            table->combine(t1, t2);
            checkAmplitude(table, maxAmplitudes, t1);

            if (table->isTop(t1)) {
                continue;
            }

            (*table)[t1].getQubitState()->applyTwoQubitGate(table->indexInState(t1),
                                                            table->indexInState(t2),
                                                            table->indexInState(min),
                                                            twoQubitMat);

            checkAmplitude(table, maxAmplitudes, t1);

            if ((*table)[t1].isQubitState() && (*table)[t1].getQubitState()->size() == 0) {
                table->setTop(t1);
            }

            continue;
        } else {
            //Single Qubit Gate
            size_t target = op->getTargets().begin()[0];

            if (table->isTop(target)) {
                continue;
            }

            auto singleQubitMat = getMatrix(*op);
            (*table)[target].getQubitState()->applyGate(table->indexInState(target),
                                                        table->indexInState(min),
                                                        singleQubitMat);
            checkAmplitude(table, maxAmplitudes, target);
            continue;
        }
    } //Loop over Gates

    qc.clear();

    for (auto &op: newQc) {
        qc.emplace_back(op->clone());
    }
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
