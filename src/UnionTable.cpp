//
// Created by Jakob on 31/01/2023.
//

#include <memory>
#include <iostream>
#include <set>
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
    if (this->quReg[qubit].isTop()) {
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
    if (qubit1 == qubit2)
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
        if (this->quReg[i].isTop())
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
        if (this->quReg[i].isTop()) {
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
            if (this->quReg[i].isQubitState()) {
                commonPrefix = (size_t)
                        this->quReg[i].getQubitState().get();
            }
        }
        for (; i < nQubits; i++) {
            commonPrefix &= (size_t)
                    this->quReg[i].getQubitState().get();
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
            os << "@" << std::hex << (commonPrefix & (size_t) qubitState.get()) << std::dec << ":\t";
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

void UnionTable::swap(size_t q1, size_t q2) {
    auto s1 = this->quReg[q1];
    auto s2 = this->quReg[q2];

    if (s1.isTop() && s2.isTop()) {
        return;
    }

    int oldS1Index = -1;
    int oldS2Index = -1;


    if (s1.isTop() && s2.isQubitState()) {
        oldS2Index = this->indexInState(q2);
        this->quReg[q1] = s2;
        this->quReg[q2] = TOP::T;
    } else if (s1.isQubitState() && s2.isTop()) {
        oldS1Index = this->indexInState(q1);
        this->quReg[q2] = s1;
        this->quReg[q1] = TOP::T;
    } else {
        auto qs1 = s1.getQubitState();
        auto qs2 = s2.getQubitState();
        if (qs1.get() == qs2.get()) {
            //SWAP in entangled group
            qs1->swapIndex(this->indexInState(q1),
                           this->indexInState(q2));
            return;
        }
        //SWAP between two groups
        oldS1Index = this->indexInState(q1);
        oldS2Index = this->indexInState(q2);

        this->quReg[q1] = qs2;
        this->quReg[q2] = qs1;
    }

    //If internal indices have changed, rearrange
    if (oldS1Index >= 0) {
        s1.getQubitState()->reorderIndex(oldS1Index, this->indexInState(q2));
    }
    if (oldS2Index >= 0) {
        s2.getQubitState()->reorderIndex(oldS2Index, this->indexInState(q1));
    }
}

bool UnionTable::isAlwaysOne(size_t q) {
    if (quReg[q].isTop())
        return false;

    return quReg[q].getQubitState()->alwaysActivated({this->indexInState(q)});
}

bool UnionTable::isAlwaysZero(size_t q) {
    if (quReg[q].isTop())
        return false;

    return quReg[q].getQubitState()->neverActivated({indexInState(q)});
}

std::pair<ActivationState, std::vector<size_t>> UnionTable::minimizeControls(std::vector<size_t> controls) {
    //If there is a control state that is always 0 --> never activated
    for (size_t control: controls) {
        if (isAlwaysZero(control)) {
            return {ActivationState::NEVER, {}};
        }
    }

    std::vector<size_t> minimizedControls{};
    //Add all that are TOP
    bool controlsContainTop = false;
    for (size_t i = 0; i < controls.size(); ++i) {
        if (this->quReg[controls[i]].isTop()) {
            minimizedControls.emplace_back(controls[i]);
            controls.erase(controls.begin() + i);
            i--;
            controlsContainTop = true;
        }
    }

    //Remove those that are always one
    for (size_t i = 0; i < controls.size(); ++i) {
        if (isAlwaysOne(controls[i])) {
            controls.erase(controls.begin() + i);
            i--;
        }
    }

    if (controls.empty()) {
        return {minimizedControls.empty() ? ActivationState::ALWAYS : ActivationState::UNKNOWN,
                minimizedControls};
    }

    //Potential can only be within a group at this point, because we removed all always 0 and always 1
    size_t groups = 0;
    std::vector<std::vector<size_t>> groupIndices{};
    std::vector<QubitState *> groupStates{};

    for (auto control: controls) {
        QubitState *cs = this->quReg[control].getQubitState().get();
        bool found = false;
        for (size_t i = 0; i < groupStates.size(); ++i) {
            if (groupStates[i] == cs) {
                groupIndices[i].emplace_back(control);
                found = true;
                break;
            }
        }
        if (!found) {
            groupStates.emplace_back(cs);
            groupIndices.emplace_back(std::vector<size_t>{control});
            groups++;
        }
    }

    for (size_t groupI = 0; groupI < groups; ++groupI) {
        if (groupIndices[groupI].size() == 1) {
            minimizedControls.emplace_back(groupIndices[groupI][0]);
            continue;
        }

        //See for each index if necessary
        for (size_t indexI = 0; indexI < groupIndices[groupI].size(); indexI++) {
            //For each key calculate result without index
            size_t index = groupIndices[groupI][indexI];

            bool isRedundant = true;
            for (auto const &[key, value]: *groupStates[groupI]) {
                bool res = true;
                for (size_t calcI: groupIndices[groupI]) {
                    if (calcI == index) continue;

                    if (!key[indexInState(calcI)]) {
                        res = false;
                        break;
                    }
                }
                if (res && !key[indexInState(index)]) {
                    isRedundant = false;
                    break;
                }
            }

            if (isRedundant) {
                groupIndices[groupI].erase(groupIndices[groupI].begin() + indexI);
                indexI--;
            } else {
                minimizedControls.emplace_back(index);
            }
        }
    }

    if (controlsContainTop) {
        return {ActivationState::UNKNOWN,
                minimizedControls};
    } else {
        return {ActivationState::SOMETIMES,
                minimizedControls};
    }
}

std::shared_ptr<UnionTable> UnionTable::clone() const {
    auto newTable = std::make_shared<UnionTable>(this->nQubits);

    std::vector<size_t> leftForClone{};
    for (size_t i = 0; i < this->nQubits; ++i) {
        leftForClone.emplace_back(i);
    }

    while (!leftForClone.empty()) {
        size_t t = leftForClone[0];
        leftForClone.erase(leftForClone.begin());
        if (this->quReg[t].isTop()) {
            newTable->quReg[t] = TOP::T;
            continue;
        } else {
            newTable->quReg[t] = this->quReg[t].getQubitState()->clone();

            //See if state is entangled and set
            for (size_t i = 0; i < leftForClone.size(); i++) {
                size_t o = leftForClone[i];
                if (this->quReg[o].isQubitState() &&
                    this->quReg[o].getQubitState().get() == this->quReg[t].getQubitState().get()) {
                    newTable->quReg[o] = newTable->quReg[t];
                    leftForClone.erase(leftForClone.begin() + i);
                    i--;
                }
            }
        }
    }

    return newTable;
}

std::vector<size_t> UnionTable::indexInState(const std::vector<size_t> &qubit) const {
    std::vector<size_t> indices{};
    for (size_t q: qubit) {
        indices.emplace_back(this->indexInState(q));
    }

    return indices;
}
