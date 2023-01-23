//
// Created by Jakob on 20/01/2023.
//

#ifndef QCPROP_CIRCUIT_HPP
#define QCPROP_CIRCUIT_HPP

#include <cstdlib>

class Circuit {
public:
    Circuit(size_t n);
    ~Circuit();

    size_t getSize();
private:
    size_t numQubits;
};


#endif //QCPROP_CIRCUIT_HPP
