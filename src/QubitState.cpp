//
// Created by Jakob on 31/01/2023.
//

#include <bitset>

#include "../include/QubitState.hpp"

QubitState::QubitState(size_t _nQubits) {
    this->nQubits = _nQubits;
    this->map = std::unordered_map<BitSet, Complex>();

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
    std::map ordered = std::map<BitSet, Complex>(this->map.begin(), this->map.end());
    for (auto const &[key, val]: ordered) {
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
    this->operator*=(1 / norm);
}

void QubitState::swapIndex(size_t q1, size_t q2) {
    if (q1 >= this->nQubits) {
        std::string msg = "Index q1 = " + std::to_string(q1) + " is out of bounds for QubitState with " +
                          std::to_string(this->nQubits) + " qubits";
        throw std::out_of_range(msg);
    }

    if (q2 >= this->nQubits) {
        std::string msg = "Index q2 = " + std::to_string(q2) + " is out of bounds for QubitState with " +
                          std::to_string(this->nQubits) + " qubits";
        throw std::out_of_range(msg);
    }

    std::unordered_map<BitSet, Complex> newMap = std::unordered_map<BitSet, Complex>();
    for (auto [key, value]: this->map) {
        BitSet k1 = key & (BitSet(this->getNQubits(), 1) << q1);
        BitSet k2 = key & (BitSet(this->getNQubits(), 1) << q2);
        BitSet newKey = key & ~((BitSet(this->getNQubits(), 1) << q1) | (BitSet(this->getNQubits(), 1) << q2));
        if (q1 >= q2) {
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

[[maybe_unused]] void QubitState::removeBit(size_t q) {
    std::unordered_map<BitSet, Complex> newMap{};
    this->nQubits--;

    for (auto [key, value]: this->map) {
        BitSet newKey = key & (BitSet(this->getNQubits() - 1, true, q)); //Keep bits right of q
        newKey |=
                (key & (~BitSet(this->getNQubits() - 1, true, q + 1))) >> 1; //Keep bits left of q and shift them right
        newKey.setSize(nQubits);
        newMap[newKey] += value;
    }

    removeZeroEntries();
}

void QubitState::reorderIndex(size_t oldI, size_t newI) {
    if (oldI == newI) {
        return;
    }

    if (oldI >= this->nQubits) {
        std::string msg = "Index oldI = " + std::to_string(oldI) + " is out of bounds for QubitState with " +
                          std::to_string(this->nQubits) + " qubits";
        throw std::out_of_range(msg);
    }

    if (newI >= this->nQubits) {
        std::string msg = "Index newI = " + std::to_string(newI) + " is out of bounds for QubitState with " +
                          std::to_string(this->nQubits) + " qubits";
        throw std::out_of_range(msg);
    }

    size_t nq = this->getNQubits();

    //Left being MSB
    size_t left = oldI > newI ? oldI : newI;
    size_t right = oldI < newI ? oldI : newI;

    std::unordered_map<BitSet, Complex> newMap = std::unordered_map<BitSet, Complex>();
    for (auto [key, value]: this->map) {
        //Calculate new key
        BitSet leftUnchanged = key & ~BitSet(nq, true, left + 1);
        BitSet rightUnchanged = key & BitSet(nq, true, right);
        bool movingBit = key[oldI];

        BitSet middle(0, 0);

        if (oldI >= newI) {
            //Move middle to the left
            middle = key & (BitSet(nq, true, left) & ~BitSet(nq, true, right));
            middle <<= 1;
        } else {
            //Move middle to the right
            middle = key & (BitSet(nq, true, left + 1) & ~BitSet(nq, true, right + 1));
            middle >>= 1;
        }

        BitSet newKey(key.getSize(), leftUnchanged | middle | rightUnchanged);
        newKey[newI] = movingBit;
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
    for (const auto &[key1, value1]: *qubitState1) {
        for (const auto &[key2, value2]: *qubitState2) {
            //Calculate new key
            BitSet newKey = BitSet(qubitState1->getNQubits() + qubitState2->getNQubits(), 0);
            size_t nq = newQubitState->getNQubits();
            size_t nextBitNew = 0;
            size_t nextBit1 = 0;
            size_t nextBit2 = 0;
            for (bool nextIsFrom1: interlace) {
                if (nextIsFrom1) {
                    BitSet nextBit = (key1 & (BitSet(key1.getSize(), 1) << nextBit1)) >> nextBit1;
                    nextBit.setSize(nq);
                    newKey |= nextBit << nextBitNew;
                    nextBit1++;
                } else {
                    //use next bit from 2 first
                    BitSet nextBit = (key2 & (BitSet(key2.getSize(), 1) << nextBit2)) >> nextBit2;
                    nextBit.setSize(nq);
                    newKey |= nextBit << nextBitNew;
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
        size_t nq = this->getNQubits();
        if (key[target]) {
            //Qubit is 1
            if (!matrix[1].isZero()) {
                BitSet newKey(nq, key);
                newKey[target] = false;
                newMap[newKey] += val * matrix[1];
            }
            if (!matrix[3].isZero()) {
                newMap[key] += val * matrix[3];
            }
        } else {
            //Qubit is 0
            if (!matrix[0].isZero()) {
                newMap[key] += val * matrix[0];
            }
            if (!matrix[2].isZero()) {
                BitSet newKey(nq, key);
                newKey[target] = true;
                newMap[newKey] += val * matrix[2];
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
            double scale = it->second.norm();
            it = this->map.erase(it);
            (*this) *= 1 / sqrt(1 - scale);
        } else {
            it++;
        }
    }
}

bool QubitState::operator==(const QubitState &rhs) const {
    if (this->size() != rhs.size())
        return false;

    return std::all_of(this->map.begin(), this->map.end(), [&](const std::pair<const BitSet, Complex> &p) {
        auto [key, val] = p;
        return (rhs.map.find(p.first) != rhs.map.end())
               && (val != rhs.map.at(key));
    });
}

bool QubitState::neverActivated(const std::vector<size_t> &indices) const {
    //None of the keys are active
    return std::none_of(this->map.begin(), this->map.end(),
                        [&](const std::pair<const BitSet, Complex> p) {
                            //This key activates - for all indices is true
                            return std::all_of(indices.begin(), indices.end(),
                                               [&](const size_t index) { return p.first[index]; });
                        });
}

bool QubitState::alwaysActivated(const std::vector<size_t> &indices) const {
    //All keys activate
    return std::all_of(this->map.begin(), this->map.end(),
                       [&](const std::pair<const BitSet, Complex> p) {
                           //This key activates - for all indices is true
                           return std::all_of(indices.begin(), indices.end(),
                                              [&](const size_t index) { return p.first[index]; });
                       });
}

std::shared_ptr<QubitState> QubitState::clone() const {
    auto clone = std::make_shared<QubitState>(this->nQubits);
    clone->clear();
    //Copy map
    for (auto const &[key, val]: this->map) {
        clone->map[key] = val;
    }

    return clone;
}

void
QubitState::applyGate(const size_t target, const std::vector<size_t> &controls, const std::array<Complex, 4> matrix) {
    if (controls.empty()) {
        this->applyGate(target, matrix);
        return;
    }

    //Split amplitudes into activated and deactivated
    QubitState activated(this->nQubits);
    activated.clear();
    QubitState deactivated(this->nQubits);
    deactivated.clear();

    for (auto const &[key, value]: this->map) {
        if (key.allTrue(controls)) {
            activated[key] = value;
        } else {
            deactivated[key] = value;
        }
    }

    //Apply gate to activated amplitudes
    activated.applyGate(target, matrix);

    //Merge activated and deactivated amplitudes
    this->map = activated.map;
    for (auto const &[key, value]: deactivated.map) {
        this->map[key] += value;
    }

    this->removeZeroEntries();
}

void QubitState::applyTwoQubitGate(size_t t1, size_t t2, const std::vector<size_t> &controls,
                                   std::array<std::array<Complex, 4>, 4> mat) {
    if (controls.empty()) {
        this->applyTwoQubitGate(t1, t2, mat);
        return;
    }

    //Split amplitudes into activated and deactivated
    QubitState activated(this->nQubits);
    activated.clear();
    QubitState deactivated(this->nQubits);
    deactivated.clear();

    for (auto const &[key, value]: this->map) {
        if (key.allTrue(controls)) {
            activated[key] = value;
        } else {
            deactivated[key] = value;
        }
    }

    activated.applyTwoQubitGate(t1, t2, mat);

    this->map = activated.map;
    for (auto const &[key, value]: deactivated.map) {
        this->map[key] += value;
    }

    this->removeZeroEntries();
}

void QubitState::applyTwoQubitGate(size_t t1, size_t t2,
                                   std::array<std::array<Complex, 4>, 4> mat) {
    std::unordered_map<BitSet, Complex> newMap{};
    size_t nq = this->nQubits;

    for (auto &[key, value]: this->map) {
        bool t1Val = key[t1];
        bool t2Val = key[t2];
        unsigned col = t1Val + static_cast<unsigned>(2) * t2Val;
        BitSet newKey(nq, key);
        for (unsigned row = 0; row < 4; row++) {
            newKey = BitSet(nq, key);
            newKey[t1] = row & 1;
            newKey[t2] = (row & 2) >> 1;
            newMap[newKey] += mat[row][col] * value;
        }
    }

    this->map = newMap;
    this->removeZeroEntries();
}

QubitState &QubitState::operator*=(const Complex &rhs) {
    for (auto &entry: this->map) {
        entry.second *= rhs;
    }

    return *this;
}

QubitState &QubitState::operator+=(const QubitState &rhs) {
    for (auto &[key, value]: this->map) {
        value += rhs[key];
    }

    return *this;
}

std::shared_ptr<QubitState>
QubitState::fromVector(const std::vector<std::pair<size_t, Complex>> &vector, size_t nQubits) {
    auto state = std::make_shared<QubitState>(nQubits);
    state->clear();

    for (auto const &[key, value]: vector) {
        state->map[BitSet(nQubits, key)] = value;
    }

    return state;
}
