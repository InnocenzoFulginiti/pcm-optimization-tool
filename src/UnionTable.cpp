//
// Created by Jakob on 31/01/2023.
//

#include <memory>
#include <iostream>
#include "../include/UnionTable.hpp"

UnionTable::UnionTable(size_t nQubits) {
    this->nQubits = nQubits;
    this->table = new QubitStateOrTop [nQubits];
    for(int i = 0; i < nQubits; i++) {
        this->table[i] = std::make_shared<QubitState>(1);
    }
}

UnionTable::~UnionTable() {
    delete[] this->table;
}

//Combine UnionTable entries of two qubits. If either is Top, the result is Top.
//If both are QubitStates, the result is the tensor product of the two.
void UnionTable::combine(size_t qubit1, size_t qubit2) {
    if (std::holds_alternative<TOP>(this->table[qubit1])) {
        this->table[qubit2] = TOP::T;
        return;
    }
    
    if (std::holds_alternative<TOP>(this->table[qubit2])) {
        this->table[qubit1] = TOP::T;
        return;
    }

    QubitState* qubitState1 = std::get<std::shared_ptr<QubitState>>(this->table[qubit1]).get();
    QubitState* qubitState2 = std::get<std::shared_ptr<QubitState>>(this->table[qubit2]).get();

    if(qubitState1 == qubitState2) {
        std::cerr << "Error: Qubit " << qubit1 << " and Qubit " << qubit2 << " are already combined." << std::endl;
        return;
    }

    //Find Indices of qubitStates in table
    std::vector<int> qubitState1Indices{};
    std::vector<int> qubitState2Indices{};

    for (int i = 0; i < this->nQubits ; ++i) {
        if(std::get<std::shared_ptr<QubitState>>(this->table[i]).get() == qubitState1) {
            qubitState1Indices.emplace_back(i);
        }
        if(std::get<std::shared_ptr<QubitState>>(this->table[i]).get() == qubitState2) {
            qubitState2Indices.emplace_back(i);
        }
    }

    //Find how to interlace indices (so they are sorted again)
    std::vector<bool> interlace{}; //true = qubitState1, false = qubitState2
    while (!qubitState1Indices.empty() || !qubitState2Indices.empty()) {
        //Only one of the two is empty
        if(qubitState1Indices.empty()) {
            interlace.emplace_back(false);
            qubitState2Indices.erase(qubitState2Indices.begin());
            continue;
        }
        if (qubitState2Indices.empty()) {
            interlace.emplace_back(true);
            qubitState1Indices.erase(qubitState1Indices.begin());
            continue;
        }

        //Both are not empty
        if (qubitState1Indices[0] < qubitState2Indices[0]) {
            interlace.emplace_back(true);
            qubitState1Indices.erase(qubitState1Indices.begin());
        } else {
            interlace.emplace_back(false);
            qubitState2Indices.erase(qubitState2Indices.begin());
        }
    }

    std::shared_ptr<QubitState> newQubitState = std::make_shared<QubitState>(QubitState(qubitState1->getNQubits() + qubitState2->getNQubits()));
    newQubitState->clear();

    //Iterate over qubitState entries
    for(auto [key1, value1] : *qubitState1) {
        for(auto [key2, value2] : *qubitState2) {
            //Calulate new key
            BitSet newKey = BitSet(qubitState1->getNQubits() + qubitState2->getNQubits(), 0);
            int nextBitNew = 0;
            int nextBit1 = 0;
            int nextBit2 = 0;
            for(bool next1 : interlace) {
                if(next1) {
                    newKey |= ((key1 & (1 << nextBit1)) >> nextBit1) << nextBitNew;
                    nextBit1++;
                } else {
                    newKey |= ((key2 & (1 << nextBit2)) >> nextBit2) << nextBitNew;
                    nextBit2++;
                }
                nextBitNew++;
            }


            //Insert new value
            (*newQubitState)[newKey] = value1 * value2;
        }
    }

    //Replace old qubitStates with new one
    for (int i = 0; i < nQubits; i++) {
        if(std::get<std::shared_ptr<QubitState>>(this->table[i]).get() == qubitState1
            || std::get<std::shared_ptr<QubitState>>(this->table[i]).get() == qubitState2) {
            this->table[i] = newQubitState;
        }
    }
}

void UnionTable::print(std::ostream &os) const {
    os << this->to_string() << std::endl;
}

std::string UnionTable::to_string() const {
    std::stringstream os;
    for(int i = 0; i < nQubits; i++) {
        os << i << ": -> ";
        if (std::holds_alternative<TOP>(this->table[i])) {
            os << "Top" << std::endl;
        } else {
            std::shared_ptr<QubitState> qubitState = std::get<std::shared_ptr<QubitState>>(this->table[i]);
            os << std::hex << qubitState << std::dec << ": ";
            qubitState->print(os);
            os << std::endl;
        }
    }
    return os.str();
}
