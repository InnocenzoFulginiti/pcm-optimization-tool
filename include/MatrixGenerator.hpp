#ifndef QCPROP_MATRIXGENERATOR_HPP
#define QCPROP_MATRIXGENERATOR_HPP

#include <array>

#include "../extern/qfr/include/operations/Operation.hpp"
#include "util/Complex.hpp"

std::array<Complex, 4> getMatrix(const qc::Operation &op);

std::array<std::array<Complex, 4>, 4> getTwoQubitMatrix(const qc::Operation &op);

#endif //QCPROP_MATRIXGENERATOR_HPP
