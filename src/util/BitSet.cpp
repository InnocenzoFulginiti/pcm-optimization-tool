//
// Created by Jakob on 01/02/2023.
//

#include "../../include/util/BitSet.hpp"

#define BITS_PER_BYTE 8

std::ostream &operator<<(std::ostream &os, const BitSet &bitSet) {
    for (size_t i = bitSet.size - 1; i != 0; i--) {
        os << bitSet.bits[i];
    }

    return os;
}

BitSet::BitSet(int value) {
    int msb = 1 << (sizeof(int) * BITS_PER_BYTE - 1);

    this->size = sizeof(int);
    this->bits = std::vector<bool>();
    this->bits.reserve(this->size);

    value = value & ~msb;
    while (value > 0) {
        this->bits.emplace_back(value & 1);
        value >>= 1;
    }
}

BitSet::BitSet(size_t _size, const std::vector<bool> &_copy) {
    this->size = _size;
    this->bits = std::vector<bool>(_copy);
}

BitSet::BitSet(unsigned int value) {
    this->size = sizeof(unsigned int) * BITS_PER_BYTE;

    this->bits = std::vector<bool>();
    this->bits.reserve(this->size);
    while (value != 0) {
        this->bits.push_back(value & 1);
        value >>= 1;
    }
}

BitSet::BitSet(size_t value) {
    this->size = sizeof(size_t) * BITS_PER_BYTE;

    this->bits = std::vector<bool>();
    this->bits.reserve(this->size);
    while (value != 0) {
        this->bits.push_back(value & 1);
        value >>= 1;
    }
}

BitSet::BitSet(size_t _size, size_t _value) {
    this->size = _size;

    this->bits = std::vector<bool>();
    this->bits.reserve(this->size);

    while (_value != 0) {
        this->bits.push_back(_value & 1);
        _value >>= 1;
    }
}

BitSet::BitSet(size_t _size, const BitSet &_copy) {
    this->size = _size;
    this->bits = std::vector<bool>(_copy.bits);
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

bool BitSet::operator<(const BitSet &other) const {
    size_t i = this->bits.size();
    size_t j = this->bits.size();
    while (i > 0 && j > 0 && i > j) {
        if (this->bits[i]) {
            return false;
        }
        i--;
    }

    while (i > 0 && j > 0 && i < j) {
        if (other.bits[j]) {
            return true;
        }
        j--;
    }

    // i == j
    while (i > 0 && j > 0) {
        if (this->bits[i] && !other.bits[j]) {
            return false;
        } else if (!this->bits[i] && other.bits[j]) {
            return true;
        }
        i--;
        j--;
    }

    return false;
}

bool BitSet::operator==(const BitSet &other) const {
    size_t i = this->bits.size();
    size_t j = this->bits.size();
    while (i > 0 && j > 0 && i > j) {
        if (this->bits[i]) {
            return false;
        }
        i--;
    }

    while (i > 0 && j > 0 && i < j) {
        if (other.bits[j]) {
            return false;
        }
        j--;
    }

    // i == j
    while (i > 0 && j > 0) {
        if (this->bits[i] != other.bits[j]) {
            return false;
        }
        i--;
        j--;
    }

    return true;
}

BitSet BitSet::operator|(const BitSet &other) const {
    size_t newSize = this->size > other.size ? this->size : other.size;

    std::vector<bool> newBits(newSize);

    size_t i = 0;
    while (i < other.bits.size() && i < this->bits.size()) {
        newBits[i] = this->bits[i] || other.bits[i];
        i++;
    }

    while (i < other.bits.size()) {
        newBits[i] = other.bits[i];
        i++;
    }

    while (i < this->bits.size()) {
        newBits[i] = this->bits[i];
        i++;
    }

    return {newSize, newBits};
}

bool BitSet::operator!=(const BitSet &other) const {
    return !(*this == other);
}

BitSet BitSet::operator>>(const size_t shift) const {
    std::vector<bool> newBits = std::vector<bool>(this->bits.size());

    newBits.erase(newBits.begin(), newBits.begin() + static_cast<long long>(shift));

    return {this->size - shift, newBits};
}

BitSet BitSet::operator<<(const size_t shift) const {
    std::vector<bool> newBits = std::vector<bool>(this->bits);
    newBits.insert(newBits.begin(), shift, false);
    return {this->size + shift, newBits};
}

BitSet BitSet::operator&(const BitSet &other) const {
    return BitSet();
}

BitSet BitSet::operator^(const BitSet &other) const {
    return BitSet();
}

BitSet &BitSet::operator&=(const BitSet &other) {
    *this = *this & other;
    return *this;
}

BitSet &BitSet::operator|=(const BitSet &other) {
    *this = *this | other;
    return *this;
}

BitSet &BitSet::operator<<=(const size_t shift) {
    *this = *this << shift;
    return *this;
}

BitSet &BitSet::operator>>=(const size_t shift) {
    *this = *this >> shift;
    return *this;
}

BitSet BitSet::operator~() const {
    return BitSet();
}


