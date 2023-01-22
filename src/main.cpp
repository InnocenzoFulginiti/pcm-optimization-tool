//
// Created by Jakob on 20/01/2023.
//

#include <iostream>


#include "circuit/Circuit.hpp"
#include "parser/Parser.hpp"

using namespace std;

int main() {

    Parser::parse("../../test/circuits/circ1.qasm");

    return 0;
}