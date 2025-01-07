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
    // Use qfr to flatten compound gates
    qc::CircuitOptimizer::flattenOperations(qc);

    qc::QuantumComputation newQc(qc.getNqubits());

    auto it = qc.begin();
    while (it != qc.end()) {
        auto op = (*it).get();
        it++;

        // TODO: remove this ****************************************************************************
        std::cout << op->getName() << std::endl;
        std::cout << (*table).to_string() << std::endl;
        
        // std::cout << table->to_string() << std::endl;
        // for (auto c : op->getControls()) {
        //     std::cout << "Qubit " << c.qubit << std::endl;
        //     if (c.type == qc::Control::Type::Pos) {
        //         std::cout << "POSITIVE\n";
        //     }
        //     else if (c.type == qc::Control::Type::Neg) {
        //         std::cout << "NEGATIVE\n";
        //     }
            
        // }
        
        // for (auto t : op->getTargets()) {
        //     std::cout << t << std::endl;
        // }
        
        // auto qubit_state_or_top = (*table)[0];
        // if (qubit_state_or_top.isQubitState()) {
        //     auto qubit_state = qubit_state_or_top.getQubitState();
        //     auto state_vector = qubit_state->toStateVector(qubit_state->getNQubits());
        //     for (size_t i = 0; i < state_vector.size(); i++) {
        //         std::cout << state_vector[i] << " ";
        //     }
        //     std::cout << std::endl;
        // }
        // ************************************************************************************************

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

        if (op->getType() == qc::Reset) {
            for (auto t: op->getTargets()) {
                if (!table->purityTest(t)) {
                    table->setTop(t);
                    (*table)[t] = std::make_shared<QubitState>(1);
                    newQc.emplace_back(op->clone());
                }
                else {
                    table->resetState(t);

                    // Obtain alpha and beta values from the state of the qubit table->indexInState(t)
                    auto [alpha, beta] = (*table)[t].getQubitState()->amplitudes(table->indexInState(t));

                    std::complex<double> alpha_conj = std::conj(alpha);
                    std::complex<double> beta_conj = std::conj(beta);

                    std::complex<double> x = std::complex<double>(1) / sqrt((alpha_conj * (-alpha) - beta_conj * beta));
                    double gamma = - atan2(x.imag(), x.real());
                    alpha *= std::exp(std::complex<double>(0, gamma));
                    beta *= std::exp(std::complex<double>(0, gamma));
                    alpha_conj *= std::exp(std::complex<double>(0, gamma));
                    beta_conj *= std::exp(std::complex<double>(0, gamma));

                    double theta = 2 * atan2(std::arg(beta_conj), std::arg(alpha_conj));
                    double phi = atan2(-alpha.imag(), -alpha.real()) + atan2(beta_conj.imag(), beta_conj.real());
                    double lambda = atan2(-alpha.imag(), -alpha.real()) - atan2(beta_conj.imag(), beta_conj.real());
                    
                    newQc.emplace_back(std::make_unique<qc::StandardOperation>(op->getNqubits(), op->getControls(), op->getTargets(), qc::OpType::RZ, std::vector<qc::fp>(1, phi), op->getStartingQubit()));
                    newQc.emplace_back(std::make_unique<qc::StandardOperation>(op->getNqubits(), op->getControls(), op->getTargets(), qc::OpType::RY, std::vector<qc::fp>(1, theta), op->getStartingQubit()));
                    newQc.emplace_back(std::make_unique<qc::StandardOperation>(op->getNqubits(), op->getControls(), op->getTargets(), qc::OpType::RZ, std::vector<qc::fp>(1, lambda), op->getStartingQubit()));

                }
                
                classicControlBits[t] = ZERO;
            }
            
            continue;
        }

        if (op->getType() == qc::Measure) {
            auto nonUni = dynamic_cast<qc::NonUnitaryOperation *>(op);
            for (auto const t: nonUni->getTargets()) {
                // Check if the classical register belongs to the one used for claassical-controlled operations
                auto cl_bits = nonUni->getClassics().cbegin();
                if (*cl_bits >= table->getNQubits()) {
                    table->setTop(t);
                    newQc.emplace_back(op->clone());
                    continue;
                }

                if (table->purityTest(t)) {

                    double _probabilityMeasureZero = (*table)[t].getQubitState()->probabilityMeasureZero(table->indexInState(t));
                    double _probabilityMeasureOne = (*table)[t].getQubitState()->probabilityMeasureOne(table->indexInState(t));

                    if (_probabilityMeasureZero != 1.0 && 
                        _probabilityMeasureOne != 1.0) {
                        
                        auto [alpha, beta] = (*table)[t].getQubitState()->amplitudes(table->indexInState(t));
                        std::complex<double> alpha_conj = std::conj(alpha);
                        std::complex<double> beta_conj = std::conj(beta);

                        std::complex<double> x = std::complex<double>(1) / sqrt((alpha_conj * (-alpha) - beta_conj * beta));
                        double gamma = - atan2(x.imag(), x.real());
                        alpha *= std::exp(std::complex<double>(0, gamma));
                        beta *= std::exp(std::complex<double>(0, gamma));
                        alpha_conj *= std::exp(std::complex<double>(0, gamma));
                        beta_conj *= std::exp(std::complex<double>(0, gamma));

                        double theta = 2 * atan2(std::arg(beta_conj), std::arg(alpha_conj));
                        double phi = atan2(-alpha.imag(), -alpha.real()) + atan2(beta_conj.imag(), beta_conj.real());
                        double lambda = atan2(-alpha.imag(), -alpha.real()) - atan2(beta_conj.imag(), beta_conj.real());
                        
                        newQc.emplace_back(std::make_unique<qc::StandardOperation>(op->getNqubits(), op->getControls(), op->getTargets(), qc::OpType::RZ, std::vector<qc::fp>(1, phi), op->getStartingQubit()));
                        newQc.emplace_back(std::make_unique<qc::StandardOperation>(op->getNqubits(), op->getControls(), op->getTargets(), qc::OpType::RY, std::vector<qc::fp>(1, theta), op->getStartingQubit()));
                        newQc.emplace_back(std::make_unique<qc::StandardOperation>(op->getNqubits(), op->getControls(), op->getTargets(), qc::OpType::RZ, std::vector<qc::fp>(1, lambda), op->getStartingQubit()));

                        auto tmp = std::make_unique<qc::StandardOperation>(op->getNqubits(), op->getControls(), op->getTargets(), qc::OpType::X, std::vector<qc::fp>(), op->getStartingQubit())->clone();
                        auto probabilisticOp = std::make_unique<qc::ProbabilisticOperation>(tmp, _probabilityMeasureOne);
                        
                        newQc.emplace_back(probabilisticOp);
                        classicControlBits[t] = NOT_KNOWN;
                        table->separate(t);
                        table->setTop(t);
                    }
                    // else if probability is 0 or 1, apply nothing and and leave the qubit state unchanged
                    else if (_probabilityMeasureZero == 1.0) {
                        classicControlBits[t] = ZERO;
                    }
                    else {
                        classicControlBits[t] = ONE;
                    }
                }
                else {
                    table->setTop(t);
                    newQc.emplace_back(op->clone());
                    classicControlBits[t] = NOT_KNOWN;
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

                if (classicControlBits[control] == ONE) {
                    std::vector<size_t> controls{};
                    for (qc::Control c: ccop_in->getControls())
                        controls.emplace_back(c.qubit);

                    auto [act, min] = table->minimizeControls(controls);

                    if (act == NEVER) {
                        continue;
                    }

                    ccop_in->getControls().clear();
                    for (auto c: min) {
                        ccop_in->getControls().insert({static_cast<unsigned int>(c)});
                    }

                    newQc.emplace_back(ccop_in->clone());
                    size_t target = ccop_in->getTargets()[0];

                    // Entangle Control Bits and Targets
                    table->combine(target, min);

                    if (table->isTop(target)) {
                        continue;
                    }
                    
                    if (qc::isTwoQubitGate(ccop_in->getType())) {
                        auto mat = getTwoQubitMatrix(*ccop_in);
                        size_t target_2 = op->getTargets()[1];
                        table->combine(target, target_2);

                        checkAmplitude(table, maxAmplitudes, target);

                        if (table->isTop(target)) {
                            continue;
                        }

                        (*table)[target].getQubitState()->applyTwoQubitGate(table->indexInState(target),
                                                            table->indexInState(target_2),
                                                            table->indexInState(min),
                                                            mat);
                    }
                    else {
                        auto mat = getMatrix(*ccop_in);
                        (*table)[target].getQubitState()->applyGate(table->indexInState(target),
                                                            table->indexInState(min),
                                                            mat);
                    }
                    
                    checkAmplitude(table, maxAmplitudes, target);
                    for (auto t: op->getTargets()) {
                        table->separate(t);
                    }
                    for (auto c : op->getControls()) {
                        table->separate(c.qubit);
                    }
                }
                    
                else if (classicControlBits[control] == NOT_KNOWN) {
                    std::vector<size_t> controls{};
                    for (qc::Control c: ccop_in->getControls())
                        controls.emplace_back(c.qubit);

                    auto [act, min] = table->minimizeControls(controls);

                    if (act == NEVER) {
                        continue;
                    }

                    ccop_in->getControls().clear();
                    for (auto c: min) {
                        ccop_in->getControls().insert({static_cast<unsigned int>(c)});
                    }

                    for (auto c : ccop_in->getControls()) {
                        table->setTop(c.qubit);
                    }

                    for (auto t : ccop_in->getTargets()) {
                        table->setTop(t);
                    }

                    ccop_in->getControls().insert({static_cast<unsigned int>(control)});
                        
                    auto new_op = std::make_unique<qc::StandardOperation>(ccop_in->getNqubits(), ccop_in->getControls(), ccop_in->getTargets(), ccop_in->getType(), ccop_in->getParameter(), ccop_in->getStartingQubit());
                    newQc.emplace_back(new_op);
                }
                else { // if (classicControlBits[control] == ZERO)
                    // Add nothing
                }
                
                
            } catch (const std::bad_cast& e) {
                std::cerr << "Bad cast: " << e.what() << std::endl;
                
            }
            continue;
        }

        std::vector<size_t> controls{};
        for (qc::Control c: op->getControls()) {
            controls.emplace_back(c.qubit);
        }

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
                    for (auto t: op->getTargets()) {
                        table->separate(t);
                    }

                    for (auto c : op->getControls()) {
                        table->separate(c.qubit);
                    }
                }
            }
                
            continue;
        }

        // Entangle Control Bits and Targets
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
            for (auto t: op->getTargets()) {
                table->separate(t);
            }

            for (auto c : op->getControls()) {
                table->separate(c.qubit);
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
            for (auto t: op->getTargets()) {
                table->separate(t);
            }

            for (auto c : op->getControls()) {
                table->separate(c.qubit);
            }
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
