//
// Created by Jakob on 31/01/2023.
//

#include <bitset>

#include "../include/QubitState.hpp"

QubitState::QubitState(size_t nQubits) {
    this->nQubits = nQubits;
    this->map = std::unordered_map<BitSet, Complex>();
    this->wasMeasured = false;

    map[BitSet(1, 0)] = Complex(1, 0);
}

size_t QubitState::size() const {
    return this->map.size();
}

size_t QubitState::getNQubits() const {
    return this->nQubits;
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

void QubitState::swapIndex(size_t q1, size_t q2) {
    if(q1 >= this->nQubits) {
        std::string msg = "Index q1 = " + std::to_string(q1) + " is out of bounds for QubitState with " + std::to_string(this->nQubits) + " qubits";
        throw std::out_of_range(msg);
    }

    if(q2 >= this->nQubits) {
        std::string msg = "Index q2 = " + std::to_string(q2) + " is out of bounds for QubitState with " + std::to_string(this->nQubits) + " qubits";
        throw std::out_of_range(msg);
    }

    std::unordered_map<BitSet, Complex> newMap = std::unordered_map<BitSet, Complex>();
    for (auto [key, value] : this->map) {
        BitSet k1 = key & (1 << q1);
        BitSet k2 = key & (1 << q2);
        BitSet newKey = key & ~((1 << q1) | (1 << q2));
        if(q1 >= q2) {
            newKey |= k1 >> (q1 - q2);
            newKey |= k2 << (q1 - q2);
        } else {
            newKey |= k1 << (q2 - q1);
            newKey |= k2 >> (q2 - q1);
        }

        newMap[newKey] = value;
    }

    this->map = newMap;
}

void QubitState::reorderIndex(size_t oldI, size_t newI) {
    if(oldI >= this->nQubits) {
        std::string msg = "Index oldI = " + std::to_string(oldI) + " is out of bounds for QubitState with " + std::to_string(this->nQubits) + " qubits";
        throw std::out_of_range(msg);
    }

    if(newI >= this->nQubits) {
        std::string msg = "Index newI = " + std::to_string(newI) + " is out of bounds for QubitState with " + std::to_string(this->nQubits) + " qubits";
        throw std::out_of_range(msg);
    }

    //Left being MSB
    size_t left = oldI > newI ? oldI : newI;
    size_t right = oldI < newI ? oldI : newI;

    std::unordered_map<BitSet, Complex> newMap = std::unordered_map<BitSet, Complex>();
    for (auto [key, value] : this->map) {
        //Calculate new key
        BitSet leftUnchanged = key & ~((1 << (left + 1)) - 1);
        BitSet rightUnchanged = key & ((1 << right) - 1);
        BitSet movingBit = key & (1 << oldI);

        BitSet middle;
        BitSet shiftedMoving;

        if(oldI >= newI) {
            //Move middle to the left
            middle = key & (((1 << left) - 1) & ~((1 << right) - 1));
            middle <<= 1;
            shiftedMoving = movingBit >> (left - right);
        } else {
            //Move middle to the right
            middle = key & (((1 << (left + 1)) - 1) & ~((1 << (right + 1)) - 1));
            middle >>= 1;
            shiftedMoving = movingBit << (left - right);
        }

        BitSet newKey(key.getSize(), leftUnchanged | middle | shiftedMoving | rightUnchanged);
        newMap[newKey] = value;
    }

    this->map = newMap;
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
            //TODO RENAME
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

void QubitState::applyGate(const size_t target, const std::array<Complex, 4> matrix) {
    std::unordered_map newMap = std::unordered_map<BitSet, Complex>();

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

std::pair<size_t, size_t> QubitState::countActivations(std::vector<size_t> indices) {
    size_t zeros = 0;
    size_t ones = 0;
    BitSet mask(0);
    for (size_t index: indices) {
        mask |= 1 << index;
    }

    for (auto const &[key, val]: this->map) {
        if ((key & mask) == mask) {
            ones++;
        } else {
            zeros++;
        }
    }

    return {zeros, ones};
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

bool QubitState::alwaysActivated(const std::vector<size_t> &indices) const {
    BitSet mask(0);
    for (int index: indices) {
        mask |= 1 << index;
    }

    for (auto const &[key, val]: this->map) {
        if ((key & mask) != mask) {
            return false;
        }
    }

    return true;
}

QubitState QubitState::clone() const {
    auto clone = QubitState(QubitState(this->nQubits));
    clone.map = this->map;
    return clone;
}

void
QubitState::applyGate(const size_t target, const std::vector<size_t> &controls, const std::array<Complex, 4> matrix) {
    if (controls.empty()) {
        this->applyGate(target, matrix);
        return;
    }

    BitSet mask(0);
    for (size_t index: controls) {
        mask |= 1 << index;
    }

    //Split amplitudes into activated and deactivated
    QubitState activated(this->nQubits);
    activated.clear();
    QubitState deactivated(this->nQubits);
    deactivated.clear();

    for (auto const [key, value]: this->map) {
        if ((key & mask) == mask) {
            activated[key] = value;
        } else {
            deactivated[key] = value;
        }
    }

    //Apply gate to activated amplitudes
    activated.applyGate(target, matrix);

    //Merge activated and deactivated amplitudes
    this->map = activated.map;
    for (auto const [key, value]: deactivated.map) {
        this->map[key] = value;
    }
}

QubitState &QubitState::operator*=(const Complex &rhs) {
    for (auto &entry: this->map) {
        entry.second *= rhs;
    }

    return *this;
}

QubitState &QubitState::operator+=(const QubitState &rhs) {
    for(auto &[key, value] : this->map) {
        value += rhs[key];
    }

    return *this;
}
