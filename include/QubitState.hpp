//
// Created by Jakob on 31/01/2023.
//

#ifndef QCPROP_QUBITSTATE_HPP
#define QCPROP_QUBITSTATE_HPP


#include <map>
#include <complex>
#include <bitset>
#include <iostream>
#include <memory>
#include <variant>

#include "util/Complex.hpp"
#include "util/BitSet.hpp"
#include "../extern/qfr/include/operations/OpType.hpp"

class QubitState {
public:
    explicit QubitState(size_t _nQubits);

    QubitState(const QubitState &qubitState) = default;

    ~QubitState() = default;

    [[nodiscard]] size_t size() const;

    [[nodiscard]] size_t getNQubits() const;

    void clear() {
        this->map.clear();
    }

    std::shared_ptr<QubitState> clone() const;

    void print(std::ostream &os) const;

    //Iterator over map entries
    typedef std::unordered_map<BitSet, Complex>::iterator iterator;

    /**
     * Combine Quantum States by applying the tensor product
     * New keys are generated by sorting the indices in ascending order
     * LSB of the Key is smallest index
     * @param qubitState1
     * @param indices1 - in ascending order
     * @param qubitState2
     * @param indices2 - in ascending order
     * @return
     */
    static std::shared_ptr<QubitState>
    combine(const std::shared_ptr<QubitState> &qubitState1, std::vector<int> indices1,
            const std::shared_ptr<QubitState> &qubitState2, std::vector<int> indices2);

    iterator begin() {
        return this->map.begin();
    }

    iterator end() {
        return this->map.end();
    }

    Complex &operator[](const BitSet &key) {
        return this->map.operator[](key);
    }

    Complex operator[](const BitSet &key) const {
        auto it = this->map.find(key);
        if (it != this->map.end()) {
            return it->second;
        } else {
            return 0;
        }
    }

    //Define operator <<
    friend std::ostream &operator<<(std::ostream &os, const QubitState &qubitState) {
        qubitState.print(os);
        return os;
    }

    //Define operator ==
    bool operator==(const QubitState &rhs) const;

    QubitState &operator*=(const Complex &rhs);

    QubitState &operator+=(const QubitState &rhs);

    //to_string
    [[nodiscard]] std::string to_string() const;

    /**
     * @return Sum of all probabilities. This should always be 1
     */
    [[nodiscard]] double norm() const;

    void normalize();

    [[nodiscard]] bool alwaysActivated(const std::vector<size_t> &indices) const;

    void applyGate(size_t target,
                   std::array<Complex, 4> matrix);

    void applyGate(size_t target,
                   const std::vector<size_t> &controls,
                   std::array<Complex, 4> matrix);

    void applyTwoQubitGate(size_t t1, size_t t2, std::array<std::array<Complex, 4>, 4> mat);

    void applyTwoQubitGate(size_t, size_t, const std::vector<size_t> &, std::array<std::array<Complex, 4>, 4>);

    void reorderIndex(size_t oldI, size_t newI);

    void swapIndex(size_t q1, size_t q2);

    bool neverActivated(const std::vector<size_t> &indices) const;

    void removeBit(size_t q);

    static std::shared_ptr<QubitState>
    fromVector(const std::vector<std::pair<size_t, Complex>> &vector, size_t nQubits);

private:
    size_t nQubits;
    std::unordered_map<BitSet, Complex> map;

    void removeZeroEntries();
};

enum TOP {
    T
};

class QubitStateOrTop {
private:
    std::variant<TOP, std::shared_ptr<QubitState>> variant;
public:
    QubitStateOrTop() : variant(TOP::T) {}

    QubitStateOrTop(TOP top) : variant(top) {}

    QubitStateOrTop(std::shared_ptr<QubitState> qubitState) : variant(qubitState) {}

    QubitStateOrTop(const QubitStateOrTop &qubitStateOrTop) = default;

    QubitStateOrTop &operator=(const QubitStateOrTop &qubitStateOrTop) = default;

    QubitStateOrTop &operator=(const std::shared_ptr<QubitStateOrTop> &qubitState) {
        if (qubitState->isTop()) {
            this->variant = TOP::T;
        } else {
            this->variant = qubitState->getQubitState()->clone();
        }
        return *this;
    }

    QubitStateOrTop &operator=(const std::shared_ptr<QubitState> &qubitState) {
        this->variant = qubitState;
        return *this;
    }

    QubitStateOrTop &operator=(const TOP &t) {
        this->variant = t;
        return *this;
    }

    bool operator==(const QubitStateOrTop &rhs) const {
        if (this->isTop() && rhs.isTop()) {
            return true;
        } else if (this->isTop() || rhs.isTop()) {
            return false;
        } else {
            return (*this->getQubitState()) == (*rhs.getQubitState());
        }
    }

    bool operator!=(const QubitStateOrTop &rhs) const {
        return !(rhs == *this);
    }

    ~QubitStateOrTop() = default;

    [[nodiscard]] bool isTop() const {
        return std::holds_alternative<TOP>(variant);
    }

    [[nodiscard]] bool isQubitState() const {
        return std::holds_alternative<std::shared_ptr<QubitState>>(variant);
    }

    [[nodiscard]] std::shared_ptr<QubitState> getQubitState() const {
        return std::get<std::shared_ptr<QubitState>>(variant);
    }

    [[nodiscard]] std::string to_string() const {
        if (isTop()) {
            return "TOP";
        } else {
            return getQubitState()->to_string();
        }
    }

    friend std::ostream &operator<<(std::ostream &os, const QubitStateOrTop &qubitStateOrTop) {
        os << qubitStateOrTop.to_string();
        return os;
    }
};

#endif //QCPROP_QUBITSTATE_HPP
