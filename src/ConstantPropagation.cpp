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

std::string complexToString(Complex c) {
    return std::to_string(c.real()) + 
           (c.imag() >= 0 ? "+" : "") + 
           std::to_string(c.imag()) + "i";
}

// Struct to hold quantum gate data
struct GateFromFile {
    std::string operation;
    std::vector<int> qubits;
    std::vector<double> parameters;
};

// Function to trim spaces and special characters from a string
std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" ('");
    size_t end = str.find_last_not_of(" )");
    return str.substr(start, end - start);
}

// Function that reads a file and returns a vector of QuantumGate structs
std::vector<GateFromFile> parseQuantumGatesFromFile(const std::string& filename) {
    std::vector<GateFromFile> gateList;
    std::ifstream file(filename);
    
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return gateList;  // Return empty vector on error
    }

    std::string line;
    while (std::getline(file, line)) {
        GateFromFile gate;
        std::stringstream ss(line);
        char discard;
        
        // Parsing operation name
        ss >> discard;  // Discard initial '('
        std::string op;
        std::getline(ss, op, ',');
        gate.operation = trim(op);

        // Parsing qubits list
        ss >> discard;  // Discard '['

        int qubit;

        char c;

        do {
            ss >> qubit;
            gate.qubits.push_back(qubit);
            
            c = ss.peek();

            if (c == ',') {
                ss >> discard; // Consume blank space
            }
        } while (c != ']');
        
        ss >> discard;  // Discard comma
        ss >> discard;  // Discard space after

        // Parsing parameters list (FIXED for empty [])
        ss >> discard;  // Discard '['
        if (ss.peek() == ']') {
            ss >> discard;  // If the list is empty, discard the closing bracket ']'
        } else {
            double param;
            while (ss.peek() != ']') {
                ss >> param;
                gate.parameters.push_back(param);
                if (ss.peek() == ',') ss >> discard;  // Only discard comma if present
            }
            ss >> discard;  // Discard closing bracket ']'
        }

        // Add parsed gate to the list
        gateList.push_back(gate);
    }


    file.close();
    return gateList;
}

void
ConstantPropagation::checkAmplitudes(const std::shared_ptr<UnionTable> &table, size_t maxAmplitudes) {
    for (size_t i = 0; i < table->size(); i++) {
        checkAmplitude(table, maxAmplitudes, i);
    }
}

// Function that calls a Python script to synthesize the circuit that rotates the state from |0> to the current state of the qubit t
// in the table. If the flag inverse is true, then the returned circuit rotates the state from the current state of the qubit t into |0>
void synthesize_rotation(const std::shared_ptr<UnionTable> &table, qc::QuantumComputation &newQc, qc::Qubit t, bool inverse) {

    auto state_vector = (*table)[t].getQubitState()->toStateVector();
    qc::Targets targets = table->qubitsInState((*table)[t].getQubitState());
    std::string python_call = "python ../generate_rotation.py ";

    python_call += " \"[";
    python_call += std::to_string(targets[0]);
    for (size_t i = 1; i < targets.size(); i++) {
        python_call += ", " + std::to_string(targets[i]);
    }
    python_call += "]\"";
    python_call += " \"[";
    python_call += complexToString(state_vector[0]);
    for (size_t i = 1; i < state_vector.size(); i++) {
        python_call += ", " + complexToString(state_vector[i]);
    }
    
    if (inverse) {
        python_call += "]\" True";
    }
    else {
        python_call += "]\" False";
    }

    system(python_call.c_str());

    std::vector<GateFromFile> gates = parseQuantumGatesFromFile("rotation.txt");

    for (const auto& gate : gates) {
        if (gate.operation == "ry") {
            newQc.emplace_back(std::make_unique<qc::StandardOperation>(1, gate.qubits[0], qc::OpType::RY, gate.parameters, 0));
        }
        if (gate.operation == "rz") {
            newQc.emplace_back(std::make_unique<qc::StandardOperation>(1, gate.qubits[0], qc::OpType::RZ, gate.parameters, 0));
        }
        if (gate.operation == "h") {
            newQc.emplace_back(std::make_unique<qc::StandardOperation>(1, gate.qubits[0], qc::OpType::H, gate.parameters, 0));
        }
        if (gate.operation == "cx") {
            qc::Qubit q(gate.qubits[0]);
            qc::Control c{q, qc::Control::Type::Pos};
            newQc.emplace_back(std::make_unique<qc::StandardOperation>(2, c, gate.qubits[1], qc::OpType::X, gate.parameters, 0));
        }
    }
}

void ConstantPropagation::propagate(qc::QuantumComputation &qc, size_t maxAmplitudes, size_t max_ent_group_size,
                                    const std::shared_ptr<UnionTable> &table) {                               
    std::vector<ClassicalRegisterValue> classicControlBits(qc.getNqubits(), NOT_KNOWN);
    // Use qfr to flatten compound gates
    qc::CircuitOptimizer::flattenOperations(qc);

    qc::QuantumComputation newQc(qc.getNqubits());

    auto it = qc.begin();

    while (it != qc.end()) {
        auto op = (*it).get();
        it++;

        checkAmplitudes(table, maxAmplitudes);

        if (table->allTop() && op->getType() != qc::Reset) {
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
                    if ((*table)[t].isQubitState() && (*table)[t].getQubitState()->getNQubits() <= max_ent_group_size) {
                        // Perform rotation from the current state to |0>
                        synthesize_rotation(table, newQc, t, true);

                        // Reset t-th qubit
                        table->resetState(t);

                        // Add gates to perform rotation from state |0> to the state before reset
                        synthesize_rotation(table, newQc, t, false);
                    
                        for (auto q : table->qubitsInState((*table)[t].getQubitState())) {
                            table->separate(q);
                            classicControlBits[q] = NOT_KNOWN;
                        }
                    }
                    else {
                        if ((*table)[t].isQubitState()) {
                            for (auto q : table->qubitsInState((*table)[t].getQubitState())) {
                                classicControlBits[q] = NOT_KNOWN;
                            }
                            table->setTop(t);
                        }
                        newQc.emplace_back(op->clone());
                    }
                }
                else if ((*table)[t].isQubitState()) {
                    double _probabilityMeasureZero = (*table)[t].getQubitState()->probabilityMeasureZero(table->indexInState(t));
                    double _probabilityMeasureOne = (*table)[t].getQubitState()->probabilityMeasureOne(table->indexInState(t));

                    if (_probabilityMeasureOne == 1.0) {
                        newQc.emplace_back(std::make_unique<qc::StandardOperation>(op->getNqubits(), op->getControls(), op->getTargets(), qc::OpType::X, std::vector<qc::fp>(0), op->getStartingQubit()));
                    }
                    else if (_probabilityMeasureZero != 1.0 && _probabilityMeasureOne != 1.0) {
                        synthesize_rotation(table, newQc, t, true);
                    }
                }

                table->resetState(t);
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

                    if (_probabilityMeasureZero != 1.0 && _probabilityMeasureOne != 1.0) {
                        // Synthesize the circuit to rotate the state to |0>
                        synthesize_rotation(table, newQc, t, true);

                        // Add probabilistc X gate
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
                else if ((*table)[t].isQubitState() && (*table)[t].getQubitState()->getNQubits() <= max_ent_group_size) {
                    // Synthesize the circuit to rotate the state to |0>
                    synthesize_rotation(table, newQc, t, true);

                    qc::Targets targets = table->qubitsInState((*table)[t].getQubitState());

                    // Construct and append the big probabilistic gate
                    std::unordered_map<BitSet, Complex> state = (*table)[t].getQubitState()->getQuantumState();
                    std::vector<double> probabilities;
                    std::vector<std::vector<bool>> basis_states;

                    for (auto [key, value] : state) {
                        probabilities.push_back(value.norm());
                        basis_states.push_back(key.getBits());
                    }
                    qc::Controls controls;

                    auto tmp = std::make_unique<qc::StandardOperation>(targets.size(), controls, targets, qc::OpType::X, std::vector<qc::fp>(), 0)->clone();
                    auto probabilisticOp = std::make_unique<qc::BigProbabilisticOperation>(tmp, probabilities, basis_states);
                    newQc.emplace_back(probabilisticOp);

                    targets.size();

                    for (auto i : targets) {
                        classicControlBits[i] = NOT_KNOWN;
                        table->separate(i);
                        table->setTop(i);
                    }
                }
                else {
                    if ((*table)[t].isQubitState()) {
                        table->setTop(t);
                        classicControlBits[t] = NOT_KNOWN;
                    }
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

std::shared_ptr<UnionTable> ConstantPropagation::propagate(qc::QuantumComputation &qc, size_t maxAmplitudes,  size_t max_ent_group_size) {
    auto table = std::make_shared<UnionTable>(qc.getNqubits());
    propagate(qc, maxAmplitudes, max_ent_group_size, table);
    return table;
}

void ConstantPropagation::optimize(qc::QuantumComputation &qc) {
    optimize(qc, MAX_AMPLITUDES, MAX_ENT_GROUP_SIZE);
}

void ConstantPropagation::optimize(qc::QuantumComputation &qc, size_t maxAmplitudes) {
    propagate(qc, maxAmplitudes, MAX_ENT_GROUP_SIZE);
}

void ConstantPropagation::optimize(qc::QuantumComputation &qc, size_t maxAmplitudes, size_t max_ent_group_size) {
    propagate(qc, maxAmplitudes, max_ent_group_size);
}
