#include "TestUtils.hpp"

void testIntermediateResults(qc::QuantumComputation &qc,
                             size_t maxAmplitudes,
                             const std::vector<std::pair<size_t, std::vector<std::pair<size_t, Complex>>>> &expectedValues) {

    auto sortedExpectedValues(expectedValues);
    std::sort(sortedExpectedValues.begin(), sortedExpectedValues.end(),
              [](const auto &a, const auto &b) { return a.first < b.first; });

    auto gateIter = qc.begin();
    size_t gateIndex = 0;
    auto currentTable = std::make_shared<UnionTable>(qc.getNqubits());
    auto qcSection = qc.clone();
    qcSection.clear();

    while (gateIter != qc.end()) {
        auto lastGateIter = gateIter;

        qcSection.clear();
        while ((gateIter != qc.end()) && (sortedExpectedValues.empty() || gateIndex < sortedExpectedValues[0].first)) {
            qcSection.emplace_back(gateIter->get()->clone());
            lastGateIter = gateIter;
            gateIter++;
            gateIndex++;
        }

        auto [opt, table] = ConstantPropagation::propagate(qcSection, maxAmplitudes, currentTable);
        currentTable = table;

        INFO("Last Gate was " + lastGateIter->get()->getName() + " with target " +
             std::to_string(lastGateIter->get()->getTargets().begin()[0]));
        INFO("After " + std::to_string(gateIndex) + " gates:");
        if(sortedExpectedValues.empty()) {
            break;
        }
        auto actualTable = currentTable->clone();

        auto expected(sortedExpectedValues[0].second);
        std::sort(expected.begin(), expected.end(),[](const auto &a, const auto &b) { return a.first < b.first; });

        INFO("Actual before combination:\n" + actualTable->to_string());

        //Combine all states for easier comparison
        for (size_t i = 1; i < qc.getNqubits(); i++) {
            actualTable->combine(0, i);
        }

        REQUIRE((*actualTable)[0].isQubitState());
        auto actualState = (*actualTable)[0].getQubitState();

        INFO("Actual:\t" + actualState->to_string());
        INFO("Expected:\t" + QubitState::fromVector(expected, qc.getNqubits())->to_string());

        for (size_t key = 0; key < (1 << qc.getNqubits()); key++) {
            Complex expectedValue = 0;
            if(expected[0].first == key) {
                expectedValue = expected[0].second;
                expected.erase(expected.begin());
            }
            INFO("Key: " + std::to_string(key));
            INFO("Expected Value:\t" + expectedValue.to_string() + ", Mag: " + std::to_string(expectedValue.norm()));
            Complex actualValue = (*actualState)[key];
            INFO("Actual Value:\t" + actualValue.to_string() + ", Mag: " + std::to_string(actualValue.norm()));

            REQUIRE(actualValue.norm() == Catch::Approx(expectedValue.norm()).margin(1e-3));
        }

        sortedExpectedValues.erase(sortedExpectedValues.begin());
    }
}


TEST_CASE("Constant Propagation on small Files, multiple maxAmplitudes") {
    auto p = GENERATE(take(20, qasmFile(QASMFileGenerator::SIZE::SMALL)));
    auto maxAmplitude = GENERATE(1, 10, 40);

    INFO("File: " + p.string() + "Max Amplitude: " + std::to_string(maxAmplitude));

    qc::QuantumComputation qc(p.string());
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

    REQUIRE_FALSE(q0.get() == q1.get());
    CHECK((*q0)[0] == 1);
    CHECK((*q0)[1] == 0);
    CHECK((*q1)[0] == qc::SQRT_2_2);
    CHECK((*q1)[1] == qc::SQRT_2_2);
}

TEST_CASE("Try File with Reset") {
    auto fileWithReset = "../test/circuits/QASMBench/small/ipea_n2/ipea_n2.qasm";
    qc::QuantumComputation qc(fileWithReset);

    CHECK_NOTHROW(ConstantPropagation::propagate(qc, 3));
}

TEST_CASE("Check Intermediate Results adder_n4") {
    auto path = QASM_Bench_Path "/small/adder_n4/adder_n4.qasm";
    INFO(std::string("Working on file: ") + path);
    qc::QuantumComputation qc(path);

    testIntermediateResults(qc, 20, {
            //After n steps, have the following amplitudes
            {1,  {{0b0001, 1}}},
            {2,  {{0b0011, 1}}},
            {3,  {{0b0011, qc::SQRT_2_2},             {0b1011, qc::SQRT_2_2}}},
            {8,  {{0b0011, Complex(0, qc::SQRT_2_2)}, {0b1011, Complex(0.5, 0.5)}}},
            {9,  {{0b0001, Complex(0, qc::SQRT_2_2)}, {0b1001, Complex(0.5, 0.5)}}},
            {10, {{0b0001, Complex(0, qc::SQRT_2_2)}, {0b1001, Complex(0.5, 0.5)}}},
            {11, {{0b0001, Complex(0, qc::SQRT_2_2)}, {0b1000, Complex(0.5, 0.5)}}},
            {12, {{0b0001, Complex(0, qc::SQRT_2_2)}, {0b1000, Complex(0.5, 0.5)}}},
            {13, {{0b0011, Complex(0, qc::SQRT_2_2)}, {0b1000, Complex(0.5, 0.5)}}},
            {14, {{0b0011, Complex(0, qc::SQRT_2_2)}, {0b1000, Complex(0.5, 0.5)}}},
            {23, {{0b1001, 1}}}
    });
}

TEST_CASE("Check Intermediate Results qaoa_n6") {
    qc::QuantumComputation qc(QASM_Bench_Path "/small/qaoa_n6/qaoa_n6.qasm");
    testIntermediateResults(qc, ((size_t) 1) << 7, {
            {6, {{0, Complex(0.125,0)},{1, Complex(0.125,0)},{2, Complex(0.125,0)},{3, Complex(0.125,0)},{4, Complex(0.125,0)},{5, Complex(0.125,0)},{6, Complex(0.125,0)},{7, Complex(0.125,0)},{8, Complex(0.125,0)},{9, Complex(0.125,0)},{10, Complex(0.125,0)},{11, Complex(0.125,0)},{12, Complex(0.125,0)},{13, Complex(0.125,0)},{14, Complex(0.125,0)},{15, Complex(0.125,0)},{16, Complex(0.125,0)},{17, Complex(0.125,0)},{18, Complex(0.125,0)},{19, Complex(0.125,0)},{20, Complex(0.125,0)},{21, Complex(0.125,0)},{22, Complex(0.125,0)},{23, Complex(0.125,0)},{24, Complex(0.125,0)},{25, Complex(0.125,0)},{26, Complex(0.125,0)},{27, Complex(0.125,0)},{28, Complex(0.125,0)},{29, Complex(0.125,0)},{30, Complex(0.125,0)},{31, Complex(0.125,0)},{32, Complex(0.125,0)},{33, Complex(0.125,0)},{34, Complex(0.125,0)},{35, Complex(0.125,0)},{36, Complex(0.125,0)},{37, Complex(0.125,0)},{38, Complex(0.125,0)},{39, Complex(0.125,0)},{40, Complex(0.125,0)},{41, Complex(0.125,0)},{42, Complex(0.125,0)},{43, Complex(0.125,0)},{44, Complex(0.125,0)},{45, Complex(0.125,0)},{46, Complex(0.125,0)},{47, Complex(0.125,0)},{48, Complex(0.125,0)},{49, Complex(0.125,0)},{50, Complex(0.125,0)},{51, Complex(0.125,0)},{52, Complex(0.125,0)},{53, Complex(0.125,0)},{54, Complex(0.125,0)},{55, Complex(0.125,0)},{56, Complex(0.125,0)},{57, Complex(0.125,0)},{58, Complex(0.125,0)},{59, Complex(0.125,0)},{60, Complex(0.125,0)},{61, Complex(0.125,0)},{62, Complex(0.125,0)},{63, Complex(0.125,0)}}},
            {8, {{0, Complex(-0.121,0.033)},{1, Complex(0.125,0)},{2, Complex(0.125,0)},{3, Complex(-0.121,0.033)},{4, Complex(-0.121,0.033)},{5, Complex(0.125,0)},{6, Complex(0.125,0)},{7, Complex(-0.121,0.033)},{8, Complex(-0.121,0.033)},{9, Complex(0.125,0)},{10, Complex(0.125,0)},{11, Complex(-0.121,0.033)},{12, Complex(-0.121,0.033)},{13, Complex(0.125,0)},{14, Complex(0.125,0)},{15, Complex(-0.121,0.033)},{16, Complex(-0.121,0.033)},{17, Complex(0.125,0)},{18, Complex(0.125,0)},{19, Complex(-0.121,0.033)},{20, Complex(-0.121,0.033)},{21, Complex(0.125,0)},{22, Complex(0.125,0)},{23, Complex(-0.121,0.033)},{24, Complex(-0.121,0.033)},{25, Complex(0.125,0)},{26, Complex(0.125,0)},{27, Complex(-0.121,0.033)},{28, Complex(-0.121,0.033)},{29, Complex(0.125,0)},{30, Complex(0.125,0)},{31, Complex(-0.121,0.033)},{32, Complex(-0.121,0.033)},{33, Complex(0.125,0)},{34, Complex(0.125,0)},{35, Complex(-0.121,0.033)},{36, Complex(-0.121,0.033)},{37, Complex(0.125,0)},{38, Complex(0.125,0)},{39, Complex(-0.121,0.033)},{40, Complex(-0.121,0.033)},{41, Complex(0.125,0)},{42, Complex(0.125,0)},{43, Complex(-0.121,0.033)},{44, Complex(-0.121,0.033)},{45, Complex(0.125,0)},{46, Complex(0.125,0)},{47, Complex(-0.121,0.033)},{48, Complex(-0.121,0.033)},{49, Complex(0.125,0)},{50, Complex(0.125,0)},{51, Complex(-0.121,0.033)},{52, Complex(-0.121,0.033)},{53, Complex(0.125,0)},{54, Complex(0.125,0)},{55, Complex(-0.121,0.033)},{56, Complex(-0.121,0.033)},{57, Complex(0.125,0)},{58, Complex(0.125,0)},{59, Complex(-0.121,0.033)},{60, Complex(-0.121,0.033)},{61, Complex(0.125,0)},{62, Complex(0.125,0)},{63, Complex(-0.121,0.033)}}},
            {10, {{0, Complex(-0.246,0)},{1, Complex(0,0.033)},{2, Complex(0,0.033)},{3, Complex(-0.004,0)},{4, Complex(-0.246,0)},{5, Complex(0,0.033)},{6, Complex(0,0.033)},{7, Complex(-0.004,0)},{8, Complex(-0.246,0)},{9, Complex(0,0.033)},{10, Complex(0,0.033)},{11, Complex(-0.004,0)},{12, Complex(-0.246,0)},{13, Complex(0,0.033)},{14, Complex(0,0.033)},{15, Complex(-0.004,0)},{16, Complex(-0.246,0)},{17, Complex(0,0.033)},{18, Complex(0,0.033)},{19, Complex(-0.004,0)},{20, Complex(-0.246,0)},{21, Complex(0,0.033)},{22, Complex(0,0.033)},{23, Complex(-0.004,0)},{24, Complex(-0.246,0)},{25, Complex(0,0.033)},{26, Complex(0,0.033)},{27, Complex(-0.004,0)},{28, Complex(-0.246,0)},{29, Complex(0,0.033)},{30, Complex(0,0.033)},{31, Complex(-0.004,0)},{32, Complex(-0.246,0)},{33, Complex(0,0.033)},{34, Complex(0,0.033)},{35, Complex(-0.004,0)},{36, Complex(-0.246,0)},{37, Complex(0,0.033)},{38, Complex(0,0.033)},{39, Complex(-0.004,0)},{40, Complex(-0.246,0)},{41, Complex(0,0.033)},{42, Complex(0,0.033)},{43, Complex(-0.004,0)},{44, Complex(-0.246,0)},{45, Complex(0,0.033)},{46, Complex(0,0.033)},{47, Complex(-0.004,0)},{48, Complex(-0.246,0)},{49, Complex(0,0.033)},{50, Complex(0,0.033)},{51, Complex(-0.004,0)},{52, Complex(-0.246,0)},{53, Complex(0,0.033)},{54, Complex(0,0.033)},{55, Complex(-0.004,0)},{56, Complex(-0.246,0)},{57, Complex(0,0.033)},{58, Complex(0,0.033)},{59, Complex(-0.004,0)},{60, Complex(-0.246,0)},{61, Complex(0,0.033)},{62, Complex(0,0.033)},{63, Complex(-0.004,0)}}},
            {15, {{0, Complex(-0.169,0.023)},{1, Complex(-0.006,0.046)},{2, Complex(-0.023,0.175)},{4, Complex(-0.169,0.023)},{5, Complex(-0.006,0.046)},{6, Complex(-0.023,0.175)},{8, Complex(-0.169,0.023)},{9, Complex(-0.006,0.046)},{10, Complex(-0.023,0.175)},{12, Complex(-0.169,0.023)},{13, Complex(-0.006,0.046)},{14, Complex(-0.023,0.175)},{16, Complex(-0.169,0.023)},{17, Complex(-0.006,0.046)},{18, Complex(-0.023,0.175)},{20, Complex(-0.169,0.023)},{21, Complex(-0.006,0.046)},{22, Complex(-0.023,0.175)},{24, Complex(-0.169,0.023)},{25, Complex(-0.006,0.046)},{26, Complex(-0.023,0.175)},{28, Complex(-0.169,0.023)},{29, Complex(-0.006,0.046)},{30, Complex(-0.023,0.175)},{32, Complex(-0.169,0.023)},{33, Complex(-0.006,0.046)},{34, Complex(-0.023,0.175)},{36, Complex(-0.169,0.023)},{37, Complex(-0.006,0.046)},{38, Complex(-0.023,0.175)},{40, Complex(-0.169,0.023)},{41, Complex(-0.006,0.046)},{42, Complex(-0.023,0.175)},{44, Complex(-0.169,0.023)},{45, Complex(-0.006,0.046)},{46, Complex(-0.023,0.175)},{48, Complex(-0.169,0.023)},{49, Complex(-0.006,0.046)},{50, Complex(-0.023,0.175)},{52, Complex(-0.169,0.023)},{53, Complex(-0.006,0.046)},{54, Complex(-0.023,0.175)},{56, Complex(-0.169,0.023)},{57, Complex(-0.006,0.046)},{58, Complex(-0.023,0.175)},{60, Complex(-0.169,0.023)},{61, Complex(-0.006,0.046)},{62, Complex(-0.023,0.175)}}},
            {270, {{0, Complex(-0.073,0.037)},{1, Complex(0.05,0.087)},{2, Complex(0.05,0.087)},{3, Complex(-0.043,0.098)},{4, Complex(0.05,0.087)},{5, Complex(-0.043,0.098)},{6, Complex(-0.043,0.098)},{7, Complex(0.057,0.063)},{8, Complex(0.05,0.087)},{9, Complex(0.031,0.157)},{10, Complex(-0.029,0.061)},{11, Complex(0.018,0.101)},{12, Complex(0.031,0.157)},{13, Complex(-0.081,0.189)},{14, Complex(0.018,0.101)},{15, Complex(-0.043,0.098)},{16, Complex(0.05,0.087)},{17, Complex(0.031,0.157)},{18, Complex(0.031,0.157)},{19, Complex(-0.081,0.189)},{20, Complex(-0.029,0.061)},{21, Complex(0.018,0.101)},{22, Complex(0.018,0.101)},{23, Complex(-0.043,0.098)},{24, Complex(-0.043,0.098)},{25, Complex(-0.081,0.189)},{26, Complex(0.018,0.101)},{27, Complex(0.031,0.157)},{28, Complex(0.018,0.101)},{29, Complex(0.031,0.157)},{30, Complex(-0.029,0.061)},{31, Complex(0.05,0.087)},{32, Complex(0.05,0.087)},{33, Complex(-0.029,0.061)},{34, Complex(0.031,0.157)},{35, Complex(0.018,0.101)},{36, Complex(0.031,0.157)},{37, Complex(0.018,0.101)},{38, Complex(-0.081,0.189)},{39, Complex(-0.043,0.098)},{40, Complex(-0.043,0.098)},{41, Complex(0.018,0.101)},{42, Complex(0.018,0.101)},{43, Complex(-0.029,0.061)},{44, Complex(-0.081,0.189)},{45, Complex(0.031,0.157)},{46, Complex(0.031,0.157)},{47, Complex(0.05,0.087)},{48, Complex(-0.043,0.098)},{49, Complex(0.018,0.101)},{50, Complex(-0.081,0.189)},{51, Complex(0.031,0.157)},{52, Complex(0.018,0.101)},{53, Complex(-0.029,0.061)},{54, Complex(0.031,0.157)},{55, Complex(0.05,0.087)},{56, Complex(0.057,0.063)},{57, Complex(-0.043,0.098)},{58, Complex(-0.043,0.098)},{59, Complex(0.05,0.087)},{60, Complex(-0.043,0.098)},{61, Complex(0.05,0.087)},{62, Complex(0.05,0.087)},{63, Complex(-0.073,0.037)}}}
    });
}

TEST_CASE("Check Intermediate Results bell_n4") {
    qc::QuantumComputation qc(QASM_Bench_Path "/small/bell_n4/bell_n4.qasm");

    testIntermediateResults(qc, 1 << 4, {
            {5,{{0, Complex(0.327,0)},{1, Complex(0,0.135)},{2, Complex(0.327,0)},{3, Complex(0,0.135)},{4, Complex(0,0.135)},{5, Complex(0.327,0)},{6, Complex(0,0.135)},{7, Complex(0.327,0)},{8, Complex(0.327,0)},{9, Complex(0,0.135)},{10, Complex(0.327,0)},{11, Complex(0,0.135)},{12, Complex(0,0.135)},{13, Complex(0.327,0)},{14, Complex(0,0.135)},{15, Complex(0.327,0)}}},
            {6, {{0, Complex(0.231,0.096)},{1, Complex(0.231,0.096)},{2, Complex(0.231,0.096)},{3, Complex(0.231,0.096)},{4, Complex(-0.231,0.096)},{5, Complex(0.231,0.096)},{6, Complex(-0.231,0.096)},{7, Complex(0.231,0.096)},{8, Complex(0.231,0.096)},{9, Complex(0.231,0.096)},{10, Complex(0.231,0.096)},{11, Complex(0.231,0.096)},{12, Complex(-0.231,0.096)},{13, Complex(0.231,0.096)},{14, Complex(-0.231,0.096)},{15, Complex(0.231,0.096)}}},
          //  {8, {{0, Complex(0.462,0)},{2, Complex(0.462,0)},{5, Complex(0.462,0)},{7, Complex(0.462,0)},{8, Complex(0,0.191)},{10, Complex(0,0.191)},{13, Complex(0,0.191)},{15, Complex(0,0.191)}}},
          //  {10, {{0, Complex(0.462,0)},{2, Complex(0.462,0)},{5, Complex(0.462,0)},{7, Complex(0.462,0)},{9, Complex(0,0.191)},{11, Complex(0,0.191)},{12, Complex(0,0.191)},{14, Complex(0,0.191)}}},
            {11, {{0, Complex(0.427,0)},{1, Complex(-0.073,0)},{2, Complex(0.427,0)},{3, Complex(-0.073,0)},{4, Complex(-0.073,0)},{5, Complex(0.427,0)},{6, Complex(-0.073,0)},{7, Complex(0.427,0)},{8, Complex(0,0.177)},{9, Complex(0,0.177)},{10, Complex(0,0.177)},{11, Complex(0,0.177)},{12, Complex(0,0.177)},{13, Complex(0,0.177)},{14, Complex(0,0.177)},{15, Complex(0,0.177)}}},
          //  {12, {{0, Complex(0.354,0)},{1, Complex(-0.354,0)},{2, Complex(0.354,0)},{3, Complex(-0.354,0)},{4, Complex(0.25,0)},{5, Complex(0.25,0)},{6, Complex(0.25,0)},{7, Complex(0.25,0)},{12, Complex(0,0.25)},{13, Complex(0,0.25)},{14, Complex(0,0.25)},{15, Complex(0,0.25)}}},
          //  {13, {{0, Complex(0.354,0)},{1, Complex(-0.354,0)},{2, Complex(0.354,0)},{3, Complex(-0.354,0)},{4, Complex(0,0.25)},{5, Complex(0,0.25)},{6, Complex(0,0.25)},{7, Complex(0,0.25)},{12, Complex(0.25,0)},{13, Complex(0.25,0)},{14, Complex(0.25,0)},{15, Complex(0.25,0)}}},
            {30, {{0, Complex(0.163,0.394)},{1, Complex(-0.028,0.068)},{2, Complex(0.163,0.068)},{3, Complex(0.163,0.068)},{4, Complex(-0.028,0.068)},{5, Complex(0.163,0.394)},{6, Complex(0.163,0.068)},{7, Complex(0.163,0.068)},{8, Complex(0.299,0.068)},{9, Complex(-0.163,0.259)},{10, Complex(0.163,0.068)},{11, Complex(0.163,0.068)},{12, Complex(-0.163,0.259)},{13, Complex(0.299,0.068)},{14, Complex(0.163,0.068)},{15, Complex(0.163,0.068)}}},
            {33, {{0, Complex(0.231,0.231)},{1, Complex(0.096,0.096)},{2, Complex(0.327,0)},{3, Complex(0,0.135)},{4, Complex(0.096,0.096)},{5, Complex(0.231,0.231)},{6, Complex(0,0.135)},{7, Complex(0.327,0)},{8, Complex(0.327,0)},{9, Complex(0,0.135)},{10, Complex(0.096,0.096)},{11, Complex(0.231,0.231)},{12, Complex(0,0.135)},{13, Complex(0.327,0)},{14, Complex(0.231,0.231)},{15, Complex(0.096,0.096)}}}
    });
}

TEST_CASE("Try file with Classic Controlled gates") {
    auto classicControls = "../test/circuits/QASMBench/small/shor_n5/shor_n5.qasm";
    qc::QuantumComputation qc(classicControls);
    std::pair<qc::QuantumComputation, std::shared_ptr<UnionTable>> res;

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
    auto shor = QASM_Bench_Path "/small/shor_n5/shor_n5.qasm";
    auto file = shor;

    qc::QuantumComputation qc(file);

    CHECK_NOTHROW(ConstantPropagation::propagate(qc, 3));
}