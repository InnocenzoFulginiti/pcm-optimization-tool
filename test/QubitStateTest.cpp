#include "TestUtils.hpp"

TEST_CASE("QubitState normalization example") {
    Complex c1(1, 0);
    Complex c2(1, 0);

    QubitState qs(1);
    qs[BitSet(1, 0)] = c1;
    qs[BitSet(1, 1)] = c2;

    qs.normalize();
    REQUIRE(qs.norm() == Catch::Approx(1.0));
}

TEST_CASE("removeZeroEntries preserves norm") {
    QubitState qs(2);
    qs[BitSet(2, 0)] = Complex(0.5, 0);
    qs[BitSet(2, 1)] = Complex(0.5, 0);
    qs[BitSet(2, 2)] = Complex(SQRT_2_2, 0);

    REQUIRE(qs.norm() == Catch::Approx(1.0).margin(1e-10));

    INFO("Before:");
    CAPTURE(qs.to_string());

    double epsilon = Complex::getEpsilon();
    Complex::setEpsilon(0.6);
    qs.removeZeroEntries();
    Complex::setEpsilon(epsilon);

    INFO("After:");
    CAPTURE(qs.to_string());

    REQUIRE(qs.norm() == Catch::Approx(1.0).margin(1e-10));
}

TEST_CASE("QubitState::removeZeroEntries keeps normalization") {
    size_t size = static_cast<size_t>(GENERATE(take(10, random(1, 4))));
    unsigned int seed = static_cast<unsigned int>(GENERATE(take(10, random(0, 1000000))));

    CAPTURE(size, seed);

    auto qs = generateRandomState(size, seed);


    //Range: 10^[-10;-0.1] = [1e-10; ~0.8]
    double exp = GENERATE(take(10, random(-10.0, -0.1)));
    double epsilon = pow(5, exp);

    CAPTURE(epsilon);

    double oldEps = Complex::getEpsilon();


    REQUIRE(qs->norm() == Catch::Approx(1.0).margin(1e-10));

    INFO("Before:");
    CAPTURE(qs->to_string());

    Complex::setEpsilon(epsilon);

    qs->removeZeroEntries();

    Complex::setEpsilon(oldEps);

    INFO("After:");
    CAPTURE(qs->to_string());

    REQUIRE(qs->norm() == Catch::Approx(1.0).margin(1e-10));
}

TEST_CASE("QubitState normalization random test") {
    //Generate 4*4*4*4 = 256 doubles
    auto p1 = GENERATE(take(4, random(-10.0, 10.0)));
    auto p2 = GENERATE(take(4, random(-10.0, 10.0)));
    auto p3 = GENERATE(take(4, random(-10.0, 10.0)));
    auto p4 = GENERATE(take(4, random(-10.0, 10.0)));

    auto c1 = Complex(p1, p2);
    auto c2 = Complex(p3, p4);

    QubitState qs(1);
    qs[BitSet(1, 0)] = c1;
    qs[BitSet(1, 1)] = c2;

    qs.normalize();

    REQUIRE(qs.norm() == Catch::Approx(1.0));
}

TEST_CASE("QubitState Merge Example") {
    //Build Register for testing
    /*
     * 0&2: |00> = 1/sqrt(2), |11> = 1/sqrt(2)
     * 1: |0> = 0.5, |1> = sqrt(3)/2
     */

    QubitStateOrTop testQReg[3]{
            std::make_shared<QubitState>(2),
            std::make_shared<QubitState>(1),
            TOP::T
    };

    (testQReg[0]).getQubitState()->clear();
    (*testQReg[0].getQubitState())[BitSet(2, 0)] = Complex(0.70710678118, 0);
    (*testQReg[0].getQubitState())[BitSet(2, 3)] = Complex(0.70710678118, 0);
    (*testQReg[1].getQubitState())[BitSet(1, 0)] = Complex(0.5, 0);
    (*testQReg[1].getQubitState())[BitSet(1, 1)] = Complex(0.86602540378, 0);

    testQReg[2] = testQReg[0].getQubitState();

    std::shared_ptr<QubitState> q0_2 = testQReg[0].getQubitState();
    std::shared_ptr<QubitState> q1 = testQReg[1].getQubitState();


    std::shared_ptr<QubitState> result = QubitState::combine(q0_2, std::vector{0, 2}, q1, std::vector{1});

    Complex expected0 = Complex(SQRT_2_2 / 2.0, 0);
    Complex expected2 = Complex(SQRT_3 * SQRT_2_2 / 2.0, 0);
    Complex expected5 = expected0;
    Complex expected7 = expected2;

    //Make sure my solution is at least normed
    double sum = expected0.norm() + expected2.norm() + expected5.norm() + expected7.norm();
    REQUIRE(sum == Catch::Approx(1.0));

    //Check if the result is correct
    REQUIRE(result->size() == 4);

    approx(expected0, (*result)[BitSet(3, 0)]);

    approx(expected2, (*result)[BitSet(3, 2)]);

    approx(expected5, (*result)[BitSet(3, 5)]);

    approx(expected7, (*result)[BitSet(3, 7)]);
}

TEST_CASE("Test Gate Identities") {
    std::array<Complex, 4> X = {0, 1, 1, 0};
    std::array<Complex, 4> I = {1, 0, 0, 1};

    std::array<Complex, 4> Y = {0, Complex(0, -1), Complex(0, 1), 0};

    std::array<Complex, 4> Z = {1, 0, 0, -1};

    std::array<Complex, 4> H = {
            Complex(SQRT_2_2, 0), Complex(SQRT_2_2, 0),
            Complex(SQRT_2_2, 0), Complex(-SQRT_2_2, 0)
    };

    std::array<Complex, 4> S = {1, 0, 0, Complex(0, 1)};
    std::array<Complex, 4> T = {1, 0, 0, Complex(SQRT_2_2, SQRT_2_2)};

    size_t nQubits = static_cast<size_t>(GENERATE(1, 2, 3, 4, 5, 6, 7, 8, 9, 10));
    unsigned int seed = static_cast<unsigned int>(GENERATE(take(10, random(0, 10000))));

    auto qs = generateRandomState(nQubits, seed);

    for (size_t target = 0; target < nQubits; target++) {
        SECTION("I = I") {
            auto qsI = qs->clone();
            qsI->applyGate(target, I);
            approxQubitState(qs, qsI);
        }

        SECTION("XX = I") {
            auto qsXX = qs->clone();
            qsXX->applyGate(target, X);
            qsXX->applyGate(target, X);
            approxQubitState(qs, qsXX);
        }

        SECTION("HH = I") {
            auto qsHH = qs->clone();
            qsHH->applyGate(target, H);
            qsHH->applyGate(target, H);
            approxQubitState(qs, qsHH);
        }

        SECTION("ZZ = I") {
            auto qsZZ = qs->clone();
            qsZZ->applyGate(target, Z);
            qsZZ->applyGate(target, Z);
            approxQubitState(qs, qsZZ);
        }

        SECTION("YY = I") {
            auto qsYY = qs->clone();
            qsYY->applyGate(target, Y);
            qsYY->applyGate(target, Y);
            approxQubitState(qs, qsYY);
        }

        SECTION("TT = S") {
            auto qsTT = qs->clone();
            qsTT->applyGate(target, T);
            qsTT->applyGate(target, T);
            auto qsS = qs->clone();
            qsS->applyGate(target, S);
            approxQubitState(qsS, qsTT);
        }

        SECTION("SS = Z") {
            auto qsSS = qs->clone();
            qsSS->applyGate(target, S);
            qsSS->applyGate(target, S);
            auto qsZ = qs->clone();
            qsZ->applyGate(target, Z);
            approxQubitState(qsZ, qsSS);
        }

        SECTION("HXH = Z") {
            auto qsHXH = qs->clone();
            qsHXH->applyGate(target, H);
            qsHXH->applyGate(target, X);
            qsHXH->applyGate(target, H);
            auto qsZ = qs->clone();
            qsZ->applyGate(target, Z);
            approxQubitState(qsZ, qsHXH);
        }

        SECTION("HZH = X") {
            auto qsHZH = qs->clone();
            qsHZH->applyGate(target, H);
            qsHZH->applyGate(target, Z);
            qsHZH->applyGate(target, H);
            auto qsX = qs->clone();
            qsX->applyGate(target, X);
            approxQubitState(qsX, qsHZH);
        }
    }
}

TEST_CASE("Test Reorder Indices") {
    const int nQubits = 5;

    QubitState sut(nQubits);
    sut.clear();

    int key = GENERATE(take(10, random(0, (1 << nQubits) - 1)));
    std::bitset<5> keyBS(static_cast<unsigned long long int>(key));

    INFO("Key: " + std::to_string(key));
    INFO("Key: " + keyBS.to_string());
    BitSet keySet(nQubits, static_cast<size_t>(key));

    sut[keySet] = Complex(1, 0);

    size_t oldI = static_cast<size_t>(GENERATE(take(5, random(0, nQubits - 1))));
    size_t newI = static_cast<size_t>(GENERATE(take(5, random(0, nQubits - 1))));


    INFO("oldI: " + std::to_string(oldI));
    INFO("newI: " + std::to_string(newI));

    sut.reorderIndex(oldI, newI);

    INFO("State after first reorder: " + sut.to_string());

    sut.reorderIndex(newI, oldI);

    INFO("State after second reorder: " + sut.to_string());
    CHECK(sut[keySet] == Complex(1, 0));
}