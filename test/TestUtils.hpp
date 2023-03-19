#pragma once

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/generators/catch_generators_random.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_approx.hpp>

#define CIRCUITS_PATH "../test/circuits/"
#define QASM_Bench_Path CIRCUITS_PATH "QASMBench"

#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH "?"
#endif

#include <filesystem>
#include <string>

#include "QuantumComputation.hpp"
#include "Definitions_qcprop.hpp"
#include "UnionTable.hpp"
#include "Definitions.hpp"
#include "ConstantPropagation.hpp"
#include "MatrixGenerator.hpp"
#include "CircuitOptimizer.hpp"

void approx(const Complex &expected, const Complex &actual, double epsilon = 1e-10);

void
approxQubitState(const std::shared_ptr<QubitState> &expected, const std::shared_ptr<QubitState> &actual,
                 double epsilon = 1e-10);

void
approxUnionTable(const std::shared_ptr<UnionTable> &expected, const std::shared_ptr<UnionTable> &actual,
                 double epsilon = 1e-10);

[[maybe_unused]] [[maybe_unused]] std::shared_ptr<QubitState>
generateRandomState(size_t nQubits,
                    unsigned int seed = static_cast<unsigned int>(std::time(nullptr)));

QubitStateOrTop
generateRandomStateOrTop(size_t nQubits, double chanceForTop = 0.33,
                         unsigned int seed = static_cast<unsigned int>(std::time(nullptr)));

std::shared_ptr<UnionTable>
generateRandomUnionTable(size_t nQubits, double topChance = 0.8,
                         unsigned int seed = static_cast<unsigned int>(std::time(nullptr)));

void compareUnitTableToState(const std::shared_ptr<UnionTable> &ut,
                             const std::vector<std::pair<size_t, Complex>> &expected);

namespace fs = std::filesystem;

class QASMFileGenerator : public Catch::Generators::IGenerator<fs::path> {
public:
    enum SIZE {
        SMALL,
        MEDIUM,
        LARGE,
        ALL
    };

    const fs::path &get() const override {
        return unused.back();
    }

    explicit QASMFileGenerator(SIZE s);

    size_t getSize() const {
        return size;
    }

    bool next() override;

private:
    std::vector<fs::path> unused;
    size_t size;

    std::string stringifyImpl() const override;

    static std::vector<fs::path> findQASMFiles(const std::string &subfolder);
};

Catch::Generators::GeneratorWrapper<fs::path> qasmFile(QASMFileGenerator::SIZE s);

class CircuitMetrics {
public:
    explicit CircuitMetrics(const fs::path &pathOfQasmFile);

    [[maybe_unused]] [[maybe_unused]] [[nodiscard]] std::string csvLine() const;

    [[maybe_unused]] static std::string csvHeader();

private:
    std::string fileName;
    int qubitCount,
            circuitDepth,
            circuitWidth,
            dualGateCount,
            gateCount;
    float retLife,
            gateDens,
            measDens,
            sizeFact,
            entangleVar,
            commSuperma,
            measSuperma,
            depthSuperma,
            entangleSuperma,
            parallelismSuperma,
            livenessSuperma;
};