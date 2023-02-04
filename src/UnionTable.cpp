//
// Created by Jakob on 31/01/2023.
//

#include <memory>
#include <iostream>
#include "../include/UnionTable.hpp"

UnionTable::UnionTable(size_t nQubits) {
    this->nQubits = nQubits;
    this->quReg = new QubitStateOrTop [nQubits];
    for(int i = 0; i < (int) nQubits; i++) {
        this->quReg[i] = std::make_shared<QubitState>(1);
    }
}

UnionTable::~UnionTable() {
    delete[] this->quReg;
}

//Combine UnionTable entries of two qubits. If either is Top, the result is Top.
//If both are QubitStates, the result is the tensor product of the two.
void UnionTable::combine(size_t qubit1, size_t qubit2) {
    if (std::holds_alternative<TOP>(this->quReg[qubit1])) {
        this->quReg[qubit2] = TOP::T;
        return;
    }
    
    if (std::holds_alternative<TOP>(this->quReg[qubit2])) {
        this->quReg[qubit1] = TOP::T;
        return;
    }

    std::shared_ptr<QubitState> qubitState1 = std::get<std::shared_ptr<QubitState>>(this->quReg[qubit1]);
    std::shared_ptr<QubitState> qubitState2 = std::get<std::shared_ptr<QubitState>>(this->quReg[qubit2]);

    if(qubitState1 == qubitState2) {
        std::cerr << "Error: Qubit " << qubit1 << " and Qubit " << qubit2 << " are already combined." << std::endl;
        return;
    }

    //Find Indices of qubitStates in table
    std::vector<int> qubitState1Indices{};
    std::vector<int> qubitState2Indices{};

    for (int i = 0; i < (int) (this->nQubits) ; ++i) {
        if(std::get<std::shared_ptr<QubitState>>(this->quReg[i]) == qubitState1) {
            qubitState1Indices.emplace_back(i);
        }
        if(std::get<std::shared_ptr<QubitState>>(this->quReg[i]) == qubitState2) {
            qubitState2Indices.emplace_back(i);
        }
    }

   std::shared_ptr<QubitState> newQubitState = QubitState::combine(qubitState1, qubitState1Indices,
                                                                   qubitState2, qubitState2Indices);

    //Replace old qubitStates with new one
    for (int i = 0; i < (int) nQubits; i++) {
        if(std::get<std::shared_ptr<QubitState>>(this->quReg[i]) == qubitState1
            || std::get<std::shared_ptr<QubitState>>(this->quReg[i]) == qubitState2) {
            this->quReg[i] = newQubitState;
        }
    }
}

void UnionTable::print(std::ostream &os) const {
    os << this->to_string() << std::endl;
}

std::string UnionTable::to_string() const {
    std::stringstream os;
    for(int i = 0; i < (int) nQubits; i++) {
        os << i << ": -> ";
        if (std::holds_alternative<TOP>(this->quReg[i])) {
            os << "Top" << std::endl;
        } else {
            std::shared_ptr<QubitState> qubitState = std::get<std::shared_ptr<QubitState>>(this->quReg[i]);
            os << std::hex << qubitState << std::dec << ": ";
            qubitState->print(os);
            os << std::endl;
        }
    }
    return os.str();
}
