//
// Created by Jakob on 20/01/2023.
//

#include "Circuit.hpp"

Circuit::Circuit(size_t n) {
    numQubits = n;
}

Circuit::~Circuit() = default;

size_t Circuit::getSize() {
    return numQubits;
}
