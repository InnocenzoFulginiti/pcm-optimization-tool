//
// Created by Jakob on 31/01/2023.
//

#ifndef QCPROP_UNIONTABLE_HPP
#define QCPROP_UNIONTABLE_HPP

#include <memory>
#include <variant>
#include <vector>
#include "QubitState.hpp"

#define MAX_AMPLITUDES 10

//Define ENUM TOP
enum TOP: char {
    T
};

using QubitStateOrTop           = std::variant<TOP, std::shared_ptr<QubitState>>;

class UnionTable {
public:
    explicit UnionTable(size_t nQubits);
    ~UnionTable();

    void combine(size_t qubit1, size_t qubit2);

    void print(std::ostream& os) const;

    size_t size() const {
        return this->nQubits;
    }

    [[nodiscard]] std::string to_string() const;

    void setTable(QubitStateOrTop* newTable) {
        this->table = newTable;
    }

private:
    size_t nQubits;
    QubitStateOrTop* table;
};



#endif //QCPROP_UNIONTABLE_HPP
