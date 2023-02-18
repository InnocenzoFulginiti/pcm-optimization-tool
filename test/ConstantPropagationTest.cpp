#include "TestUtils.hpp"

TEST_CASE("Constant Propagation on small Files, multiple maxAmplitudes") {
    auto p = GENERATE(take(20, qasmFile(QASMFileGenerator::SIZE::SMALL)));
    auto maxAmplitude = GENERATE(1, 10, 40);

    INFO("File: " + p.string() + "Max Amplitude: " + std::to_string(maxAmplitude));

    qc::QuantumComputation qc(p);
    INFO("Number of Gates: " + std::to_string(qc.getNops()));
    CHECK_NOTHROW(ConstantPropagation::propagate(qc, maxAmplitude));
}

TEST_CASE("Try SWAP") {
    qc::QuantumComputation qc(3);
    qc.h(0);
    qc.swap(0, 1);

    INFO("Circuit: h[0]; swap[0,1]");
    auto res = ConstantPropagation::propagate(qc, 3);
    auto table = res.second;

    INFO("Table: " + table->to_string());
    REQUIRE((*table)[0].isQubitState());
    REQUIRE((*table)[1].isQubitState());

    auto q0 = (*table)[0].getQubitState();
    auto q1 = (*table)[1].getQubitState();

    CHECK((*q0)[1] == 0);
    CHECK((*q0)[3] == 0);
    CHECK((*q1)[0] == qc::SQRT_2_2);
    CHECK((*q1)[2] == qc::SQRT_2_2);
}

TEST_CASE("Try File with Reset") {
    auto fileWithReset = "../test/circuits/QASMBench/small/ipea_n2/ipea_n2.qasm";
    qc::QuantumComputation qc(fileWithReset);

    CHECK_NOTHROW(ConstantPropagation::propagate(qc, 3));
}

TEST_CASE("Try file with Classic Controlled gates") {
    auto classicControls = "../test/circuits/QASMBench/small/shor_n5/shor_n5.qasm";
    qc::QuantumComputation qc(classicControls);
    std::pair<std::vector<ActivationState>, std::shared_ptr<UnionTable>> res;

    std::stringstream ss;
    qc.print(ss);
    INFO("Circuit: " + ss.str());

    CHECK_NOTHROW(res = ConstantPropagation::propagate(qc, 10));

    auto table = res.second;
    INFO("Table: " + table->to_string());
    REQUIRE((*table)[0].isQubitState());
    REQUIRE((*table)[1].isQubitState());
    REQUIRE((*table)[2].isQubitState());
    REQUIRE((*table)[3].isQubitState());
    REQUIRE((*table)[4].isQubitState());

    std::shared_ptr<QubitState> state = (*table)[0].getQubitState();

    //TODO: Check state amplitudes
}

TEST_CASE("Try specific file") {
    //TODO: Handle Compound Gates
    auto fileWithCompoundGates = "../test/circuits/QASMBench/small/wstate_n3/wstate_n3.qasm";
    auto fileWithReset = "../test/circuits/QASMBench/small/ipea_n2/ipea_n2.qasm";
    auto classicControls = "../test/circuits/QASMBench/small/qec_sm_n5/qec_sm_n5.qasm";
    auto file = classicControls;

    qc::QuantumComputation qc(file);

    CHECK_NOTHROW(ConstantPropagation::propagate(qc, 3));
}