#ifndef QCPROP_BITSET_HPP
#define QCPROP_BITSET_HPP

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

class BitSet {
public:
    BitSet(size_t _size, size_t _value);

    BitSet(size_t _size, const BitSet &_copy);

    BitSet(size_t _size, const std::vector<bool> &_copy);

    /**
     *
     * @param size - Size of the BitSet
     * @param value - Value to set bits with index [0, numberOfSetValues[ to
     * @param numberOfSetValues - Number of bits to set to value
     */
    BitSet(size_t size, bool value, size_t numberOfSetValues);

    ~BitSet() = default;

    void setSize(size_t newSize);

    [[nodiscard]] size_t getSize() const {
        return size;
    }

    bool operator[](const int index) const {
        return bits[static_cast<size_t>(index)];
    }

    bool operator[](const size_t index) const {
        if (index >= bits.size()) {
            if (index >= this->size)
                throw std::out_of_range("Index out of range");
            else
                return false;
        }
        return bits[index];
    }

    std::_Bit_reference operator[](const size_t index) {
        if (index >= this->bits.size())
            throw std::out_of_range("Index out of range");

        return bits[index];
    }

    bool operator>(const BitSet &other) const;

    bool operator<(const BitSet &other) const;

    bool operator==(const BitSet &other) const;

    bool operator!=(const BitSet &other) const;

    BitSet operator&(const BitSet &other) const;

    BitSet operator|(const BitSet &other) const;

    BitSet operator^(const BitSet &other) const;

    BitSet &operator&=(const BitSet &other);

    BitSet &operator|=(const BitSet &other);

    BitSet operator>>(size_t shift) const;

    BitSet operator<<(size_t shift) const;

    BitSet &operator<<=(size_t shift);

    BitSet &operator>>=(size_t shift);

    bool allTrue(std::vector<size_t> indices) const;

    BitSet operator~() const;

    friend std::ostream &operator<<(std::ostream &os, const BitSet &bitSet);

//Write method to display value in debugger
    void print(std::ostream &os) const {
        os << *this;
    }

    [[nodiscard]] std::vector<bool> getBits() const {
        return bits;
    }

    //Write to_string method
    [[nodiscard]] std::string to_string() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

private:
    size_t size;
    std::vector<bool> bits;
};

namespace std {
    template<>
    struct hash<BitSet> {
        auto operator()(const BitSet &bs) const -> size_t {
            return hash<std::vector<bool>>()(bs.getBits());
        }
    };
}  // namespace std

#endif //QCPROP_BITSET_HPP
