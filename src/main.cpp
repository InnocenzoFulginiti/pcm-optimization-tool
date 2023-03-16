//
// Created by Jakob on 20/01/2023.
//

#include <string>
#include <iostream>
#include "../include/QuantumComputation.hpp"
#include "../include/ConstantPropagation.hpp"


using namespace std;

int main() {
    std::string FILENAME_IN = "../test/circuits/circ1.qasm";
    std::string FILENAME_OUT = "../test/circuits/circ1_OUT.qasm";

    qc::QuantumComputation qc(FILENAME_IN);
    qc.unifyQuantumRegisters("q");

    //Print the circuit
    qc.print(std::cout);
    std::cout << std::endl;

    ConstantPropagation::optimize(qc);

    qc.print(std::cout);

    return 0;
}