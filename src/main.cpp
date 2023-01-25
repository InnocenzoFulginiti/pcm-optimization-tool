//
// Created by Jakob on 20/01/2023.
//

#include <string>
#include <iostream>
#include "../include/QuantumComputation.hpp"


using namespace std;

int main() {
    std::string FILENAME_IN = "../test/circuits/circ1.qasm";
    std::string FILENAME_OUT = "../test/circuits/circ1_OUT.qasm";

    qc::QuantumComputation qc(FILENAME_IN);

    size_t nQubits = qc.getNqubits();

    //Print the circuit
    qc.print(std::cout);
    std::cout << std::endl;

    //Vector of Operations that represent the entire Circuit
    auto ops = qc.getOps();

    //Modify circuit and remove operations with erase(i)
    ops.erase(ops.begin()+1);
    ops.erase(ops.begin()+2);

    //Add Operation to qc
    qc::QuantumComputation qc_out(nQubits, ops);

    qc_out.print(std::cout);

    qc_out.dump(FILENAME_OUT);

    return 0;
}