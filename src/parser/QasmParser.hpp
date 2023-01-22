//
// Created by Jakob on 20/01/2023.
//

#ifndef QCPROP_QASMPARSER_HPP
#define QCPROP_QASMPARSER_HPP


#include "Parser.hpp"

class QasmParser : Parser {
public:
    static Circuit parse(std::string filename);
};


#endif //QCPROP_QASMPARSER_HPP
