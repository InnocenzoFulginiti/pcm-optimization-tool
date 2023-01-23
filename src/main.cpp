//
// Created by Jakob on 20/01/2023.
//

#include <windows.h>
#include <string>
#include <iostream>
#include "../include/QuantumComputation.hpp"


using namespace std;

int main() {
    std::string FILENAME_IN = "../test/circuits/circ1.qasm";
    std::string FILENAME_OUT = "../test/circuits/circ1_OUT.qasm";

    qc::QuantumComputation qc(FILENAME_IN);



    size_t nQubits = qc.getNqubits();

    cout << "Number of qubits: " << nQubits << endl;

    cout << "Number of operations: " << qc.getNops() << endl;

    qc.print(std::cout);

    return 0;
}