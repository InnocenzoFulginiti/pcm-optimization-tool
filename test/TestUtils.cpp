#include "TestUtils.hpp"

#include <memory>

Catch::Generators::GeneratorWrapper<fs::path> qasmFile(QASMFileGenerator::SIZE s) {
    return {
            new QASMFileGenerator(s)
    };
}

void approx(const Complex &expected, const Complex &actual, double epsilon) {
    CAPTURE(epsilon);
    double realDiff = std::abs(expected.real() - actual.real());
    double imagDiff = std::abs(expected.imag() - actual.imag());

    CAPTURE(realDiff, imagDiff);
    CHECK(realDiff <= epsilon);
    CHECK(imagDiff <= epsilon);
}

void approxQubitState(const std::shared_ptr<QubitState> &expected, const std::shared_ptr<QubitState> &actual,
                      double epsilon) {
    CAPTURE(epsilon);
    for (size_t key = 0; key < (static_cast<size_t>(1) << expected->getNQubits()); key++) {
        CAPTURE(key);
        approx((*expected)[key], (*actual)[key], epsilon);
    }
}

std::shared_ptr<UnionTable> generateRandomUnionTable(size_t nQubits, long long seed) {
    if (nQubits == 0)
        return std::make_shared<UnionTable>(0);

    UNSCOPED_INFO("Random UnionTable seed: " << seed);

    std::srand(static_cast<unsigned int>(seed));

    std::shared_ptr<UnionTable> table = std::make_shared<UnionTable>(nQubits);

    //Create a random partitioning of the qubits
    std::vector<std::vector<size_t>> partitioning(nQubits);

    for (size_t i = 0; i < nQubits; i++) {
        size_t j = 0;
        double r = (rand() % 1000) / 1000.0;
        while (r > 0.5 && j < partitioning.size() - 1) {
            j++;
            r = (rand() % 1000) / 1000.0;
        }
        partitioning[j].push_back(i);
    }

    //Counter so seed is different for each state
    int i = 0;
    for (const std::vector<size_t> &partition: partitioning) {
        QubitStateOrTop state = generateRandomStateOrTop(partition.size(), 0.8 / static_cast<double>(partition.size()),
                                                         seed + i++);
        for (const size_t qubit: partition) {
            (*table)[qubit] = state;
        }
    }

    return table;
}

std::shared_ptr<QubitState> generateRandomState(size_t nQubits, long long seed) {
    return generateRandomStateOrTop(nQubits, 0.0, seed).getQubitState();
}

QubitStateOrTop generateRandomStateOrTop(size_t nQubits, double chanceForTop, long long seed) {
    if (nQubits == 0)
        return std::make_shared<QubitState>(0);

    std::srand(static_cast<unsigned int>(seed));

    double r = rand() % 1000 / 1000.0;
    if (r < chanceForTop)
        return TOP::T;

    auto state = std::make_shared<QubitState>(nQubits);
    state->clear();

    for (size_t key = 0; key < (static_cast<size_t>(1) << nQubits); key++) {
        //Only values for 2/3 of the keys
        r = rand() % 1000 / 1000.0;
        if (r < 2.0 / 3.0 || state->size() == 0) {
            double real = (rand() % 1000 / 5000.0) + 1.0;
            double imag = (rand() % 1000 / 5000.0) + 1.0;
            (*state)[BitSet(nQubits, key)] = Complex(real, imag);
        }
    }

    state->normalize();

    return state;
}

void compareUnitTableToState(const std::shared_ptr<UnionTable> &ut,
                             const std::vector<std::pair<size_t, Complex>> &expectedState) {
    auto actualTable = ut->clone();

    auto expected = expectedState;
    std::sort(expected.begin(), expected.end(), [](const auto &a, const auto &b) { return a.first < b.first; });

    INFO("Actual before combination:\n" + actualTable->to_string());

    //Combine all states for easier comparison
    for (size_t i = 1; i < ut->getNQubits(); i++) {
        actualTable->combine(0, i);
    }

    REQUIRE((*actualTable)[0].isQubitState());
    auto actualState = (*actualTable)[0].getQubitState();

    INFO("Actual:\t" + actualState->to_string());
    INFO("Expected:\t" + QubitState::fromVector(expected, ut->getNQubits())->to_string());

    bool globalPhaseSet = false;
    double globalPhase = 0;

    for (size_t key = 0; key < (static_cast<size_t>(1) << ut->getNQubits()); key++) {
        Complex expectedValue = 0;
        if (expected[0].first == key) {
            expectedValue = expected[0].second;
            expected.erase(expected.begin());
        }

        INFO(std::to_string(key) + " (0b" + BitSet(ut->getNQubits(), key).to_string() + ")");
        INFO("Expected Value:\t" + expectedValue.to_string() + " = mag: " + std::to_string(expectedValue.norm()) +
             " arg: " + std::to_string(expectedValue.arg()) + " +global phase = " +
             std::to_string(expectedValue.arg() + globalPhase));
        Complex actualValue = (*actualState)[key];
        INFO("Actual Value:\t" + actualValue.to_string() + " = mag: " + std::to_string(actualValue.norm()) + " arg: " +
             std::to_string(actualValue.arg()));

        if (!expectedValue.isZero() && !globalPhaseSet) {
            globalPhase = actualValue.arg() - expectedValue.arg();
            globalPhaseSet = true;
        }

        CAPTURE(globalPhase, globalPhaseSet);

        Complex expectedPhased = expectedValue * Complex(std::cos(globalPhase), std::sin(globalPhase));
        CAPTURE(expectedPhased);

        approx(expectedPhased, actualValue, 1e-2);
    }

}

void approxUnionTable(const std::shared_ptr<UnionTable> &expected, const std::shared_ptr<UnionTable> &actual,
                      double epsilon) {
    REQUIRE(expected->getNQubits() == actual->getNQubits());

    for (size_t i = 0; i < expected->getNQubits(); i++) {
        if (expected->isTop(i)) {
            REQUIRE(actual->isTop(i));
        } else {
            REQUIRE(!actual->isTop(i));
            approxQubitState((*expected)[i].getQubitState(), (*actual)[i].getQubitState(), epsilon);
        }
    }
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

CircuitMetrics::CircuitMetrics(const fs::path &pathOfQasmFile) {
    fileName = pathOfQasmFile.string();

    fs::path readme = pathOfQasmFile.parent_path().append("README.md");
    std::ifstream file(readme);
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("Qubit Count") != std::string::npos) {
            qubitCount = std::stoi(line.substr(line.find(':') + 1));
        } else if (line.find("Circuit Depth") != std::string::npos) {
            circuitDepth = std::stoi(line.substr(line.find(':') + 1));
        } else if (line.find("Circuit Width") != std::string::npos) {
            circuitWidth = std::stoi(line.substr(line.find(':') + 1));
        } else if (line.find("Dual Gate Count") != std::string::npos) {
            dualGateCount = std::stoi(line.substr(line.find(':') + 1));
        } else if (line.find("Gate Count") != std::string::npos) {
            gateCount = std::stoi(line.substr(line.find(':') + 1));
        } else if (line.find("Retention Lifespan") != std::string::npos) {
            retLife = std::stof(line.substr(line.find(':') + 1));
        } else if (line.find("Gate Density") != std::string::npos) {
            gateDens = std::stof(line.substr(line.find(':') + 1));
        } else if (line.find("Measurement Density") != std::string::npos) {
            measDens = std::stof(line.substr(line.find(':') + 1));
        } else if (line.find("Size Factor") != std::string::npos) {
            sizeFact = std::stof(line.substr(line.find(':') + 1));
        } else if (line.find("Entanglement Variance") != std::string::npos) {
            entangleVar = std::stof(line.substr(line.find(':') + 1));
        } else if (line.find("Communication Supermarq") != std::string::npos) {
            commSuperma = std::stof(line.substr(line.find(':') + 1));
        } else if (line.find("Measurement Supermarq") != std::string::npos) {
            measSuperma = std::stof(line.substr(line.find(':') + 1));
        } else if (line.find("Depth Supermarq") != std::string::npos) {
            depthSuperma = std::stof(line.substr(line.find(':') + 1));
        } else if (line.find("Entanglement Supermarq") != std::string::npos) {
            entangleSuperma = std::stof(line.substr(line.find(':') + 1));
        } else if (line.find("Parallelism Supermarq") != std::string::npos) {
            parallelismSuperma = std::stof(line.substr(line.find(':') + 1));
        } else if (line.find("Liveness Supermarq") != std::string::npos) {
            livenessSuperma = std::stof(line.substr(line.find(':') + 1));
        }
    }

}

[[maybe_unused]] std::string CircuitMetrics::csvHeader() {
    return "File Name,"
           "Qubit Count,"
           "Circuit Depth,"
           "Circuit Width,"
           "Dual Gate Count,"
           "Gate Count,"
           "Retention Lifespan,"
           "Gate Density,"
           "Measurement Density,"
           "Size Factor,"
           "Entanglement Variance,"
           "Communication Supermarq,"
           "Measurement Supermarq,"
           "Depth Supermarq,"
           "Entanglement Supermarq,"
           "Parallelism Supermarq,"
           "Liveness Supermarq";
}

std::string CircuitMetrics::csvLine() const {
    return fileName + ","
           + std::to_string(qubitCount) + ","
           + std::to_string(circuitDepth) + ","
           + std::to_string(circuitWidth) + ","
           + std::to_string(dualGateCount) + ","
           + std::to_string(gateCount) + ","
           + std::to_string(retLife) + ","
           + std::to_string(gateDens) + ","
           + std::to_string(measDens) + ","
           + std::to_string(sizeFact) + ","
           + std::to_string(entangleVar) + ","
           + std::to_string(commSuperma) + ","
           + std::to_string(measSuperma) + ","
           + std::to_string(depthSuperma) + ","
           + std::to_string(entangleSuperma) + ","
           + std::to_string(parallelismSuperma) + ","
           + std::to_string(livenessSuperma);
}
