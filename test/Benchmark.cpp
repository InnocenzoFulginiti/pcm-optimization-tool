#include "TestUtils.hpp"
#include "CircuitOptimizer.hpp"
#include <chrono>

//This takes about 1:40h in my Laptop
TEST_CASE("Test Circuit Performance", "[!benchmark]") {
    fs::path file = GENERATE(take(240, qasmFile(QASMFileGenerator::ALL)));

    qc::QuantumComputation qc(file.string());

    CircuitMetrics metrics(file);

    size_t before = qc.getNops();
    qc::CircuitOptimizer::flattenOperations(qc);
    size_t beforeAfterInline = qc.getNops();

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    auto [newQC, ut] = ConstantPropagation::propagate(qc, 1024);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    size_t after = newQC.getNops();

    size_t wasTop = 0;
    for (auto &qs: *ut) {
        if (qs.isTop()) {
            wasTop++;
        }
    }

    auto muS = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    double sec = static_cast<double>(muS) / 1000000.0;

    INFO("Done with File: " << file.string() << "\nTook " << sec << " s");
    CHECK(false);

    //std::cout << "File Name, gates before, gates after, no was top, time in propagation [us], before after inline" << std::endl;
    std::cout << file.string() << ","
              << before << ","
              << after << ","
              << wasTop << ","
              << muS << ","
              << beforeAfterInline
              << std::endl;
}
