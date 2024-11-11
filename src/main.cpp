//
// Created by Jakob on 20/01/2023.
//

#include <string>
#include <iostream>
#include <fstream>
#include "../include/QuantumComputation.hpp"
#include "../include/ConstantPropagation.hpp"


using namespace std;

int main(int argc, char *argv[]) {
    // Check if exactly two file paths were provided
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <input_file_path> <output_file_path>" << endl;
        return 1;
    }

    // std::string FILENAME_IN = "../test/circuits/circ2.qasm";
    // std::string FILENAME_OUT = "../test/circuits/circ2_OUT.qasm";
    std::string FILENAME_IN = argv[1];
    std::string FILENAME_OUT = argv[2];

    qc::QuantumComputation qc(FILENAME_IN);
    qc.unifyQuantumRegisters("q");

    //Print the circuit
    //qc.print(std::cout);

    ConstantPropagation::optimize(qc);

    //qc.print(std::cout);

    std::ofstream file_out(FILENAME_OUT);
    qc.dumpOpenQASM(file_out);

    // file_out.close();

    /* Add probabilistic gate definition to the output QASM file */
    std::ifstream file_out_i(FILENAME_OUT);
    std::string line;
    std::vector<std::string> lines;

    // Read the contents of the output file
    if (file_out_i.is_open()) {
        while (std::getline(file_out_i, line)) {
            lines.push_back(line);
        }
        file_out_i.close();
    }
    else {
        std::cerr << "Unable to open file for reading.";
        return 1;
    }

    // Add the new gate definition after the third line
    std::string newGateDefinition = "gate prob_x(prob) q {\n\tx q;\n}";
    if (lines.size() >= 4) {
        lines.insert(lines.begin() + 4, newGateDefinition);
    }

    // Write the modified contents back to the same file
    std::ofstream file_out_o(FILENAME_OUT); // Open the same file for writing
    if (file_out_o.is_open()) {
        for (const auto& line : lines) {
            file_out_o << line << std::endl;
        }
        file_out_o.close();
    }
    else {
        std::cerr << "Unable to open file for writing.";
        return 1;
    }

    return 0;
}