#include "MatrixGenerator.hpp"

#include "Definitions_qcprop.hpp"

std::array<Complex, 4> U3(double theta, double phi, double lambda) {
    return {
            Complex(cos(theta / 2), 0) * Complex(0, 0 - (phi + lambda) / 2).exp(),
            Complex(0, 0) - Complex(sin(theta / 2), 0) * Complex(0, 0 - ((phi - lambda) / 2)).exp(),
            Complex(sin(theta / 2), 0) * Complex(0, (phi - lambda) / 2).exp(),
            Complex(cos(theta / 2), 0) * Complex(0, (phi + lambda) / 2).exp()
    };
}

std::array<Complex, 4> U2(double phi, double lambda) {
    return U3(PI_2, phi, lambda);
}

std::array<Complex, 4> U1(double lambda) {
    return U3(0, 0, lambda);
}

std::array<Complex, 4> getMatrix(const qc::Operation &op) {
    auto params = op.getParameter();
    double theta = params[2];
    double phi = params[1];
    double lambda = params[0];

    switch (op.getType()) {
        case qc::None:
            break;
        case qc::I:
            return U3(0, 0, 0);
        case qc::H:
            return {SQRT_2_2, SQRT_2_2, SQRT_2_2, -SQRT_2_2};
        case qc::X:
            return {0, 1, 1, 0};
        case qc::Y:
            return {0, Complex(0, -1), Complex(0, 1), 0};
        case qc::Z:
            return {1, 0, 0, -1};
        case qc::S:
            return {1, 0, 0, Complex(0, 1)};
        case qc::Sdag:
            return {1, 0, 0, Complex(0, -1)};
        case qc::T:
            return {1, 0, 0, Complex(0, PI / 4).exp()};
        case qc::Tdag:
            return {1, 0, 0, Complex(0, -PI / 4).exp()};
        case qc::V:
            return {Complex(0.5, 0.5), Complex(0.5, -0.5), Complex(0.5, -0.5), Complex(0.5, 0.5)};
        case qc::Vdag:
            return {Complex(0.5, -0.5), Complex(0.5, 0.5), Complex(0.5, 0.5), Complex(0.5, -0.5)};
        case qc::U3:
            return U3(theta, phi, lambda);
        case qc::U2:
            return U2(phi, lambda);
        case qc::Phase:
            return U1(lambda);
        case qc::SX:
            return {Complex(0.5, 0.5), Complex(0.5, -0.5),
                    Complex(0.5, -0.5), Complex(0.5, 0.5)};
        case qc::SXdag:
            return {Complex(0.5, -0.5), Complex(0.5, 0.5),
                    Complex(0.5, 0.5), Complex(0.5, -0.5)};
        case qc::RX:
            return U3(lambda, -PI_2, PI_2);
        case qc::RY:
            return U3(lambda, 0, 0);
        case qc::RZ:
            return U1(lambda);
        case qc::SWAP:
        case qc::iSWAP:
        case qc::Peres:
        case qc::Peresdag:
        case qc::Compound:
        case qc::Measure:
        case qc::Reset:
        case qc::Snapshot:
        case qc::ShowProbabilities:
        case qc::Barrier:
        case qc::Teleportation:
        case qc::ClassicControlled:
        case qc::ATrue:
        case qc::AFalse:
        case qc::MultiATrue:
        case qc::MultiAFalse:
        case qc::OpCount:
        default:
            std::cout << "Operation::getMatrix() not implemented for " << op.getName() << std::endl;
            break;
    }

    return {0, 0, 0, 0};
}

