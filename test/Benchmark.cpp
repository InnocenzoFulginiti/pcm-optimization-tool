#include "TestUtils.hpp"
#include "CircuitOptimizer.hpp"
#include <chrono>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>
#include <catch2/catch_test_case_info.hpp>


class testRunListener : public Catch::EventListenerBase {
public:
    using Catch::EventListenerBase::EventListenerBase;

    void testCaseStarting(Catch::TestCaseInfo const &info) override {
        if (info.name == "Test Circuit Performance") {
            std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
            std::time_t now_c = std::chrono::system_clock::to_time_t(now);
            std::tm now_tm = *std::localtime(&now_c);

            char buf[80];
            std::setlocale(LC_TIME, "C");
            std::strftime(buf, 80, "%T", &now_tm);

            std::cout << "Test Started at: " << buf << std::endl;
            std::cout
                    << "fileName,parseTime,nOpsStart,flattenTime,nOpsAfterInline,propagateTime,nOpsAfterPropagate,wasTop"
                    << std::endl;
        }
    }
};

CATCH_REGISTER_LISTENER(testRunListener);

namespace c = std::chrono;
using tp = std::chrono::steady_clock::time_point;

//Took about 1:40h in my Laptop
TEST_CASE("Test Circuit Performance", "[!benchmark]") {
    fs::path file = GENERATE(take(240, qasmFile(QASMFileGenerator::ALL)));

    tp start, end;
    long dur;

    //fileName
    std::cout << file.string();

    start = c::steady_clock::now();
    qc::QuantumComputation qc(file.string());
    end = c::steady_clock::now();
    dur = c::duration_cast<c::microseconds>(end - start).count();

    //parseTime,nOpsStart
    std::cout << "," << dur << "," << qc.getNops();
    std::cout.flush();

    start = c::steady_clock::now();
    qc::CircuitOptimizer::flattenOperations(qc);
    end = c::steady_clock::now();
    dur = c::duration_cast<c::microseconds>(end - start).count();

    //flattenTime,nOpsAfterInline
    std::cout << "," << dur << "," << qc.getNops();
    std::cout.flush();

    start = c::steady_clock::now();
    auto [newQC, ut] = ConstantPropagation::propagate(qc, 1024);
    end = c::steady_clock::now();
    dur = c::duration_cast<c::microseconds>(end - start).count();

    //propagateTime,nOpsAfterPropagate
    std::cout << "," << dur << "," << newQC.getNops();

    size_t wasTop = 0;
    for (auto &qs: *ut) {
        if (qs.isTop()) {
            wasTop++;
        }
    }

    //wasTop
    std::cout << "," << wasTop << std::endl;
}
