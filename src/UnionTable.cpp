//
// Created by Jakob on 31/01/2023.
//

#include <memory>
#include <iostream>
#include <set>
#include "../include/UnionTable.hpp"

UnionTable::UnionTable(size_t _nQubits) {
    this->nQubits = _nQubits;
    this->quReg = new QubitStateOrTop[_nQubits];
    for (size_t i = 0; i < _nQubits; i++) {
        this->quReg[i] = std::make_shared<QubitState>(1);
    }
}

UnionTable::~UnionTable() {
    delete[] this->quReg;
}

bool UnionTable::purityTest(size_t qubit) {
    if (this->quReg[qubit].isTop()) {
        return false;
    }

    QubitState qubitState = *(this->quReg[qubit].getQubitState().get());

    Complex ratio;
    std::map<std::vector<bool>, Complex> a0;
    std::map<std::vector<bool>, Complex> a1;
    size_t _indexInState = this->indexInState(qubit);

    for (auto tmp : qubitState) {
        if (tmp.first[_indexInState] == 0) {
            std::vector<bool> tmpKey = tmp.first.getBits();
            tmpKey.erase(tmpKey.begin() + static_cast<std::ptrdiff_t>(_indexInState));
            a0[tmpKey] = tmp.second;
        }
        else {
            std::vector<bool> tmpKey = tmp.first.getBits();
            tmpKey.erase(tmpKey.begin() + static_cast<std::ptrdiff_t>(_indexInState));
            a1[tmpKey] = tmp.second;
        }
    }

    if (a0.size() == 0 || a1.size() == 0) {
        return true;
    }

    if (a0.size() != a1.size()) {
        return false;
    }

    for (auto tmp : a0) {
        if (a1.find(tmp.first) != a1.end()) {

            Complex new_ratio = tmp.second / a1[tmp.first];
            if (ratio != Complex() && ratio != new_ratio) {
                return false;
            }

            ratio = new_ratio;
            a1.erase(tmp.first);
        }
        else {
            return false;
        }
    }

    return true;
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
        this->setTop(qubit2);
        return;
    }

    if (this->quReg[qubit2].isTop()) {
        this->setTop(qubit1);
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

[[maybe_unused]] void UnionTable::print(std::ostream &os) const {
    os << this->to_string() << std::endl;
}

std::string UnionTable::to_string() const {
    size_t i = 0;
    size_t commonPrefix = 0xFFFFFFFFFFFFFFFF;
    size_t compPref;
    for (i = 0; i < nQubits; i++) {
        if (quReg[i].isTop()) {
            continue;
        } else {
            compPref = reinterpret_cast<size_t>(quReg[i].getQubitState().get());
            break;
        }
    }

    for (; i < nQubits; i++) {
        if (quReg[i].isTop()) {
            continue;
        } else {
            size_t currentPrefix = reinterpret_cast<size_t>(quReg[i].getQubitState().get());
            while ((commonPrefix & currentPrefix) != (commonPrefix & compPref)) {
                commonPrefix <<= 1;
            }
        }
    }

    commonPrefix = ~commonPrefix;

    std::stringstream os;
    for (i = 0; i < nQubits; i++) {
        os << i << ": -> ";
        if (this->quReg[i].isTop()) {
            os << "Top" << std::endl;
        } else {
            std::shared_ptr<QubitState> qubitState = this->quReg[i].getQubitState();
            os << "@" << std::hex << (commonPrefix & reinterpret_cast<size_t>(qubitState.get())) << std::dec << ":\t";
            qubitState->print(os);
            os << std::endl;
        }
    }
    return os.str();
}

bool UnionTable::isTop(size_t index) const {
    return this->quReg[index].isTop();
}

bool UnionTable::allTop() {
    for (size_t i = 0; i < nQubits; i++) {
        if (!quReg[i].isTop()) {
            return false;
        }
    }

    return true;
}

size_t UnionTable::indexInState(size_t qubit) const {
    QubitStateOrTop target = this->quReg[qubit];

    if (target.isTop()) {
        return 0;
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

    size_t oldS1Index = 0;
    size_t oldS2Index = 0;

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

    //If internal indices have changed, rearrange them
    if (s1.isQubitState()) {
        s1.getQubitState()->reorderIndex(oldS1Index, this->indexInState(q2));
    }
    if (s2.isQubitState()) {
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
    if (controls.empty()) {
        return {ActivationState::ALWAYS, {}};
    }

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
            controls.erase(controls.begin() + static_cast<long long>(i));
            i--;
            controlsContainTop = true;
        }
    }

    //Remove those that are always one
    for (size_t i = 0; i < controls.size(); ++i) {
        if (isAlwaysOne(controls[i])) {
            controls.erase(controls.begin() + static_cast<long long>(i));
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

        bool groupCanActivate = false;

        //See for each index if necessary
        for (size_t indexI = 0; indexI < groupIndices[groupI].size(); indexI++) {
            //For each key calculate result without index
            size_t index = groupIndices[groupI][indexI];

            bool isRedundant = true;
            for (auto const &[key, value]: *groupStates[groupI]) {
                if (!isRedundant && groupCanActivate) break;

                bool res = true;
                for (size_t calcI: groupIndices[groupI]) {
                    if (calcI == index) continue;

                    if (!key[indexInState(calcI)]) {
                        res = false;
                        break;
                    }
                }

                if (!groupCanActivate && res && key[indexInState(index)]) {
                    groupCanActivate = true;
                }

                if (isRedundant && res && !key[indexInState(index)]) {
                    isRedundant = false;
                }
            }

            if (!groupCanActivate) {
                return {ActivationState::NEVER, {}};
            }

            if (isRedundant) {
                groupIndices[groupI].erase(groupIndices[groupI].begin() + static_cast<long long>(indexI));
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
                    leftForClone.erase(leftForClone.begin() + static_cast<long long>(i));
                    i--;
                }
            }
        }
    }

    return newTable;
}

std::vector<size_t> UnionTable::indexInState(const std::vector<size_t> &qubits) const {
    std::vector<size_t> indices{};
    indices.reserve(qubits.size());
    for (size_t q: qubits) {
        indices.emplace_back(this->indexInState(q));
    }

    return indices;
}

bool UnionTable::operator==(const UnionTable &other) const {
    if (this->nQubits != other.nQubits) return false;

    for (size_t i = 0; i < this->nQubits; ++i) {
        if (this->quReg[i] != other.quReg[i]) return false;
    }

    return true;
}

void UnionTable::resetState(size_t qubit) {
    if (this->quReg[qubit].isTop() || this->purityTest(qubit)) {
        this->quReg[qubit] = std::make_shared<QubitState>(1);
    } else {
        auto target = this->quReg[qubit].getQubitState();
        std::shared_ptr<QubitState> newQubitState = std::make_shared<QubitState>(QubitState(target->getNQubits()));
        newQubitState->clear();
        size_t index = this->indexInState(qubit);

        for (const auto &[key, value]: *target) {
            if (key[index] == false) {
                (*newQubitState)[key] = value;
            }
        }

        newQubitState->removeZeroEntries();
        newQubitState->normalize();
        
        for (size_t i = 0; i < this->nQubits; i++) {
            if (this->quReg[i].isQubitState() && this->quReg[i].getQubitState() == target) {
                this->quReg[i] = newQubitState;
            }
        }

        // // Indices of the rest of the qubits separated from the target qubit
        // std::vector<size_t> indices{};

        // auto target = this->quReg[qubit].getQubitState();
        // std::shared_ptr<QubitState> newQubitState = std::make_shared<QubitState>(QubitState(target->getNQubits()));
        // newQubitState->clear();

        // // Identify the rest of the qubits
        // for (size_t i = 0; i < this->nQubits; i++) {
        //     if (i!= qubit && this->quReg[i].isQubitState() && this->quReg[i].getQubitState() == target) {
        //         indices.push_back(i);
        //     }
        // }
        
        // size_t index = this->indexInState(qubit);

        // for (const auto &[key, value]: *target) {
        //     std::vector<bool> new_key_tmp = key.getBits();
        //     new_key_tmp.erase(new_key_tmp.begin() + index);
        //     BitSet new_key(new_key_tmp.size(), new_key_tmp);
        //     if ((*newQubitState)[new_key] == 0) {
        //         if (key[index] == false) {
        //             (*newQubitState)[new_key] = value;
        //         }
        //     }
        // }
        // newQubitState->removeZeroEntries();
        // newQubitState->normalize();
        // for (const auto &i: indices) {
        //     this->quReg[i] = newQubitState;
        // }

        // this->quReg[qubit] = std::make_shared<QubitState>(1);
    }
}

void UnionTable::separate(size_t qubit) {
    if (!this->quReg[qubit].isTop() && this->purityTest(qubit)) {

        // Indices of the rest of the qubits separated from the target qubit
        std::vector<size_t> indices{};

        auto target = this->quReg[qubit].getQubitState();
        std::shared_ptr<QubitState> newQubitState = std::make_shared<QubitState>(QubitState(target->getNQubits() - 1));
        newQubitState->clear();
        
        // Identify the rest of the qubits
        for (size_t i = 0; i < this->nQubits; i++) {
            if (i!= qubit && this->quReg[i].isQubitState() && this->quReg[i].getQubitState() == target) {
                indices.push_back(i);
            }
        }
        
        size_t index = this->indexInState(qubit);

        auto [alpha, beta] = this->quReg[qubit].getQubitState()->amplitudes(index);
        
        for (const auto &[key, value]: *target) {
            std::vector<bool> new_key_tmp = key.getBits();
            new_key_tmp.erase(new_key_tmp.begin() + index);
            BitSet new_key(new_key_tmp.size(), new_key_tmp);
            if ((*newQubitState)[new_key] == 0) {
                if (key[index] == false) {
                    Complex new_value(value / Complex(alpha.real(), alpha.imag()));
                    (*newQubitState)[new_key] = new_value;
                }
                else {
                    Complex new_value(value / Complex(beta.real(), beta.imag()));
                    (*newQubitState)[new_key] = new_value;
                }
            }
        }

        newQubitState->removeZeroEntries();

        for (const auto &i: indices) {
            this->quReg[i] = newQubitState;
        }

        std::shared_ptr<QubitState> newQubitStateForQubit = std::make_shared<QubitState>(QubitState(1));
        newQubitStateForQubit->clear();

        (*newQubitStateForQubit)[BitSet(1, std::vector<bool>(1, false))] = Complex(Complex(alpha.real(), alpha.imag()));
        (*newQubitStateForQubit)[BitSet(1, std::vector<bool>(1, true))] = Complex(Complex(beta.real(), beta.imag()));
        newQubitStateForQubit->removeZeroEntries();
        this->quReg[qubit] = newQubitStateForQubit;
    }
}

std::vector<qc::Qubit> UnionTable::qubitsInState(std::shared_ptr<QubitState> state) const {
    std::vector<qc::Qubit> qubits;

    for (size_t i = 0; i < this->nQubits; i++) {
        if (this->quReg[i].isQubitState() && this->quReg[i].getQubitState() == state) {
            qubits.push_back(i);
        }
    }

    return qubits;
}
