//
// Created by zuchna on 2/13/23.
//

#pragma once

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/generators/catch_generators_random.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_approx.hpp>

#define QASM_Bench_Path "../test/circuits/QASMBench"
#include <filesystem>
#include <string>

#include "UnionTable.hpp"
#include "Definitions.hpp"
#include "QuantumComputation.hpp"
#include "ConstantPropagation.hpp"

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
    std::vector<fs::path> findQASMFiles(const std::string& subfolder);
};

Catch::Generators::GeneratorWrapper<fs::path> qasmFile(QASMFileGenerator::SIZE s);