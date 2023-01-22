//
// Created by Jakob on 20/01/2023.
//

#ifndef QCPROP_PARSER_HPP
#define QCPROP_PARSER_HPP


#include <string>
#include "../circuit/Circuit.hpp"

class Parser {
    public:
    static Circuit parse(std::string filename);
};


#endif //QCPROP_PARSER_HPP
