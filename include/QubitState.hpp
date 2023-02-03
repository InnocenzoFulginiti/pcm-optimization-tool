//
// Created by Jakob on 31/01/2023.
//

#ifndef QCPROP_QUBITSTATE_HPP
#define QCPROP_QUBITSTATE_HPP


#include <map>
#include <complex>
#include <bitset>
#include "util/Complex.hpp"
#include <iostream>
#include "util/BitSet.hpp"


class QubitState {
public:
    explicit QubitState(int nQubits);

    ~QubitState() = default;

    [[nodiscard]] size_t size() const;

    [[nodiscard]] size_t getNQubits() const;

    void clear() {
        this->map.clear();
    }

    void print(std::ostream& os) const;

    //Iterator over map entries
    typedef std::map<BitSet, Complex>::iterator iterator;

    [[nodiscard]] int countQubitIisZero(int qubit) const;
    [[nodiscard]] int countQubitIisOne(int qubit) const;

    iterator begin() {
        return this->map.begin();
    }

    iterator end() {
        return this->map.end();
    }

    //Define operator []
    Complex& operator[](const BitSet& key) {
        return this->map.operator[](key);
    }

    //Define operator <<
    friend std::ostream& operator<<(std::ostream& os, const QubitState& qubitState) {
        qubitState.print(os);
        return os;
    }

    //to_string
    std::string to_string() const;

private:
    size_t nQubits;
    std::map<BitSet, Complex> map;
};


#endif //QCPROP_QUBITSTATE_HPP
