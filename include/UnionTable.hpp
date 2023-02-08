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

#define MAX_AMPLITUDES 10

using QubitStateOrTop = std::variant<TOP, std::shared_ptr<QubitState>>;

class UnionTable {
public:
    explicit UnionTable(size_t nQubits);

    ~UnionTable();

    void combine(size_t qubit1, size_t qubit2);

    void combine(size_t qubit1, std::vector<size_t> otherQubits);

    void combine(std::vector<size_t> qubits);

    void print(std::ostream &os) const;

    QubitStateOrTop &operator[](size_t index) {
        return this->quReg[index];
    }

    size_t size() const {
        return this->nQubits;
    }

    [[nodiscard]] std::string to_string() const;

    void setTable(QubitStateOrTop *newTable) {
        this->quReg = newTable;
    }

    QubitStateOrTop *getTable() {
        return this->quReg;
    }

    [[nodiscard]] size_t indexInState(size_t qubit) const;

    [[nodiscard]] bool canActivate(size_t qubit) const;

    [[nodiscard]] bool canActivate(std::vector<size_t> controls) const;

    [[nodiscard]] bool isTop(size_t index) const;

    std::pair<size_t, size_t> countActivations(std::vector<size_t> controls);

private:
    size_t nQubits;
    QubitStateOrTop *quReg;

    [[nodiscard]] std::vector<size_t> qubitsInSameState(size_t qubit) const;

    bool anyIsTop(std::vector<size_t> indices);
};


#endif //QCPROP_UNIONTABLE_HPP
