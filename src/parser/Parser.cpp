//
// Created by Jakob on 20/01/2023.
//

#include "Parser.hpp"
#include "QasmParser.hpp"

Circuit Parser::parse(std::string filename) {
    return QasmParser::parse(filename);
}
