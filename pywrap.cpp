#include "include/ConstantPropagation.hpp"
#include <pybind11/pybind11.h>

namespace py = pybind11;

bool fileToFile(const std::string &in, const std::string &out, size_t maxAmpls, double threshold) {
    try {
        qc::QuantumComputation qc(in);

        Complex::setEpsilon(threshold);
        ConstantPropagation::propagate(qc, maxAmpls, MAX_ENT_GROUP_SIZE);

        std::ofstream fileOut(out, std::ios::out | std::ios::trunc);
        qc.dumpOpenQASM(fileOut);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return false;
    }

    return true;
}

std::string optimize(const std::string &in, size_t maxAmpls, double threshold) {
    std::istringstream iss(in);
    qc::QuantumComputation circuit;
    circuit.import(iss, qc::Format::OpenQASM);

    Complex::setEpsilon(threshold);
    ConstantPropagation::propagate(circuit, maxAmpls, MAX_ENT_GROUP_SIZE);
    std::stringstream oss;
    circuit.dumpOpenQASM(oss);
    return oss.str();
}

PYBIND11_MODULE(qcprop_py, m) {
    m.doc() = "QCPROP Module";

    m.def("fileToFile", &fileToFile,
          py::arg("path_in"),
          py::arg("path_out"),
          py::arg("maxAmpls"),
          py::arg("threshold"));

    m.def("optimize", &optimize,
          py::arg("circuit"),
          py::arg("maxAmpls"),
          py::arg("threshold"));
}