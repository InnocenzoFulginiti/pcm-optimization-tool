//
// Created by Jakob on 31/01/2023.
//

#include <bitset>

#include "../include/QubitState.hpp"
#include "../include/util/Complex.hpp"

QubitState::QubitState(int nQubits) {
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
    for (auto const& [key, val] : this->map) {
        if (!key[qubit]) {
            count++;
        }
    }
    return count;
}

int QubitState::countQubitIisOne(int qubit) const {
    int count = 0;
    for (auto const& [key, val] : this->map) {
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
    std::string str = "";
    for (auto const& [key, val] : this->map) {
        str +=  "|" + key.to_string() + "> -> " + val.to_string() + ", ";
    }

    return str;
}
