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
    if(this->quReg[qubit].isTop()) {
        return;
    }

    auto qubitState = this->quReg[qubit].getQubitState();
    for (size_t i = 0; i < nQubits; i++) {
        if (!this->quReg[i].isTop() &&
                this->quReg[i].getQubitState() == qubitState) {
            this->quReg[i] = TOP::T;
        }
    }
}

//Combine UnionTable entries of two qubits. If either is Top, the result is Top.
//If both are QubitStates, the result is the tensor product of the two.
void UnionTable::combine(size_t qubit1, size_t qubit2) {
    if(qubit1 == qubit2)
        return;

    if (this->quReg[qubit1].isTop()) {
        this->quReg[qubit2] = TOP::T;
        return;
    }

    if (this->quReg[qubit2].isTop()) {
        this->quReg[qubit1] = TOP::T;
        return;
    }

    std::shared_ptr<QubitState> qubitState1 = this->quReg[qubit1].getQubitState();
    std::shared_ptr<QubitState> qubitState2 = this->quReg[qubit2].getQubitState();

    if (qubitState1 == qubitState2) {
        return;
    }

    //Find Indices of qubitStates in table
    std::vector<int> qubitState1Indices{};
    std::vector<int> qubitState2Indices{};

    for (size_t i = 0; i < this->nQubits; i++) {
        if(this->quReg[i].isTop())
            continue;

        if (this->quReg[i].getQubitState() == qubitState1) {
            qubitState1Indices.emplace_back(i);
        }
        if (this->quReg[i].getQubitState() == qubitState2) {
            qubitState2Indices.emplace_back(i);
        }
    }

    std::shared_ptr<QubitState> newQubitState = QubitState::combine(qubitState1, qubitState1Indices,
                                                                    qubitState2, qubitState2Indices);

    //Replace old qubitStates with new one
    for (size_t i = 0; i < nQubits; i++) {
        if(this->quReg[i].isTop()) {
            continue;
        } else if (this->quReg[i].getQubitState() == qubitState1
            || this->quReg[i].getQubitState() == qubitState2) {
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
            if(this->quReg[i].isQubitState()) {
                commonPrefix = (size_t) this->quReg[i].getQubitState().get();
            }
        }
        for (; i < nQubits; i++) {
            commonPrefix &= (size_t) this->quReg[i].getQubitState().get();
        }

        commonPrefix = ~commonPrefix;
    }


    std::stringstream os;
    for (int i = 0; i < (int) nQubits; i++) {
        os << i << ": -> ";
        if (this->quReg[i].isTop()) {
            os << "Top" << std::endl;
        } else {
            std::shared_ptr<QubitState> qubitState = this->quReg[i].getQubitState();
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
        if (this->quReg[target].isTop())
            return false;

        auto targetState = this->quReg[target].getQubitState();


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

        auto targetState = this->quReg[target].getQubitState();

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
    return this->quReg[index].isTop();
}

std::vector<size_t> UnionTable::qubitsInSameState(size_t qubit) const {
    if (this->quReg[qubit].isTop())
        return {};

    std::shared_ptr<QubitState> qubitState = this->quReg[qubit].getQubitState();

    std::vector<size_t> qubitsInSameState{};
    for (int i = 0; i < (int) nQubits; ++i) {
        if (this->quReg[i].isTop())
            continue;

        std::shared_ptr<QubitState> otherQubitState = this->quReg[i].getQubitState();
        if (qubitState == otherQubitState)
            qubitsInSameState.emplace_back(i);
    }

    return qubitsInSameState;
}

bool UnionTable::canActivate(size_t qubit) const {
    QubitStateOrTop target = this->quReg[qubit];
    if (target.isTop()) {
        //TODO: Error
        return false;
    }

    std::shared_ptr<QubitState> targetState = target.getQubitState();
    size_t inStateIndex = this->indexInState(qubit);

    return targetState->canActivate(inStateIndex);
}

size_t UnionTable::indexInState(size_t qubit) const {
    QubitStateOrTop target = this->quReg[qubit];

    if (target.isTop()) {
        return -1;
    }

    std::shared_ptr<QubitState> targetState = target.getQubitState();

    size_t inStateIndex = 0;
    for (size_t i = 0; i < qubit; ++i) {
        auto entry = this->quReg[i];
        if (entry.isTop()) {
            continue;
        } else {
            std::shared_ptr<QubitState> qubitState = entry.getQubitState();
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
