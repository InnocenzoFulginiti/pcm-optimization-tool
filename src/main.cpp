//
// Created by Jakob on 20/01/2023.
//

#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include "../include/QuantumComputation.hpp"
#include "../include/ConstantPropagation.hpp"


using namespace std;

int main(int argc, char *argv[]) {
    // Check if exactly two file paths were provided
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <input_file_path> <output_file_path> <n_pcm>" << endl;
        return 1;
    }

    std::string FILENAME_IN = argv[1];
    std::string FILENAME_OUT = argv[2];

    size_t max_ent_group_size = std::stoull(argv[3]);

    qc::QuantumComputation qc(FILENAME_IN);
    qc.unifyQuantumRegisters("q");

    // Print the circuit
    // qc.print(std::cout);

    ConstantPropagation::optimize(qc, MAX_AMPLITUDES, max_ent_group_size);

    // Print the optimized circuit
    // qc.print(std::cout);

    std::ofstream file_out(FILENAME_OUT);
    qc.dumpOpenQASM(file_out);

    file_out.close();

    return 0;
}