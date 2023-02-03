//
// Created by Jakob on 01/02/2023.
//

#include "../../include/util/BitSet.hpp"

std::ostream &operator<<(std::ostream &os, const BitSet &bitSet) {
    for(int i = ((int) bitSet.size)-1; i >= 0; i--)
        os << bitSet.bits[i];

    return os;
}
