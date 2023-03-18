#include "TestUtils.hpp"
#include "CircuitOptimizer.hpp"
#include <chrono>
#include <fstream>

#include <catch2/reporters/catch_reporter_registrars.hpp>

namespace c = std::chrono;
using tp = std::chrono::steady_clock::time_point;

#define RUNTIME_HEADER "commit;file;maxNAmpls;eps;parseTime;nQubits;nOpsStart;flattenTime;nOpsInlined;propagateTime;nOpsAfterProp;wasTop"
#define REDUCTION_HEADER "file;type;iBf;iAf;ctrBf;ctrAf;targets"

#define BENCHMARK_FOLDER "..//benchmark-results"

#define COMPARE true

#define RUNTIME_OUTPUT BENCHMARK_FOLDER "runtime-"
#define REDUCTION_OUTPUT BENCHMARK_FOLDER "reduction"

void runBenchmark(qc::QuantumComputation &qc, size_t maxNAmpls, std::ostream &out) {
    tp start, end;
    long long dur;

    start = c::steady_clock::now();
    qc::CircuitOptimizer::flattenOperations(qc);
    end = c::steady_clock::now();
    dur = c::duration_cast<c::microseconds>(end - start).count();

    //flattenTime,nOpsAfterInline
    out << ";" << dur << ";" << qc.getNops();

    start = c::steady_clock::now();
    auto ut = ConstantPropagation::propagate(qc, maxNAmpls);
    end = c::steady_clock::now();
    dur = c::duration_cast<c::microseconds>(end - start).count();

    //propagateTime,nOpsAfterPropagate
    out << ";" << dur << ";" << qc.getNops();

    size_t wasTop = 0;
    for (auto &qs: *ut) {
        if (qs.isTop()) {
            wasTop++;
        }
    }

    //wasTop
    out << ";" << wasTop << std::endl;
    out.flush();
}

void compareQcs(const fs::path &file, qc::QuantumComputation &before, qc::QuantumComputation &after, std::ostream &s) {
    auto beforeIt = before.begin();
    auto afterIt = after.begin();

    size_t beforeIndex = 0;
    size_t afterIndex = 0;

    while (beforeIt != before.end()) {
        //Check if the operations are the same
        if (beforeIt->get()->getType() == afterIt->get()->getType() && //Same Type
            beforeIt->get()->getTargets() == afterIt->get()->getTargets() && //Same Targets
            //Controls are subset
            std::includes(beforeIt->get()->getControls().begin(), beforeIt->get()->getControls().end(),
                          afterIt->get()->getControls().begin(), afterIt->get()->getControls().end(),
                          [](auto a, auto b) {
                              return a == b;
                          })) {
            //Same Gate
            if (beforeIt->get()->getNcontrols() > afterIt->get()->getNcontrols()) {
                //Something was optimized
                //fileName
                s << file.string() << ";"
                  //type
                  << qc::toString(beforeIt->get()->getType()) << ";"
                  //beforeIndex
                  << beforeIndex << ";"
                  //afterIndex
                  << afterIndex << ";"
                  //BeforeControls
                  << "[" << std::accumulate(beforeIt->get()->getControls().begin(),
                                            beforeIt->get()->getControls().end(),
                                            std::string(), [](const auto &a, const qc::Control &b) {
                            return a + std::to_string(b.qubit) + ",";
                        }) << "];"
                  //afterControls
                  << "[" << std::accumulate(afterIt->get()->getControls().begin(),
                                            afterIt->get()->getControls().end(),
                                            std::string(), [](const auto &a, const qc::Control &b) {
                            return a + std::to_string(b.qubit) + ",";
                        }) << "];"
                  //Targets
                  << "[" << std::accumulate(beforeIt->get()->getTargets().begin(),
                                            beforeIt->get()->getTargets().end(),
                                            std::string(), [](const auto &a, const auto &b) {
                            return a + std::to_string(b) + ",";
                        }) << "]\n";
            }

            afterIt++;
            afterIndex++;
        } else {
            //Not the same gate. Something was removed
            s << file.string() << ";"
              //type
              << qc::toString(beforeIt->get()->getType()) << ";"
              //beforeIndex
              << beforeIndex << ";"
              //afterIndex
              << "-1;"
              //BeforeControls
              << "[" << std::accumulate(beforeIt->get()->getControls().begin(),
                                        beforeIt->get()->getControls().end(),
                                        std::string(), [](const auto &a, const qc::Control &b) {
                        return a + std::to_string(b.qubit) + ",";
                    }) << "];"
              //afterControls
              << "[];"
              //Targets
              << "[" << std::accumulate(beforeIt->get()->getTargets().begin(),
                                        beforeIt->get()->getTargets().end(),
                                        std::string(), [](const auto &a, const auto &b) {
                        return a + std::to_string(b) + ",";
                    }) << "]\n";
        }

        beforeIt++;
        beforeIndex++;
    }
}

//2:30h on lxhalle
TEST_CASE("Test Circuit Performance", "[!benchmark]") {
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_c);

    char dateTime[80];
    std::setlocale(LC_TIME, "C");
    std::strftime(dateTime, 80, "%Y-%m-%d-%H-%M-%S", &now_tm);

    fs::path benchmarkFolder = BENCHMARK_FOLDER;
    create_directories(benchmarkFolder);

    std::string runtimeFileName = BENCHMARK_FOLDER "//runtime-" + std::string(dateTime) + ".csv";
    std::cout << "Writing to:" << runtimeFileName << std::endl;

    std::ofstream runtimeOut(runtimeFileName, std::ios::out | std::ios::trunc);
    runtimeOut << RUNTIME_HEADER << std::endl;

    std::ofstream compareOut;

    if (COMPARE) {
        std::string compareFileName = BENCHMARK_FOLDER "//compare-" + std::string(dateTime) + ".csv";
        compareOut.open(compareFileName, std::ios::out | std::ios::trunc);
        compareOut << REDUCTION_HEADER << std::endl;
    }

    auto fileGen = qasmFile(QASMFileGenerator::ALL);

    size_t i = 0;
    size_t limit = 250;

    size_t maxNAmpls = 1024;
    double eps = 0.0;

    while (fileGen.next() && i++ < limit) {
        const fs::path &file = fileGen.get();

        now_c = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        now_tm = *std::localtime(&now_c);
        std::strftime(dateTime, 80, "%H:%M:%S", &now_tm);

        std::cout << std::string(dateTime) << ": " << file.string();

        runtimeOut << GIT_COMMIT_HASH << ";" << file.string() << ";" << maxNAmpls << ";" << eps;

        tp start = c::steady_clock::now();
        qc::QuantumComputation qc;

        try {
            qc = qc::QuantumComputation(file.string());
        } catch (std::exception &e) {
            runtimeOut << ", qfr threw an exception while importing: " << e.what() << "\n";
            continue;
        }

        tp end = c::steady_clock::now();
        long long dur = c::duration_cast<c::microseconds>(end - start).count();

        std::cout << ", nOps=" << qc.getNops();
        std::cout.flush();

        //parseTime,nQubits,nOpsStart
        runtimeOut << ";" << dur << ";" << qc.getNqubits() << ";" << qc.getNops();

        qc::QuantumComputation before = qc.clone();

        runBenchmark(qc, maxNAmpls, runtimeOut);
        runtimeOut.flush();

        if (COMPARE) {
            compareQcs(file, before, qc, compareOut);
            compareOut.flush();
        }

        end = c::steady_clock::now();
        dur = c::duration_cast<c::microseconds>(end - start).count();
        std::cout << ", done in " << static_cast<double>(dur) / 1e6 << "s" << std::endl;
    }

    compareOut.close();
    runtimeOut.close();

    now_c = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    now_tm = *std::localtime(&now_c);
    std::strftime(dateTime, 80, "%H:%M:%S", &now_tm);
    std::cout << "Finished at " << std::string(dateTime) << std::endl;
}
