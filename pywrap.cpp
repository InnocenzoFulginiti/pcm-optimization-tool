#include "include/ConstantPropagation.hpp"
#include <pybind11/pybind11.h>

namespace py = pybind11;
constexpr auto byref = py::return_value_policy::reference_internal;

bool fileToFile(const std::string &in, const std::string &out, size_t maxAmpls, double threshold) {
    try {
        qc::QuantumComputation qc(in);

        Complex::setEpsilon(threshold);
        ConstantPropagation::propagate(qc, maxAmpls);

        std::ofstream fileOut(out, std::ios::out | std::ios::trunc);
        qc.dumpOpenQASM(fileOut);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return false;
    }

    return true;

}

PYBIND11_MODULE(qcprop_py, m) {
    m.doc() = "optional module docstring";

    m.def("fileToFile", &fileToFile,
          py::arg("path_in"),
          py::arg("path_out"),
          py::arg("maxAmpls"),
          py::arg("threshold"));
}