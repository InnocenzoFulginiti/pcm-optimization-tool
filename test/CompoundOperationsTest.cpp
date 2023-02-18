//
// Created by zuchna on 2/17/23.
//

#include "TestUtils.hpp"

TEST_CASE("Try Files with Compound Gates") {
    std::string fileWithCompounds = GENERATE(
            QASM_Bench_Path "/small/wstate_n3/wstate_n3.qasm",
            QASM_Bench_Path "/small/ipea_n2/ipea_n2.qasm",
            QASM_Bench_Path "/small/pea_n5/pea_n5.qasm",
            QASM_Bench_Path "/small/adder_n10/adder_n10.qasm",
            QASM_Bench_Path "/large/dnn_n33/dnn_n33.qasm",
            QASM_Bench_Path "/large/qugan_n39/qugan_n39.qasm"
            );

    INFO("File: " + fileWithCompounds);

    qc::QuantumComputation before(fileWithCompounds);
    qc::QuantumComputation after{};

    after = CompoundGateInliner().optimize(before);

    for(auto const& op : after) {
        CHECK(!op->isCompoundOperation());
    }

    std::stringstream beforeStr;
    before.dumpOpenQASM(beforeStr);

    INFO("Before: \n" + beforeStr.str());

    std::stringstream afterStr;
    after.dumpOpenQASM(afterStr);

    //Use that dumpOpenQASM also replaces compound gates
    CHECK(beforeStr.str() == afterStr.str());
}