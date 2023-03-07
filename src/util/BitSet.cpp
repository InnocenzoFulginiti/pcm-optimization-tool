//
// Created by Jakob on 01/02/2023.
//

#include "../../include/util/BitSet.hpp"

std::ostream &operator<<(std::ostream &os, const BitSet &bitSet) {
    for(size_t i = bitSet.size-1;; i--) {
        os << bitSet.bits[i];
        if(i == 0) break;
    }

    return os;
}
