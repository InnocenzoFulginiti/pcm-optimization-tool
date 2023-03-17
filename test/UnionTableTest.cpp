//
// Created by Jakob on 02/02/2023.
//

#include "TestUtils.hpp"

TEST_CASE("Make a Union Table") {
    UnionTable ut(3);
    REQUIRE(ut.size() == 3);
}

TEST_CASE("Test combine Method") {
    UnionTable table(3);

    table.combine(0, 1);

    REQUIRE(table[0].getQubitState().get() == table[1].getQubitState().get());
    REQUIRE(table[1].getQubitState().get() != table[2].getQubitState().get());
}

TEST_CASE("Test UnionTable::minimizeControls Example 1") {
    UnionTable ut(5);
    //Entangle All
    ut[4] = ut[3] = ut[2] = ut[1] = ut[0];

    auto qs = ut[0].getQubitState();
    qs->clear();
    //Values so that 4 => 3, 2, 0
    (*qs)[0b00000] = 1;
    (*qs)[0b00001] = 1;
    (*qs)[0b00100] = 1;
    (*qs)[0b00101] = 1;
    (*qs)[0b01101] = 1;
    (*qs)[0b11101] = 1;

    auto ret = ut.minimizeControls({0, 2, 3, 4});
    auto min = ret.second;
    //Only "4" should be left
    CHECK(ret.second.size() == 1);
    CHECK(std::find(min.begin(), min.end(), 4) != min.end());
}

TEST_CASE("Test UnionTable::minimizeControls Example 2") {
    UnionTable ut(5);
    //Entangle All
    ut[4] = ut[3] = ut[2] = ut[1] = ut[0];

    auto qs = ut[0].getQubitState();
    qs->clear();
    //Values so that 4 => 3, 2 => 0, no other implication
    (*qs)[1] = 1;
    (*qs)[2] = 1;
    (*qs)[5] = 1;
    (*qs)[8] = 1;
    (*qs)[24] = 1;
    (*qs)[31] = 1;

    auto ret = ut.minimizeControls({0, 2, 3, 4});
    auto min = ret.second;

    //Only "4 & 2" should be left
    CHECK(ret.second.size() == 2);
    CHECK(std::find(min.begin(), min.end(), 4) != min.end());
    CHECK(std::find(min.begin(), min.end(), 2) != min.end());
}

TEST_CASE("Test UnionTable::minimizeControls <=>") {
    UnionTable ut(5);
    //Entangle All
    ut[4] = ut[3] = ut[2] = ut[1] = ut[0];

    auto qs = ut[0].getQubitState();
    qs->clear();
    //Values so that 4 => 3, 4 <= 3,0 => 2, no others
    (*qs)[0b00000] = 1;
    (*qs)[0b00010] = 1;
    (*qs)[0b00100] = 1;
    (*qs)[0b00110] = 1;
    (*qs)[0b00111] = 1;
    (*qs)[0b11000] = 1;
    (*qs)[0b11010] = 1;
    (*qs)[0b11100] = 1;
    (*qs)[0b11101] = 1;
    (*qs)[0b11110] = 1;
    (*qs)[0b11111] = 1;

    auto [act, min] = ut.minimizeControls({0, 2, 3, 4});

    //Only "4/3, 0" should be left
    CHECK(min.size() == 2);

    CHECK(std::find(min.begin(), min.end(), 0) != min.end());

    bool has3 = std::find(min.begin(), min.end(), 3) != min.end();
    bool has4 = std::find(min.begin(), min.end(), 4) != min.end();

    CHECK((has3 || has4));
}

TEST_CASE("Test UnionTable::minimizeControls not able to activate") {
    UnionTable ut(5);
    //Entangle All
    ut[4] = ut[3] = ut[2] = ut[1] = ut[0];

    auto qs = ut[0].getQubitState();
    qs->clear();
    //4 => ~3 -> 4 and 3 can not activate, 3 => 2
    (*qs)[0b00000] = 1;
    (*qs)[0b00001] = 1;
    (*qs)[0b00100] = 1;
    (*qs)[0b00110] = 1;
    (*qs)[0b01100] = 1;
    (*qs)[0b10000] = 1;

    qs->normalize();

    auto [act, min] = ut.minimizeControls({0, 2, 3, 4});

    CAPTURE(act);
    CAPTURE(to_string(act));
    REQUIRE(act == ActivationState::NEVER);
}

TEST_CASE("Test UnionTable::minimizeControls") {
    UnionTable ut(6);
    ut[0] = TOP::T;
    ut[1].getQubitState()->clear();
    (*ut[1].getQubitState())[0] = SQRT_2_2;
    (*ut[1].getQubitState())[1] = SQRT_2_2;

    ut[3] = ut[2];
    ut[3].getQubitState()->clear();
    (*ut[3].getQubitState())[2] = SQRT_2_2;
    (*ut[3].getQubitState())[3] = SQRT_2_2;

    ut[4] = ut[5];
    ut[4].getQubitState()->clear();
    (*ut[4].getQubitState())[0] = SQRT_2_2;
    (*ut[4].getQubitState())[3] = SQRT_2_2;

    auto res = ut.minimizeControls({0, 1, 2, 3, 4, 5});

    CHECK(res.second.size() == 4);
}

TEST_CASE("Test UnionTable SWAP") {
    size_t size = static_cast<size_t>(GENERATE(1, 2, 10));
    CAPTURE(size);
    auto sut = generateRandomUnionTable(size, 1256258075);
    CAPTURE(sut->to_string());

    size_t i = static_cast<size_t>(GENERATE(take(3, random(0, 500)))) % size;
    size_t j = static_cast<size_t>(GENERATE(take(3, random(0, 500)))) % size;

    CAPTURE(i);
    CAPTURE(j);

    auto expected = sut->clone();

    sut->swap(i, j);
    INFO("After first swap:");
    CAPTURE(sut->to_string());
    sut->swap(i, j);

    INFO("After second swap:");
    CAPTURE(sut->to_string());

    approxUnionTable(expected, sut);
}

TEST_CASE("Test single RXX gate") {
    qc::QuantumComputation qc(4);
    qc.h(1);
    qc.rxx(1, 2, PI_2);

    auto ut = ConstantPropagation::propagate(qc, 10);
    REQUIRE(qc.getNops() == 2);
    REQUIRE(!ut->isTop(0));
    REQUIRE(!ut->isTop(1));
    REQUIRE(!ut->isTop(2));
    REQUIRE(!ut->isTop(3));

    auto state = ut->operator[](1).getQubitState();
    REQUIRE(state->size() == 4);

    approx(0.5, (*state)[0b00]);
    approx(0.5, (*state)[0b01]);
    approx(Complex(0, -0.5), (*state)[0b10]);
    approx(Complex(0, -0.5), (*state)[0b11]);
}

TEST_CASE("Test multiple RXX gates") {
    qc::QuantumComputation qc(4);
    qc.h(1);
    qc.rxx(1, 2, PI_2);
    qc.rxx(2, 3, PI / 4.0);
    qc.rxx(0, 1, PI * 1.5);

    auto ut = ConstantPropagation::propagate(qc, 16);

    REQUIRE(qc.getNops() == 4);

    compareUnitTableToState(ut, {
            {0,  Complex(-0.327, 0)},
            {1,  Complex(0, -0.327)},
            {2,  Complex(-0.327, 0)},
            {3,  Complex(0, -0.327)},
            {4,  Complex(0, 0.327)},
            {5,  Complex(-0.327, 0)},
            {6,  Complex(0, 0.327)},
            {7,  Complex(-0.327, 0)},
            {8,  Complex(0.135, 0)},
            {9,  Complex(0, 0.135)},
            {10, Complex(0.135, 0)},
            {11, Complex(0, 0.135)},
            {12, Complex(0, 0.135)},
            {13, Complex(-0.135, 0)},
            {14, Complex(0, 0.135)},
            {15, Complex(-0.135, 0)}
    });
}

TEST_CASE("Test RZZ gate") {
    qc::QuantumComputation qc(4);
    qc.h(0);
    qc.rzz(0, 1, PI * 1.5);

    auto ut = ConstantPropagation::propagate(qc, 16);

    REQUIRE(qc.getNops() == 2);
    compareUnitTableToState(ut, {
            {0b0000, Complex(-0.5, -0.5)},
            {0b0001, Complex(-0.5, 0.5)}
    });
}

TEST_CASE("Test RXX, RYY, RZZ identities") {
    size_t UT_SIZE = 2;
    auto sut = generateRandomUnionTable(UT_SIZE);


    unsigned int t1 = 0;
    unsigned int t2 = 1;

    auto sut_combined = sut->clone();
    sut_combined->combine(t1, t2);

    SECTION("RXX(0) = I") {
        qc::QuantumComputation rxxI(2);
        rxxI.rxx(t1, t2, 0);
        auto result = sut->clone();
        ConstantPropagation::propagate(rxxI, 1 << UT_SIZE, result);

        approxUnionTable(sut_combined, result);
    }

    SECTION("RXX(4*pi) = I") {
        qc::QuantumComputation rxxI(2);
        rxxI.rxx(t1, t2, 4 * PI);
        auto result = sut->clone();
        ConstantPropagation::propagate(rxxI, 1 << UT_SIZE, result);

        approxUnionTable(sut_combined, result);
    }

    SECTION("RZZ(0) = I") {
        qc::QuantumComputation rzzI(2);
        rzzI.rzz(t1, t2, 0);
        auto result = sut->clone();
        ConstantPropagation::propagate(rzzI, 1 << UT_SIZE, result);

        approxUnionTable(sut_combined, result);
    }

    SECTION("RZZ(4*pi) = I") {
        qc::QuantumComputation rzzI(2);
        rzzI.rzz(t1, t2, 4 * PI);
        auto result = sut->clone();
        ConstantPropagation::propagate(rzzI, 1 << UT_SIZE, result);

        approxUnionTable(sut_combined, result);
    }

    SECTION("RYY(0) = I") {
        qc::QuantumComputation ryyI(2);
        ryyI.ryy(t1, t2, 0);
        auto result = sut->clone();
        ConstantPropagation::propagate(ryyI, 1 << UT_SIZE, result);

        approxUnionTable(sut_combined, result);
    }
}

TEST_CASE("Test iSWAP") {
    size_t size_ut = static_cast<size_t>(GENERATE(2, 3, 8));
    unsigned int seed = static_cast<unsigned int>(GENERATE(take(3, random(0, 1000000))));
    CAPTURE(seed);
    CAPTURE(size_ut);
    unsigned int t1 = 0;
    unsigned int t2 = 1;

    CAPTURE(t1, t2);

    //Compare iSWAP(i, j) with it's decomposition
    //iSWAP(i, j) = S(i) S(j) H(i) CX(j, i) CX(i, j) H(j)

    qc::QuantumComputation iSWAP(size_ut);
    qc::QuantumComputation decomposition(size_ut);

    iSWAP.h(t1);
    iSWAP.iswap(t1, t2);

    //Just so there is 2 amplitudes
    decomposition.h(t1);
    //Decomposition
    decomposition.s(t1);
    decomposition.s(t2);
    decomposition.h(t1);
    decomposition.x(t2, {t1});
    decomposition.x(t1, {t2});
    decomposition.h(t2);

    auto startTable = generateRandomUnionTable(size_ut, 0.1, seed);

    auto iSWAPTable = startTable->clone();
    auto decompositionTable = startTable->clone();

    ConstantPropagation::propagate(iSWAP, 1024, iSWAPTable);
    ConstantPropagation::propagate(decomposition, 1024, decompositionTable);

    CAPTURE(iSWAPTable->to_string());
    CAPTURE(decompositionTable->to_string());

    approxUnionTable(iSWAPTable, decompositionTable, 1e-7);
}

TEST_CASE("Test DCX") {
    size_t size_ut = 3;
    unsigned int t1 = 0;
    unsigned int t2 = 1;

    //See if dcx(i, j) is equivalent to x(i, {j}) x(j, {i})

    auto startTable = generateRandomUnionTable(3, 0.1);

    qc::QuantumComputation dcx(size_ut);
    qc::QuantumComputation doubleCx(size_ut);

    dcx.h(t1);
    doubleCx.h(t1);

    dcx.dcx(t1, t2);

    doubleCx.x(t2, {t1});
    doubleCx.x(t1, {t2});

    auto dcxTable = startTable->clone();
    auto cxcxTable = startTable->clone();

    ConstantPropagation::propagate(dcx, 1 << size_ut, dcxTable);
    ConstantPropagation::propagate(doubleCx, 1 << size_ut, cxcxTable);

    CAPTURE(dcxTable->to_string());
    CAPTURE(cxcxTable->to_string());

    approxUnionTable(dcxTable, cxcxTable);
}

TEST_CASE("Test ECR") {
    size_t size_ut = 4;
    qc::QuantumComputation ecr(size_ut);

    ecr.h(1);
    ecr.x(2, {1});
    ecr.ecr(1, 2);

    auto ut = ConstantPropagation::propagate(ecr, 1 << size_ut);

    REQUIRE(!ut->isTop(1));
    REQUIRE(!ut->isTop(2));

    auto state = ut->operator[](1).getQubitState();

    auto expected = std::make_shared<QubitState>(2);
    expected->clear();
    (*expected)[BitSet(2, 0b00)] = Complex(0, 0.5);
    (*expected)[BitSet(2, 0b01)] = Complex(0.5, 0);
    (*expected)[BitSet(2, 0b10)] = Complex(0.5, 0);
    (*expected)[BitSet(2, 0b11)] = Complex(0, -0.5);

    CAPTURE(expected->to_string());
    CAPTURE(state->to_string());

    approxQubitState(expected, state);
}

TEST_CASE("Test RXX, RYY") {
    size_t size_ut = 4;
    qc::QuantumComputation I(size_ut);
    //Entangle qubits
    I.h(1);
    I.h(2);
    I.x(0, {1});
    I.x(3, {2});
    //Something that should cancel out if done correctly
    I.rzz(1, 2, PI);
    I.rxx(1, 2, PI);
    I.rzz(1, 2, PI);
    I.rxx(1, 2, PI);
    I.rzz(1, 2, PI);
    I.rxx(1, 2, PI);
    I.rzz(1, 2, PI);
    I.rxx(1, 2, PI);

    I.x(0, {1});
    I.x(3, {2});

    I.h(1);
    I.h(2);

    auto startUT = generateRandomUnionTable(size_ut);

    CAPTURE(startUT->to_string());

    auto actual = startUT->clone();

    ConstantPropagation::propagate(I, 1 << size_ut, actual);

    for (size_t i = 1; i < size_ut; i++) {
        startUT->combine(0, i);
    }

    INFO("Combined:");
    CAPTURE(startUT->to_string());
    CAPTURE(actual->to_string());

    approxUnionTable(startUT, actual);
}