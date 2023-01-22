//
// Created by Jakob on 20/01/2023.
//

#include <fstream>
#include <iostream>
#include "QasmParser.hpp"

Circuit QasmParser::parse(std::string filename) {
    //Parse OpenQASM file
    auto ifs = std::ifstream(filename);

    if (ifs.bad()) {
        throw std::runtime_error("Could not open file " + filename);
    } else {
        std::cout << "Opened file " << filename << std::endl;
        //Print all lines of file
        std::string line;
        while (std::getline(ifs, line)) {
            std::cout << line << std::endl;
        }

        return Circuit(0);
    }
}
