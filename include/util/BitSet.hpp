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
    BitSet(): bits(), size(MAX_QUBITS) {}
    BitSet(int value): bits(value), size(MAX_QUBITS) {}
    BitSet(size_t size, int value): bits(value), size(size) {};
    BitSet(std::bitset<MAX_QUBITS> bits): bits(bits), size(MAX_QUBITS) {}
    BitSet(size_t size, std::bitset<MAX_QUBITS> bits): bits(bits), size(size) {}

    ~BitSet() = default;

    //Set Size
    void setSize(size_t newSize) {
        this->size = newSize;
    }

    //Define operator []
    bool operator[](const int index) const {
        return bits[index];
    }

    //Define operator <
    bool operator<(const BitSet& other) const
    {
        for (int i = size-1; i >= 0; i--) {
            if ((*this)[i] ^ other[i]) return other[i];
        }
        return false;
    }

    bool operator>(const BitSet& other) const
    {
        return other < *this;
    }

    //Define operator ==
    bool operator==(const BitSet& other) const
    {
        for (int i = size-1; i >= 0; i--) {
            if ((*this)[i] ^ other[i]) return false;
        }
        return true;
    }

    //Define operator !=
    bool operator!=(const BitSet& other) const
    {
        return !(*this == other);
    }

    /**
     * Bitwise AND
     * @param other
     * @return
     */
    BitSet operator&(const BitSet& other) const
    {
        return {this-> size, this->bits & other.bits};
    }

    //Define operator |
    BitSet operator|(const BitSet& other) const
    {
        size_t newSize = this->size > other.size ? this->size : other.size;
        return {newSize, this->bits | other.bits};
    }

    //Define operator ^
    BitSet operator^(const BitSet& other) const
    {
        size_t newSize = this->size > other.size ? this->size : other.size;
        return {newSize, this->bits ^ other.bits};
    }

    //Define operator &=
    BitSet& operator&=(const BitSet& other)
    {
        this->bits &= other.bits;
        return *this;
    }

    //Define operator |=
    BitSet& operator|=(const BitSet& other)
    {
        this->size = this->size > other.size ? this->size : other.size;
        this->bits |= other.bits;
        return *this;
    }

    //Define operator >> that is right shift
    BitSet operator>>(const int shift) const
    {
        return {this->size, this->bits >> shift};
    }

    //Define operator << that is left shift
    BitSet operator<<(const int shift) const
    {
        return {this->size, this->bits << shift};
    }

    //Define operator <<
    friend std::ostream& operator<<(std::ostream& os, const BitSet& bitSet);

    //Write method to display value in debugger
    void print(std::ostream& os) const {
        os << *this;
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


#endif //QCPROP_BITSET_HPP
