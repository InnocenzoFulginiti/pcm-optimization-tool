//
// Created by Jakob on 01/02/2023.
//

#include "../../include/util/BitSet.hpp"

#define BITS_PER_BYTE 8

std::ostream &operator<<(std::ostream &os, const BitSet &bitSet) {
    if (bitSet.size == 0)
        return os;

    for (size_t i = bitSet.size - 1;;) {
        os << bitSet[i];
        if (i == 0)
            break;

        i--;
    }

    return os;
}

BitSet::BitSet(int value) {
    if (value < 0)
        throw std::invalid_argument("BitSet cannot be negative");


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

    this->bits = std::vector<bool>(this->size, false);

    size_t i = 0;
    while (value != 0) {
        (*this)[i++] = (value & 1);
        value >>= 1;
    }
}

BitSet::BitSet(size_t value) {
    this->size = sizeof(size_t) * BITS_PER_BYTE;

    this->bits = std::vector<bool>(this->size, false);
    size_t i = 0;
    while (value != 0) {
        (*this)[i++] = (value & 1);
        value >>= 1;
    }
}

BitSet::BitSet(size_t _size, size_t _value) {
    this->size = _size;

    this->bits = std::vector<bool>(this->size, false);

    size_t i = 0;
    while (_value != 0) {
        this->bits[i++] = (_value & 1);
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

    if (*this < other) {
        throw std::runtime_error("Minuend of subtraction must be larger than subtrahend");
    }

    bool carry = false;
    BitSet ret(this->getSize(), 0);

    for (size_t i = 0; i < this->getSize(); i++) {
        bool a = (*this)[i];
        bool b = false;
        if (i < other.getSize()) {
            b = other[i];
        }
        bool r = a ^ b ^ carry;
        carry = (b && carry) || (!a && (carry ^ b));
        ret[i] = r;
    }

    return ret;
}

bool BitSet::operator<(const BitSet &other) const {
    if (this->getSize() == 0) {
        return !other.isZero();
    }

    if (other.getSize() == 0) {
        return false;
    }

    size_t i = this->getSize() - 1;
    size_t j = other.getSize() - 1;
    while (i > j) {
        if ((*this)[i]) {
            return false;
        }
        i--;
    }

    while (i < j) {
        if (other[j]) {
            return true;
        }
        j--;
    }

    // i == j
    while (true) {
        if ((*this)[i] && !other[j]) {
            return false;
        } else if (!(*this)[i] && other[j]) {
            return true;
        }
        if (i == 0 || j == 0) {
            break;
        }
        i--;
        j--;
    }

    return false;
}

bool BitSet::operator==(const BitSet &other) const {
    if (this->getSize() == 0) {
        return other.isZero();
    }

    if (other.getSize() == 0) {
        return this->isZero();
    }


    size_t i = this->getSize() - 1;
    size_t j = other.getSize() - 1;
    while (i > j) {
        if ((*this)[i]) {
            return false;
        }
        i--;
    }

    while (i < j) {
        if (other[j]) {
            return false;
        }
        j--;
    }

    // i == j
    while (true) {
        if ((*this)[i] != other[j]) {
            return false;
        }

        if (i == 0 || j == 0) {
            break;
        }

        i--;
        j--;
    }

    return true;
}

BitSet BitSet::operator|(const BitSet &other) const {
    if (this->size != other.size) {
        std::cerr << "BitSet::operator|: Sizes of bitsets do not match" << std::endl;
        throw std::runtime_error("BitSet::operator|: Sizes of bitsets do not match");
    }


    size_t newSize = this->size > other.size ? this->size : other.size;

    std::vector<bool> newBits(newSize);

    size_t i = 0;
    while (i < other.getSize() && i < this->getSize()) {
        newBits[i] = (*this)[i] || other[i];
        i++;
    }

    while (i < other.getSize()) {
        newBits[i] = other[i];
        i++;
    }

    while (i < this->getSize()) {
        newBits[i] = (*this)[i];
        i++;
    }

    return {newSize, newBits};
}

bool BitSet::operator!=(const BitSet &other) const {
    return !(*this == other);
}

BitSet BitSet::operator>>(const size_t shift) const {
    if (shift > this->getSize()) {
        return {0, 0};
    }

    std::vector<bool> newBits = std::vector<bool>(this->bits);
    newBits.erase(newBits.begin(), newBits.begin() + static_cast<long long>(shift));

    return {this->size - shift, newBits};
}

BitSet BitSet::operator<<(const size_t shift) const {
    std::vector<bool> newBits = std::vector<bool>(this->bits);
    newBits.insert(newBits.begin(), shift, false);
    return {this->size + shift, newBits};
}

BitSet BitSet::operator&(const BitSet &other) const {
    if (this->getSize() != other.getSize()) {
        throw std::runtime_error("BitSet::operator&: Sizes of bitsets are not equal");
    }

    BitSet ret(this->size, 0);

    for (size_t i = 0; i < this->size; i++) {
        ret[i] = (*this)[i] && other[i];
    }

    return ret;
}

BitSet BitSet::operator^(const BitSet &other) const {
    if (this->size != other.size) {
        std::cerr << "BitSet::operator^: Sizes of bitsets are not equal" << std::endl;
        throw std::runtime_error("BitSet::operator^: Sizes of bitsets are not equal");
    }

    BitSet ret(this->size, 0);

    for (size_t i = 0; i < this->size; i++) {
        ret[i] = (*this)[i] ^ other[i];
    }

    return ret;
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
    BitSet ret(this->size, 0);

    for (size_t i = 0; i < this->size; i++) {
        ret[i] = !(*this)[i];
    }

    return ret;
}

bool BitSet::operator>(const BitSet &other) const {
    return other < *this;
}

bool BitSet::isZero() const {
    for (size_t i = 0; i < getSize(); i++) {
        if ((*this)[i]) {
            return false;
        }
    }

    return true;
}

void BitSet::setSize(size_t newSize) {
    this->bits.resize(newSize, false);
    this->size = newSize;
}
