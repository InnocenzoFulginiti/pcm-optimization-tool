#pragma once

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/generators/catch_generators_random.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_approx.hpp>

#define CIRCUITS_PATH "../test/circuits/"
#define QASM_Bench_Path CIRCUITS_PATH "QASMBench"
#include <filesystem>
#include <string>

#include "QuantumComputation.hpp"
#include "Definitions_qcprop.hpp"
#include "UnionTable.hpp"
#include "Definitions.hpp"
#include "ConstantPropagation.hpp"
#include "MatrixGenerator.hpp"

#define CHECK_MESSAGE(cond, msg) do { INFO(msg); CHECK(cond); } while((void)0, 0)

namespace fs = std::filesystem;

class QASMFileGenerator : public Catch::Generators::IGenerator<fs::path> {
    std::vector<fs::path> unused;

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

private:
    bool next() override;

    std::string stringifyImpl() const override;
    static std::vector<fs::path> findQASMFiles(const std::string& subfolder);
};

Catch::Generators::GeneratorWrapper<fs::path> qasmFile(QASMFileGenerator::SIZE s);