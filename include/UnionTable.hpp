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

    void print(std::ostream &os) const;

    QubitStateOrTop &operator[](size_t index) {
        return this->quReg[index];
    }

    [[nodiscard]] size_t size() const {
        return this->nQubits;
    }

    [[nodiscard]] std::string to_string() const;

    void setTable(QubitStateOrTop *newTable) {
        this->quReg = newTable;
    }

    QubitStateOrTop *getTable() {
        return this->quReg;
    }

    std::pair<ActivationState, std::vector<size_t>> minimizeControls(std::vector<size_t> controls);

    void swap(size_t q1, size_t q2);

    [[nodiscard]] size_t indexInState(size_t qubit) const;

    [[nodiscard]] std::vector<size_t> indexInState(const std::vector<size_t>& qubit) const;

    [[nodiscard]] bool canActivate(size_t qubit) const;

    [[nodiscard]] bool canActivate(std::vector<size_t> controls) const;

    [[nodiscard]] bool isTop(size_t index) const;

    std::pair<size_t, size_t> countActivations(std::vector<size_t> controls);

    void setTop(size_t qubit);

    //Define methods to iterate over table
    QubitStateOrTop *begin() {
        return this->quReg;
    }

    QubitStateOrTop *end() {
        return this->quReg + this->nQubits;
    }

    [[nodiscard]] std::shared_ptr<UnionTable> clone() const;

private:
    size_t nQubits;
    QubitStateOrTop *quReg;

    [[nodiscard]] std::vector<size_t> qubitsInSameState(size_t qubit) const;

    bool anyIsTop(std::vector<size_t> indices);

    bool isAlwaysOne(size_t q);

    bool isAlwaysZero(size_t q);
};


#endif //QCPROP_UNIONTABLE_HPP
