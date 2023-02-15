//
// Created by zuchna on 2/13/23.
//
#include "TestUtils.hpp"

Catch::Generators::GeneratorWrapper<fs::path> qasmFile(QASMFileGenerator::SIZE s) {
    return {
            new QASMFileGenerator(s)
    };
}

QASMFileGenerator::QASMFileGenerator(QASMFileGenerator::SIZE s) {
    switch (s) {
        case SMALL:
            unused = findQASMFiles("small");
            break;
        case MEDIUM:
            unused = findQASMFiles("medium");
            break;
        case LARGE:
            unused = findQASMFiles("large");
            break;
        case ALL:
            std::vector<fs::path> small = findQASMFiles("small");
            std::vector<fs::path> medium = findQASMFiles("medium");
            std::vector<fs::path> large = findQASMFiles("large");

            unused.reserve(small.size() + medium.size() + large.size());
            unused.insert(unused.end(), small.begin(), small.end());
            unused.insert(unused.end(), medium.begin(), medium.end());
            unused.insert(unused.end(), large.begin(), large.end());
            break;
    }

    UNSCOPED_INFO("Number of Files found: " + std::to_string(unused.size()));
}

bool QASMFileGenerator::next() {
    if (unused.empty())
        return false;
    unused.pop_back();
    return !unused.empty();
}

std::string QASMFileGenerator::stringifyImpl() const {
    return unused.back().string();
}

std::vector<fs::path> QASMFileGenerator::findQASMFiles(const std::string &subfolder) {
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
