//
// Created by Jakob on 31/01/2023.
//

#ifndef QCPROP_UNIONTABLE_HPP
#define QCPROP_UNIONTABLE_HPP

#include <memory>
#include <variant>
#include <vector>
#include "QubitState.hpp"
#include "Definitions.hpp"
#include "ActivationState.hpp"

class UnionTable {
public:
    explicit UnionTable(size_t _nQubits);

    ~UnionTable();

    void combine(size_t qubit1, size_t qubit2);

    void combine(size_t qubit1, std::vector<size_t> otherQubits);

    void combine(std::vector<size_t> qubits);

    [[maybe_unused]] void print(std::ostream &os) const;

    QubitStateOrTop &operator[](size_t index) {
        return this->quReg[index];
    }

    bool operator==(const UnionTable &other) const;

    [[nodiscard]] size_t size() const {
        return this->nQubits;
    }

    [[nodiscard]] std::string to_string() const;

    std::pair<ActivationState, std::vector<size_t>> minimizeControls(std::vector<size_t> controls);

    void swap(size_t q1, size_t q2);

    [[nodiscard]] size_t indexInState(size_t qubit) const;

    [[nodiscard]] std::vector<size_t> indexInState(const std::vector<size_t> &qubits) const;

    [[nodiscard]] bool isTop(size_t index) const;

    void setTop(size_t qubit);

    //Define methods to iterate over table
    QubitStateOrTop *begin() {
        return this->quReg;
    }

    QubitStateOrTop *end() {
        return this->quReg + this->nQubits;
    }

    [[nodiscard]] std::shared_ptr<UnionTable> clone() const;

    [[nodiscard]] size_t getNQubits() const {
        return nQubits;
    }

    [[nodiscard]] bool allTop();

private:
    size_t nQubits;
    QubitStateOrTop *quReg;
    bool allTopState = false;

    bool isAlwaysOne(size_t q);

    bool isAlwaysZero(size_t q);
};


#endif //QCPROP_UNIONTABLE_HPP
