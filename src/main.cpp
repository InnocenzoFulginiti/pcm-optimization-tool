//
// Created by Jakob on 20/01/2023.
//

#include <iostream>

#include "circuit/Circuit.hpp"

using namespace std;

int main() {

    Circuit c(5);

    cout << "Circuit size: " << c.getSize() << endl;

    return 0;
}