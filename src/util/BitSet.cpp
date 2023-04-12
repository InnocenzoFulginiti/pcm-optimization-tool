//
// Created by Jakob on 01/02/2023.
//

#include <algorithm>
#include "../../include/util/BitSet.hpp"

std::ostream &operator<<(std::ostream &os, const BitSet &bitSet) {
    if (bitSet.size == 0)
        return os;

    for (size_t i = bitSet.getSize() - 1;;) {
        os << bitSet[i];
        if (i == 0)
            break;

        i--;
    }

    return os;
}

BitSet::BitSet(size_t _size, const std::vector<bool> &_copy) {
    this->size = _size;
    this->bits = std::vector<bool>(_size, false);
    for (size_t i = 0; i < _size; i++) {
        (*this)[i] = _copy[i];
    }
}

BitSet::BitSet(size_t _size, size_t value) {
    this->size = _size;

    this->bits = std::vector<bool>(_size, false);
    size_t i = 0;
    while (value != 0) {
        (*this)[i++] = (value & 1);
        value >>= 1;
    }
}


BitSet::BitSet(size_t _size, bool value, size_t numberOfSetValues) {
    this->size = _size;
    this->bits = std::vector<bool>(_size, false);
    if (value) {
        for (size_t i = 0; i < numberOfSetValues; i++) {
            (*this)[i] = true;
        }
    }
}


BitSet::BitSet(size_t _size, const BitSet &_copy) {
    this->size = _size;
    this->bits = std::vector<bool>(_copy.bits);
    this->setSize(_size);
}

bool BitSet::operator<(const BitSet &other) const {
    if (this->getSize() != other.getSize()) {
        throw std::runtime_error("BitSets must have same size for operator<");
    }

    if (this->getSize() == 0) return other.getSize() == 0;

    for (size_t i = this->getSize() - 1;;) {
        if ((*this)[i] && !other[i]) {
            return false;
        } else if (!(*this)[i] && other[i]) {
            return true;
        }
        if (i == 0) {
            break;
        }
        i--;
    }

    return false;
}

bool BitSet::operator==(const BitSet &other) const {
    if (this->getSize() != other.getSize()) return false;

    size_t i = 0;

    while (i < other.getSize() && i < this->getSize()) {
        if ((*this)[i] != other[i]) return false;
        i++;
    }

    return true;
}

BitSet BitSet::operator|(const BitSet &other) const {
    if (this->getSize() != other.getSize()) {
        throw std::runtime_error("BitSets must have same size for operator|");
    }

    BitSet ret(this->getSize(), 0);

    size_t i = 0;
    while (i < other.getSize() && i < this->getSize()) {
        ret[i] = (*this)[i] || other[i];
        i++;
    }

    return ret;
}

bool BitSet::operator!=(const BitSet &other) const {
    return !(*this == other);
}

BitSet BitSet::operator>>(const size_t shift) const {
    if (shift > this->getSize()) {
        return {this->getSize(), false};
    }

    BitSet ret(this->getSize(), 0);

    for (size_t i = shift; i < this->getSize(); i++) {
        ret[i - shift] = (*this)[i];
    }

    return ret;
}

BitSet BitSet::operator<<(const size_t shift) const {
    std::vector<bool> newBits = std::vector<bool>(this->bits);
    newBits.insert(newBits.begin(), shift, false);

    return {this->size, newBits};
}

BitSet BitSet::operator&(const BitSet &other) const {
    if (this->getSize() != other.getSize()) {
        throw std::runtime_error("BitSets must be of same size for operator&");
    }

    BitSet ret(this->getSize(), 0);

    for (size_t i = 0; i < this->getSize(); i++) {
        ret[i] = (*this)[i] && other[i];
    }

    return ret;
}

BitSet BitSet::operator^(const BitSet &other) const {
    if (this->getSize() != other.getSize()) {
        throw std::runtime_error("BitSets must be of same size for operator^");
    }

    BitSet ret(this->getSize(), 0);

    if (this->getSize() > other.getSize()) {
        ret = BitSet(this->getSize(), 0);
        size_t i = other.getSize();
        while (i < this->getSize()) {
            ret[i] = (*this)[i];
            i++;
        }
    } else {
        ret = BitSet(other.getSize(), 0);
        size_t i = this->getSize();
        while (i < this->getSize()) {
            ret[i] = other[i];
            i++;
        }
    }

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

void BitSet::setSize(size_t newSize) {
    this->bits.resize(newSize, false);
    this->size = newSize;
}

bool BitSet::allTrue(std::vector<size_t> indices) const {
    return std::all_of(indices.begin(), indices.end(), [this](size_t i) { return (*this)[i]; });
}
