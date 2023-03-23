#ifndef QCPROP_BITSET_HPP
#define QCPROP_BITSET_HPP

#include <iostream>
#include <vector>

class BitSet {
public:
    BitSet() : BitSet(0) {};

    BitSet(int value);

    BitSet(unsigned int value);

    BitSet(size_t value);

    BitSet(size_t _size, size_t _value);

    BitSet(size_t _size, const BitSet &_copy);

    BitSet(size_t _size, const std::vector<bool> &_copy);

    ~BitSet() = default;

    void setSize(size_t newSize) {
        this->size = newSize;
    }

    [[nodiscard]] size_t getSize() const {
        return size;
    }

    bool operator[](const int index) const {
        return bits[static_cast<size_t>(index)];
    }

    bool operator[](const size_t index) const {
        return bits[index];
    }

    bool operator<(const BitSet &other) const;

    bool operator==(const BitSet &other) const;

    bool operator!=(const BitSet &other) const;

    BitSet operator&(const BitSet &other) const;

    BitSet operator|(const BitSet &other) const;

    BitSet operator^(const BitSet &other) const;

    BitSet &operator&=(const BitSet &other);

    BitSet &operator|=(const BitSet &other);

    BitSet operator>>(const size_t shift) const;

    BitSet operator<<(const size_t shift) const;

    BitSet &operator<<=(const size_t shift);

    BitSet &operator>>=(const size_t shift);

    BitSet operator~() const;

    BitSet operator-(const BitSet &other) const;

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
        return "";
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
