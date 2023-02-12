#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators_adapters.hpp"
#include "catch2/generators/catch_generators_random.hpp"

#include <filesystem>
#include <string>
#include <iostream>

#include "QuantumComputation.hpp"
#include "ConstantPropagation.hpp"

#define QASM_Bench_Path "../test/circuits/QASMBench"

namespace fs = std::filesystem;

std::vector<fs::path> QASMFiles(std::string subfolder) {
    auto smallFolder = std::filesystem::path((std::string(QASM_Bench_Path) + "/" + subfolder));
    std::vector<fs::path> foundQASMFiles{};
    for (auto &f: fs::directory_iterator(smallFolder)) {
        if (f.is_directory()) {
            for (auto &q: fs::directory_iterator(f)) {
                if (q.is_regular_file()) {
                    if (q.path().extension() == ".qasm")
                        foundQASMFiles.emplace_back(q);
                }
            }
        }
    }

    return foundQASMFiles;
}

class QASMFileGenerator : public Catch::Generators::IGenerator<fs::path> {
    std::vector<fs::path> unused;

public:
    enum SIZE{
        SMALL,
        MEDIUM,
        LARGE,
        ALL
    };
    const fs::path &get() const override {
        return unused.back();
    }

    explicit QASMFileGenerator(SIZE s) {
        switch (s) {
            case SMALL:
                unused = QASMFiles("small");
                break;
            case MEDIUM:
                unused = QASMFiles("medium");
                break;
            case LARGE:
                unused = QASMFiles("large");
            case ALL:
                std::vector<fs::path> small = QASMFiles("small");
                std::vector<fs::path> medium = QASMFiles("medium");
                std::vector<fs::path> large = QASMFiles("large");

                unused.reserve(small.size() + medium.size() + large.size());
                unused.insert(unused.end(), small.begin(), small.end());
                unused.insert(unused.end(), medium.begin(), medium.end());
                unused.insert(unused.end(), large.begin(), large.end());

        }
    }

private:
    bool next() override {
        if (unused.empty())
            return false;
        unused.pop_back();
        return !unused.empty();
    }

    std::string stringifyImpl() const override {
        return unused.back().string();
    }
};

Catch::Generators::GeneratorWrapper<fs::path> qasmFile(QASMFileGenerator::SIZE s) {
    return {
            new QASMFileGenerator(s)
            // Another possibility:
            // Catch::Detail::make_unique<RandomIntGenerator>(low, high)
    };
}

TEST_CASE("Run Constant Propagation on small QASM Files with small MAX_AMPLITUDES") {
    auto p = GENERATE(take(100, qasmFile(QASMFileGenerator::SIZE::ALL)));

    INFO("File: " + p.string());
    qc::QuantumComputation qc(p);

    CHECK_NOTHROW(ConstantPropagation::propagate(qc, 3));
}

TEST_CASE("Try specific file") {
    //Special Files:
    auto fileWithCompoundGates = "../test/circuits/QASMBench/small/wstate_n3/wstate_n3.qasm";

    auto file = "../test/circuits/QASMBench/small/adder_n10/adder_n10_transpiled.qasm";

    qc::QuantumComputation qc(file);

    CHECK_NOTHROW(ConstantPropagation::propagate(qc, 3));
}