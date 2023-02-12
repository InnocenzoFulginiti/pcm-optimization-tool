//
// Created by Jakob on 31/01/2023.
//

#include <memory>
#include <iostream>
#include "../include/UnionTable.hpp"

UnionTable::UnionTable(size_t nQubits) {
    this->nQubits = nQubits;
    this->quReg = new QubitStateOrTop[nQubits];
    for (int i = 0; i < (int) nQubits; i++) {
        this->quReg[i] = std::make_shared<QubitState>(1);
    }
}

UnionTable::~UnionTable() {
    delete[] this->quReg;
}

void UnionTable::setTop(size_t qubit) {
    if(std::holds_alternative<TOP>(this->quReg[qubit])) {
        return;
    }

    auto qubitState = std::get<std::shared_ptr<QubitState>>(this->quReg[qubit]);
    for (size_t i = 0; i < nQubits; i++) {
        if (!std::holds_alternative<TOP>(this->quReg[i]) &&
                std::get<std::shared_ptr<QubitState>>(this->quReg[i]) == qubitState) {
            this->quReg[i] = TOP::T;
        }
    }
}

//Combine UnionTable entries of two qubits. If either is Top, the result is Top.
//If both are QubitStates, the result is the tensor product of the two.
void UnionTable::combine(size_t qubit1, size_t qubit2) {
    if(qubit1 == qubit2)
        return;

    if (std::holds_alternative<TOP>(this->quReg[qubit1])) {
        this->quReg[qubit2] = TOP::T;
        return;
    }

    if (std::holds_alternative<TOP>(this->quReg[qubit2])) {
        this->quReg[qubit1] = TOP::T;
        return;
    }

    std::shared_ptr<QubitState> qubitState1 = std::get<std::shared_ptr<QubitState>>(this->quReg[qubit1]);
    std::shared_ptr<QubitState> qubitState2 = std::get<std::shared_ptr<QubitState>>(this->quReg[qubit2]);

    if (qubitState1 == qubitState2) {
        return;
    }

    //Find Indices of qubitStates in table
    std::vector<int> qubitState1Indices{};
    std::vector<int> qubitState2Indices{};

    for (size_t i = 0; i < this->nQubits; i++) {
        if(std::holds_alternative<TOP>(this->quReg[i]))
            continue;

        if (std::get<std::shared_ptr<QubitState>>(this->quReg[i]) == qubitState1) {
            qubitState1Indices.emplace_back(i);
        }
        if (std::get<std::shared_ptr<QubitState>>(this->quReg[i]) == qubitState2) {
            qubitState2Indices.emplace_back(i);
        }
    }

    std::shared_ptr<QubitState> newQubitState = QubitState::combine(qubitState1, qubitState1Indices,
                                                                    qubitState2, qubitState2Indices);

    //Replace old qubitStates with new one
    for (size_t i = 0; i < nQubits; i++) {
        if(std::holds_alternative<TOP>(this->quReg[i])) {
            continue;
        } else if (std::get<std::shared_ptr<QubitState>>(this->quReg[i]) == qubitState1
            || std::get<std::shared_ptr<QubitState>>(this->quReg[i]) == qubitState2) {
            this->quReg[i] = newQubitState;
        }
    }
}

void UnionTable::print(std::ostream &os) const {
    os << this->to_string() << std::endl;
}

std::string UnionTable::to_string() const {
    size_t commonPrefix;
    if (nQubits > 0) {
        size_t i = 0;
        for (; i < nQubits; i++) {
            if(std::holds_alternative<std::shared_ptr<QubitState>>(this->quReg[i])) {
                commonPrefix = (size_t) std::get<std::shared_ptr<QubitState>>(this->quReg[i]).get();
            }
        }
        for (; i < nQubits; i++) {
            commonPrefix &= (size_t) std::get<std::shared_ptr<QubitState>>(this->quReg[i]).get();
        }

        commonPrefix = ~commonPrefix;
    }


    std::stringstream os;
    for (int i = 0; i < (int) nQubits; i++) {
        os << i << ": -> ";
        if (std::holds_alternative<TOP>(this->quReg[i])) {
            os << "Top" << std::endl;
        } else {
            std::shared_ptr<QubitState> qubitState = std::get<std::shared_ptr<QubitState>>(this->quReg[i]);
            os << "@" << std::hex << (commonPrefix & (size_t) qubitState.get()) << std::dec << ": ";
            qubitState->print(os);
            os << std::endl;
        }
    }
    return os.str();
}

bool UnionTable::canActivate(std::vector<size_t> controls) const {
    for (size_t i = 0; i < controls.size(); ++i) {
        size_t target = controls[i];
        if (std::holds_alternative<TOP>(this->quReg[target]))
            return false;

        auto targetState = std::get<std::shared_ptr<QubitState>>(this->quReg[target]);


        std::vector<size_t> qubitsInSameState = this->qubitsInSameState(target);
        //Add first index of target to list of indices
        std::vector<size_t> internalIndices{this->indexInState(target)};

        //Find other indices in controls that might be in the same state
        for (size_t otherIndex: qubitsInSameState) {
            for (size_t j = i + 1; j < controls.size(); ++j) {
                if (controls[j] == otherIndex) {
                    internalIndices.emplace_back(this->indexInState(otherIndex));
                    controls.erase(controls.begin() + j);
                    break;
                }
            }
        }

        //Check if group can activate
        if (!targetState->canActivate(internalIndices))
            return false;
    }

    //If all groups can activate, return true
    return true;
}

bool UnionTable::anyIsTop(std::vector<size_t> indices) {
    return std::any_of(indices.begin(), indices.end(),
                       [this](size_t index) { return this->isTop(index); }
    );
}

std::pair<size_t, size_t> UnionTable::countActivations(std::vector<size_t> controls) {
    size_t zeros = 0;
    size_t ones = 0;

    if (this->anyIsTop(controls)) {
        //TODO: Error handling
        return {-1, -1};
    }

    for (size_t i = 0; i < controls.size(); ++i) {
        size_t target = controls[i];

        auto targetState = std::get<std::shared_ptr<QubitState>>(this->quReg[target]);

        std::vector<size_t> qubitsInSameState = this->qubitsInSameState(target);
        //Add first index of target to list of indices
        std::vector<size_t> internalIndices{this->indexInState(target)};

        //Find other indices in controls that might be in the same state
        for (size_t otherIndex: qubitsInSameState) {
            for (size_t j = i + 1; j < controls.size(); ++j) {
                if (controls[j] == otherIndex) {
                    internalIndices.emplace_back(this->indexInState(otherIndex));
                    controls.erase(controls.begin() + j);
                    break;
                }
            }
        }

        //Check if group can activate
        std::pair counts = targetState->countActivations(internalIndices);
        zeros += counts.first;
        ones += counts.second;
    }

    //If all groups can activate, return true
    return {zeros, ones};
}

bool UnionTable::isTop(size_t index) const {
    return std::holds_alternative<TOP>(this->quReg[index]);
}

std::vector<size_t> UnionTable::qubitsInSameState(size_t qubit) const {
    QubitStateOrTop isTop = this->quReg[qubit];
    if (std::holds_alternative<TOP>(isTop))
        return {};

    std::shared_ptr<QubitState> qubitState = std::get<std::shared_ptr<QubitState>>(isTop);

    std::vector<size_t> qubitsInSameState{};
    for (int i = 0; i < (int) nQubits; ++i) {
        if (std::holds_alternative<TOP>(this->quReg[i]))
            continue;

        std::shared_ptr<QubitState> otherQubitState = std::get<std::shared_ptr<QubitState>>(this->quReg[i]);
        if (qubitState == otherQubitState)
            qubitsInSameState.emplace_back(i);
    }

    return qubitsInSameState;
}

bool UnionTable::canActivate(size_t qubit) const {
    QubitStateOrTop target = this->quReg[qubit];
    if (std::holds_alternative<TOP>(target)) {
        //TODO: Error
        return false;
    }

    std::shared_ptr<QubitState> targetState = std::get<std::shared_ptr<QubitState>>(target);
    size_t inStateIndex = this->indexInState(qubit);

    return targetState->canActivate(inStateIndex);
}

size_t UnionTable::indexInState(size_t qubit) const {
    QubitStateOrTop target = this->quReg[qubit];

    if (std::holds_alternative<TOP>(target)) {
        return -1;
    }

    std::shared_ptr<QubitState> targetState = std::get<std::shared_ptr<QubitState>>(target);

    size_t inStateIndex = 0;
    for (size_t i = 0; i < qubit; ++i) {
        auto entry = this->quReg[i];
        if (std::holds_alternative<TOP>(entry)) {
            continue;
        } else {
            std::shared_ptr<QubitState> qubitState = std::get<std::shared_ptr<QubitState>>(entry);
            if (qubitState == targetState) {
                inStateIndex++;
            }
        }
    }

    return inStateIndex;
}

void UnionTable::combine(size_t qubit1, std::vector<size_t> otherQubits) {
    otherQubits.emplace_back(qubit1);
    this->combine(otherQubits);
}

void UnionTable::combine(std::vector<size_t> qubits) {
    size_t first = qubits[0];

    for (size_t qubit: qubits) {
        this->combine(first, qubit);
    }
}
