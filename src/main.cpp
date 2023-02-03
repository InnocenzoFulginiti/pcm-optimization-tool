//
// Created by Jakob on 20/01/2023.
//

#include <string>
#include <iostream>
#include "../include/QuantumComputation.hpp"
#include "../include/CircuitOptimization.hpp"
#include "../include/ConstantPropagation.hpp"


using namespace std;

int main() {
    std::string FILENAME_IN = "../test/circuits/circ1.qasm";
    std::string FILENAME_OUT = "../test/circuits/circ1_OUT.qasm";

    qc::QuantumComputation qc(FILENAME_IN);

    //Print the circuit
    qc.print(std::cout);
    std::cout << std::endl;

    //List of CircuitOptimizations that are all allplied to ops
    std::vector<std::unique_ptr<CircuitOptimization>> optimizations;
    optimizations.push_back(std::make_unique<ConstantPropagation>());

    //Apply all CircuitOptimizations to ops
    for (auto& opt : optimizations) {
        opt->optimize(qc);
    }


    for(auto& gate : qc) {
        std::cout << gate->getName() << std::endl;
        gate->print(std::cout);
    }

    return 0;
}