//
// Created by Jakob on 31/01/2023.
//

#include <bitset>

#include "../include/QubitState.hpp"

QubitState::QubitState(size_t nQubits) {
    this->nQubits = nQubits;
    this->map = std::map<BitSet, Complex>();

    map[BitSet(1, 0)] = Complex(1, 0);
}

size_t QubitState::size() const {
    return this->map.size();
}

size_t QubitState::getNQubits() const {
    return this->nQubits;
}

int QubitState::countQubitIisZero(int qubit) const {
    int count = 0;
    for (auto const &[key, val]: this->map) {
        if (!key[qubit]) {
            count++;
        }
    }
    return count;
}

int QubitState::countQubitIisOne(int qubit) const {
    int count = 0;
    for (auto const &[key, val]: this->map) {
        if (key[qubit]) {
            count++;
        }
    }
    return count;
}

void QubitState::print(std::ostream &os) const {
    os << this->to_string();
}

std::string QubitState::to_string() const {
    std::string str;
    for (auto const &[key, val]: this->map) {
        str += "|" + key.to_string() + "> -> " + val.to_string() + ", ";
    }

    return str;
}

double QubitState::norm() const {
    double norm = 0;
    for (auto const &[key, val]: this->map) {
        norm += val.norm();
    }
    return norm;
}

void QubitState::normalize() {
    double norm = this->norm();
    norm = sqrt(norm);
    for (auto &[key, val]: this->map) {
        val /= norm;
    }
}

std::shared_ptr<QubitState>
QubitState::combine(const std::shared_ptr<QubitState> &qubitState1, std::vector<int> indices1,
                    const std::shared_ptr<QubitState> &qubitState2, std::vector<int> indices2) {
    //Find how to interlace indices (so they are sorted again)
    std::vector<bool> interlace{}; //true = qubitState1, false = qubitState2
    while (!indices1.empty() || !indices2.empty()) {
        //Only one of the two is empty
        if (indices1.empty()) {
            interlace.emplace_back(false);
            indices2.erase(indices2.begin());
            continue;
        }
        if (indices2.empty()) {
            interlace.emplace_back(true);
            indices1.erase(indices1.begin());
            continue;
        }

        //Both are not empty
        if (indices1[0] < indices2[0]) {
            interlace.emplace_back(true);
            indices1.erase(indices1.begin());
        } else {
            interlace.emplace_back(false);
            indices2.erase(indices2.begin());
        }
    }

    std::shared_ptr<QubitState> newQubitState = std::make_shared<QubitState>(
            QubitState(qubitState1->getNQubits() + qubitState2->getNQubits()));
    newQubitState->clear();

    //Iterate over qubitState entries
    for (const auto [key1, value1]: *qubitState1) {
        for (auto [key2, value2]: *qubitState2) {
            //Calulate new key
            BitSet newKey = BitSet(qubitState1->getNQubits() + qubitState2->getNQubits(), 0);
            int nextBitNew = 0;
            int nextBit1 = 0;
            int nextBit2 = 0;
            for (bool next1: interlace) {
                if (next1) {
                    newKey |= ((key1 & (1 << nextBit1)) >> nextBit1) << nextBitNew;
                    nextBit1++;
                } else {
                    newKey |= ((key2 & (1 << nextBit2)) >> nextBit2) << nextBitNew;
                    nextBit2++;
                }
                nextBitNew++;
            }

            //Insert new value
            (*newQubitState)[newKey] = value1 * value2;
        }
    }

    return newQubitState;
}

void QubitState::applyGate(size_t target, Complex matrix[4]) {
    std::map newMap = std::map<BitSet, Complex>();

    for (auto const &[key, val]: this->map) {
        if (val.isZero())
            continue;

        /*
         * Matrix = | a b | = [a b c d]
         *          | c d |
         */
        if ((key & (1 << target)) == 0) {
            //Qubit is 0
            if (!matrix[0].isZero()) {
                newMap[key] += val * matrix[0];
            }
            if (!matrix[2].isZero()) {
                newMap[BitSet(this->nQubits, key | (1 << target))] += val * matrix[2];
            }
        } else {
            //Qubit is 1
            if (!matrix[1].isZero()) {
                newMap[key & ~(1 << target)] += val * matrix[1];
            }
            if (!matrix[3].isZero()) {
                newMap[key] += val * matrix[3];
            }
        }
    }

    this->map = newMap;
    this->removeZeroEntries();
}

void QubitState::removeZeroEntries() {
    auto it = this->map.begin();
    while (it != this->map.end()) {
        if (it->second.isZero()) {
            it = this->map.erase(it);
        } else {
            it++;
        }
    }
}

bool QubitState::operator==(const QubitState &rhs) const {
    return map == rhs.map;
}

bool QubitState::canActivate(size_t index) const {
    BitSet mask(1 << index);
    for (auto const &[key, val]: this->map) {
        if ((key & mask) != 0) {
            return true;
        }
    }

    return false;
}

bool QubitState::canActivate(const std::vector<size_t> &indices) const {
    BitSet mask(0);
    for (int index: indices) {
        mask |= 1 << index;
    }

    for (auto const &[key, val]: this->map) {
        if ((key & mask) == mask) {
            return true;
        }
    }

    return false;
}

QubitState QubitState::clone() const {
    auto clone = QubitState(QubitState(this->nQubits));
    clone.map = this->map;
    return clone;
}

void QubitState::applyGate(size_t target, std::vector<size_t> controls, Complex *matrix) {
    if (controls.empty()) {
        this->applyGate(target, matrix);
        return;
    }


    BitSet mask(0);
    for (int index: controls) {
        mask |= 1 << index;
    }

    Complex activatedAmplitude = 0;
    Complex notActivatedAmplitude = 0;

    for (auto const &[key, val]: this->map) {
        if (mask == (key & mask)) {
            activatedAmplitude += val;
        } else {
            notActivatedAmplitude += val;
        }
    }

    if (activatedAmplitude == 0) {
        return;
    } else if (notActivatedAmplitude == 0) {
        this->applyGate(target, matrix);
    }

    QubitState notActivatedQubitState = this->clone();
    QubitState activatedQubitState = this->clone();
    activatedQubitState.applyGate(target, matrix);

    //Combine States
    this->map.clear();
    for (auto const &[key, val]: notActivatedQubitState.map) {
        //TODO: Is this right? Probably not
        this->map[key] = val / notActivatedAmplitude + activatedQubitState[key] / activatedAmplitude;
    }
}
