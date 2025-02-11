//
// Created by Jakob on 31/01/2023.
//

#include <bitset>

#include "../include/QubitState.hpp"

QubitState::QubitState(size_t _nQubits) {
    this->nQubits = _nQubits;
    this->map = std::unordered_map<BitSet, Complex>();

    map[BitSet(_nQubits, 0)] = Complex(1, 0);
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



double QubitState::probabilityMeasureZero(size_t index) const {
    return probabilityMeasureX(index, false);
}

double QubitState::probabilityMeasureOne(size_t index) const {
    return probabilityMeasureX(index, true);
}

double QubitState::probabilityMeasureX(size_t index, bool x) const {
    double prob = 0.0;
    bool isAlwaysX = true;

    for (auto const &[key, val]: this->map) {
        if (key[index] == x) {
            prob += val.norm();
        }
        else {
            isAlwaysX = false;
        }
    }
    
    if (isAlwaysX) {
        return 1.0;
    }

    return prob;
}

std::pair<std::complex<double>, std::complex<double>> QubitState::amplitudes(size_t index) const {
    std::complex<double> alpha;
    std::complex<double> beta;
    if (this->getNQubits() == 1) {
        for (auto const &[key, val]: this->map) {
            if (key[0] == false) {
                alpha = std::complex<double>(val.real(), val.imag());
            }
            else {
                beta = std::complex<double>(val.real(), val.imag());
            }
        }

        return std::make_pair(alpha, beta);
    }
    else {
        // Density matrix representign the quantum state of the index-th qubit
        std::complex<double> density_mat_00(0., 0.);
        std::complex<double> density_mat_01(0., 0.);
        std::complex<double> density_mat_10(0., 0.);
        std::complex<double> density_mat_11(0., 0.);

        // Build the density matrix
        for (auto const &[key_1, val_1]: this->map) {
            for (auto const  &[key_2, val_2]: this->map) {
                std::string key_1_string = key_1.to_string();
                std::string key_2_string = key_2.to_string();

                int new_index = key_1_string.size() - index - 1;

                char key_1_bit = key_1_string.at(new_index);
                char key_2_bit = key_2_string.at(new_index);

                key_1_string.erase(new_index, 1);
                key_2_string.erase(new_index, 1);

                std::complex<double> val_2_conj(val_2.real(), - val_2.imag());
                std::complex<double> val_1_tmp(val_1.real(), val_1.imag());

                if (key_1_string == key_2_string) {
                    if (key_1_bit == '0' && key_2_bit == '0')
                        density_mat_00 += val_1_tmp * val_2_conj;
                    if (key_1_bit == '0' && key_2_bit == '1')
                        density_mat_01 += val_1_tmp * val_2_conj;
                    if (key_1_bit == '1' && key_2_bit == '0')
                        density_mat_10 += val_1_tmp * val_2_conj;
                    if (key_1_bit == '1' && key_2_bit == '1')
                        density_mat_11 += val_1_tmp * val_2_conj;
                }
            }
        }

        Eigen::Matrix2cd density_matrix;
        density_matrix(0, 0) = density_mat_00;
        density_matrix(0, 1) = density_mat_01;
        density_matrix(1, 0) = density_mat_10;
        density_matrix(1, 1) = density_mat_11;

        Eigen::SelfAdjointEigenSolver<Eigen::Matrix2cd> es(density_matrix);

        Eigen::Vector2cd eigenvalues = es.eigenvalues();
        Eigen::Matrix2cd eigenvectors = es.eigenvectors();
        
        int index_quantum_state;
        
        for (int i = 0; i < eigenvalues.size(); ++i) {
            if (std::abs(eigenvalues[i] - 1.0) < 1e-5) {
                index_quantum_state = i;
                break;
            }
        }
        
        std::pair<std::complex<double>, std::complex<double>> state_vector_pair(
                    eigenvectors(0, index_quantum_state),
                    eigenvectors(1, index_quantum_state)
                );
        
        return state_vector_pair;
    }
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
        BitSet newKey = key;

        newKey[q1] = key[q2];
        newKey[q2] = key[q1];

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

    this->removeZeroEntries();
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

        BitSet newKey = leftUnchanged | middle | rightUnchanged;
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
            size_t nextBitNew = 0;
            size_t nextBit1 = 0;
            size_t nextBit2 = 0;
            for (bool nextIsFrom1: interlace) {
                if (nextIsFrom1) {
                    newKey[nextBitNew++] = key1[nextBit1++];
                } else {
                    //use next bit from 2 first
                    newKey[nextBitNew++] = key2[nextBit2++];
                }
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

    double removed = 0.0;

    while (it != this->map.end()) {
        if (it->second.isZero()) {
            removed += it->second.norm();
            it = this->map.erase(it);
        } else {
            it++;
        }
    }

    (*this) *= 1 / sqrt(1 - removed);
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

    // Split amplitudes into activated and deactivated
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

size_t vectorBoolToSizeT(const std::vector<bool>& vec) {
    size_t value = 0;
    size_t size = vec.size();

    for (size_t i = 0; i < size; ++i) {
        if (vec[i]) {
            value |= (static_cast<size_t>(1) << i);
        }
    }
    return value;
}

std::vector<Complex> QubitState::toStateVector() {
    // Creates a vector of size 2^nQubits elements
    std::vector<Complex> state_vector(1 << this->getNQubits(), Complex(0.0, 0.0));

    // It creates the state vector
    for (auto &[key, value]: this->map) {
        state_vector[vectorBoolToSizeT(key.getBits())] = value;
    }

    return state_vector;
}

std::unordered_map<BitSet, Complex> QubitState::getQuantumState() {
    std::unordered_map<BitSet, Complex> state;

    for (auto &[key, value]: this->map) {
        state[key] = value;
    }

    return state;
}