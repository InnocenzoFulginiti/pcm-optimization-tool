//
// Created by Jakob on 01/02/2023.
//

#ifndef QCPROP_BITSET_HPP
#define QCPROP_BITSET_HPP


#include <bitset>
#include <iostream>

//TODO: Make this dynamic
//E.g. vector<bool> or dynamic_bitset (boost)
#define MAX_QUBITS 32

class BitSet {
public:
    BitSet() : size(MAX_QUBITS), bits() {}

    BitSet(int value) : size(MAX_QUBITS), bits(value) {}

    BitSet(size_t value) : size(MAX_QUBITS), bits(value) {}

    BitSet(size_t size, size_t value) : size(size), bits(value) {};

    explicit BitSet(std::bitset<MAX_QUBITS> bits) : size(MAX_QUBITS), bits(bits) {}

    BitSet(size_t size, std::bitset<MAX_QUBITS> bits) : size(size), bits(bits) {}

    BitSet(size_t size, BitSet copy) : size(size), bits(copy.bits) {}

    ~BitSet() = default;

    void setSize(size_t newSize) {
        this->size = newSize;
    }

    size_t getSize() const {
        return size;
    }

    bool operator[](const int index) const {
        return bits[index];
    }

    bool operator[](const size_t index) const {
        return bits[index];
    }

    bool operator<(const BitSet &other) const {
        auto str1 = this->bits.to_string();
        auto strOther = other.bits.to_string();
        return str1 < strOther;
    }

    bool operator>(const BitSet &other) const {
        return other < *this;
    }

    bool operator==(const BitSet &other) const {
        for (int i = size - 1; i >= 0; i--) {
            if ((*this)[i] ^ other[i]) return false;
        }
        return true;
    }

    bool operator!=(const BitSet &other) const {
        return !(*this == other);
    }

    BitSet operator&(const BitSet &other) const {
        return {this->size, this->bits & other.bits};
    }

    BitSet operator|(const BitSet &other) const {
        size_t newSize = this->size > other.size ? this->size : other.size;
        return {newSize, this->bits | other.bits};
    }

    BitSet operator^(const BitSet &other) const {
        size_t newSize = this->size > other.size ? this->size : other.size;
        return {newSize, this->bits ^ other.bits};
    }

    BitSet &operator&=(const BitSet &other) {
        this->bits &= other.bits;
        return *this;
    }

    BitSet &operator|=(const BitSet &other) {
        this->size = this->size > other.size ? this->size : other.size;
        this->bits |= other.bits;
        return *this;
    }

    BitSet operator>>(const int shift) const {
        return {this->size, this->bits >> shift};
    }

    BitSet operator<<(const int shift) const {
        return {this->size, this->bits << shift};
    }

    BitSet &operator<<=(const int shift) {
        this->bits <<= shift;
        return *this;
    }

    BitSet &operator>>=(const int shift) {
        this->bits >>= shift;
        return *this;
    }

    friend std::ostream &operator<<(std::ostream &os, const BitSet &bitSet);

//Write method to display value in debugger
    void print(std::ostream &os) const {
        os << *this;
    }

    [[nodiscard]] std::bitset<MAX_QUBITS> getBits() const {
        return bits;
    }

    //Write to_string method
    [[nodiscard]] std::string to_string() const {
        auto bitString = this->bits.to_string();
        //Remove leading zeros
        std::string ret = bitString.substr(MAX_QUBITS - size, size);
        return ret;
    }

private:
    size_t size;
    std::bitset<MAX_QUBITS> bits;
};

namespace std {
    template<>
    struct hash<BitSet> {
        auto operator()(const BitSet &bs) const -> size_t {
            return hash<std::bitset<MAX_QUBITS>>()(bs.getBits());
        }
    };
}  // namespace std

#endif //QCPROP_BITSET_HPP
