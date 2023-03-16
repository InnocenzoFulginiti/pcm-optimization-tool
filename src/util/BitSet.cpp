//
// Created by Jakob on 01/02/2023.
//

#include "../../include/util/BitSet.hpp"

std::ostream &operator<<(std::ostream &os, const BitSet &bitSet) {
    for (size_t i = bitSet.size - 1;; i--) {
        os << bitSet.bits[i];
        if (i == 0) break;
    }

    return os;
}

BitSet::BitSet(int value) {
    if (value < 0)
        throw std::invalid_argument("BitSet value must be positive");
    this->size = MAX_QUBITS;
    this->bits = std::bitset<MAX_QUBITS>(static_cast<unsigned long long int>(value));
}

BitSet BitSet::operator-(const BitSet &other) const {
    if (this->size < other.size) {
        throw std::runtime_error("Minuend of subtraction must have larger size than subtrahend");
    }

    bool carry = false;
    BitSet ret(this->size, 0);

    for (size_t i = 0; i < this->size; i++) {
        bool a = (*this)[i];
        bool b = other[i];
        bool r = a ^ b ^ carry;
        carry = (b && carry) || (!a && (carry ^ b));
        ret.bits[i] = r;
    }

    return ret;
}
