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
    std::vector<double> params = op.getParameter();

    switch (op.getType()) {
        case qc::None:
            return {0, 0, 0, 0};
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
            return U1(PI_2);
        case qc::Sdag:
            return U1(-PI_2);
        case qc::T:
            return {1, 0, 0, Complex(0, PI / 4).exp()};
        case qc::Tdag:
            return {1, 0, 0, Complex(0, -PI / 4).exp()};
        case qc::V:
            return {Complex(0.5, 0.5), Complex(0.5, -0.5), Complex(0.5, -0.5), Complex(0.5, 0.5)};
        case qc::Vdag:
            return {Complex(0.5, -0.5), Complex(0.5, 0.5), Complex(0.5, 0.5), Complex(0.5, -0.5)};
        case qc::U3:
            return U3(params[0], params[1], params[2]);
        case qc::U2:
            return U2(params[0], params[1]);
        case qc::Phase:
            return U1(params[0]);
        case qc::SX:
            return {Complex(0.5, 0.5), Complex(0.5, -0.5),
                    Complex(0.5, -0.5), Complex(0.5, 0.5)};
        case qc::SXdag:
            return {Complex(0.5, -0.5), Complex(0.5, 0.5),
                    Complex(0.5, 0.5), Complex(0.5, -0.5)};
        case qc::RX:
            return U3(params[0], -PI_2, PI_2);
        case qc::RY:
            return U3(params[0], 0, 0);
        case qc::RZ:
            return U1(params[0]);
        case qc::GPhase:
            return {Complex(0, params[0]).exp(), 0, 0, Complex(0, params[0]).exp()};
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
        case qc::DCX:
        case qc::ECR:
        case qc::RXX:
        case qc::RYY:
        case qc::RZZ:
        case qc::RZX:
        case qc::XXminusYY:
        case qc::XXplusYY:
        default:
            std::cout << "Operation::getMatrix() not implemented for " << op.getName() << std::endl;
            break;
    }

    return {0, 0, 0, 0};
}

std::array<std::array<Complex, 4>, 4> getTwoQubitMatrix(const qc::Operation &op) {
    double t;
    double b;
    switch (op.getType()) {
        case qc::iSWAP:
            return {{
                            {1, 0, 0, 0},
                            {0, 0, Complex(0, 1), 0},
                            {0, Complex(0, 1), 0, 0},
                            {0, 0, 0, 1}
                    }};
        case qc::ECR:
            return {{
                            {0, SQRT_2_2, 0, Complex(0, SQRT_2_2)},
                            {SQRT_2_2, 0, Complex(0, -SQRT_2_2), 0},
                            {0, Complex(0, SQRT_2_2), 0, SQRT_2_2},
                            {Complex(0, -SQRT_2_2), 0, SQRT_2_2, 0}
                    }};
        case qc::DCX:
            return {{
                            {1, 0, 0, 0},
                            {0, 0, 0, 1},
                            {0, 1, 0, 0},
                            {0, 0, 1, 0}
                    }};
        case qc::RXX:
            t = op.getParameter()[0] / 2.0;
            return {{
                            {cos(t), 0, 0, Complex(0, -sin(t))},
                            {0, cos(t), Complex(0, -sin(t)), 0},
                            {0, Complex(0, -sin(t)), cos(t), 0},
                            {Complex(0, -sin(t)), 0, 0, cos(t)}
                    }};
        case qc::RYY:
            t = op.getParameter()[0] / 2.0;
            return {{
                            {cos(t), 0, 0, Complex(0, sin(t))},
                            {0, cos(t), Complex(0, -sin(t)), 0},
                            {0, Complex(0, -sin(t)), cos(t), 0},
                            {Complex(0, sin(t)), 0, 0, cos(t)}
                    }};
        case qc::RZZ:
            t = op.getParameter()[0] / 2.0;
            return {{
                            {Complex(0, -t).exp(), 0, 0, 0},
                            {0, Complex(0, t).exp(), 0, 0},
                            {0, 0, Complex(0, t).exp(), 0},
                            {0, 0, 0, Complex(0, -t).exp()}
                    }};
        case qc::RZX:
            t = op.getParameter()[0] / 2.0;
            return {{
                            {cos(t), 0, Complex(0, -sin(t)), 0},
                            {0, cos(t), 0, Complex(0, sin(t))},
                            {Complex(0, -sin(t)), 0, cos(t), 0},
                            {0, Complex(0, sin(t)), 0, cos(t)}
                    }};
        case qc::XXplusYY:
            t = op.getParameter()[0] / 2.0;
            b = op.getParameter()[1];
            return {{
                            {1, 0, 0, 0},
                            {0, cos(t), Complex(0, -sin(t)) * Complex(0, b).exp(), 0},
                            {0, Complex(0, -sin(t)) * Complex(0, -b).exp(), cos(t), 0},
                            {0, 0, 0, 1}
                    }};
        case qc::XXminusYY:
            t = op.getParameter()[0] / 2.0;
            b = op.getParameter()[1];
            return {{
                            {cos(t), 0, 0, Complex(0, -sin(t)) * Complex(0, -b).exp()},
                            {0, 1, 0, 0},
                            {0, 0, 1, 0},
                            {Complex(0, -sin(t)) * Complex(0, b).exp(), 0, 0, cos(t)}
                    }};
        case qc::None:
        case qc::GPhase:
        case qc::I:
        case qc::H:
        case qc::X:
        case qc::Y:
        case qc::Z:
        case qc::S:
        case qc::Sdag:
        case qc::T:
        case qc::Tdag:
        case qc::V:
        case qc::Vdag:
        case qc::U3:
        case qc::U2:
        case qc::Phase:
        case qc::SX:
        case qc::SXdag:
        case qc::RX:
        case qc::RY:
        case qc::RZ:
        case qc::SWAP:
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
            std::cout << "Operation::getTwoQubitMatrix() not implemented for " << op.getName() << std::endl;
            return {{
                            {0, 0, 0, 0},
                            {0, 0, 0, 0},
                            {0, 0, 0, 0},
                            {0, 0, 0, 0}
                    }};
    };
}
