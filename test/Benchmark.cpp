#include "TestUtils.hpp"
#include "CircuitOptimizer.hpp"
#include <chrono>
#include <fstream>

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>

#include <catch2/reporters/catch_reporter_registrars.hpp>

namespace c = std::chrono;
using tp = std::chrono::steady_clock::time_point;

#define RUNTIME_HEADER "file;parseTime;nQubits;nOpsStart;flattenTime;nOpsInlined;propagateTime;nOpsAfterProp;wasTop"
#define REDUCTION_HEADER "file;type;iBf;iAf;ctrBf;ctrAf;targets"
#define INFO_HEADER "commit;nMaxAmpls;threshold;start;finish"

#define BENCHMARK_FOLDER "..//benchmark-results"
#define INFO_FILENAME "info.csv"
#define REDUCTION_FILENAME "reduction.csv"
#define RUNTIME_FILENAME "runtime.csv"

#define COMPARE true
#define MULTITHREAD true

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads) {
        for (size_t i = 0; i < numThreads; i++) {
            m_workers.emplace_back([this]() {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(m_mutex);
                        m_condition.wait(lock, [this]() {
                            return !m_tasks.empty() || m_stop;
                        });
                        if (m_stop && m_tasks.empty()) {
                            return;
                        }
                        task = std::move(m_tasks.front());
                        m_tasks.pop();
                        std::cout << m_tasks.size() << " left in queue" << std::endl;
                    }
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_stop = true;
        }
        m_condition.notify_all();
        for (std::thread &worker: m_workers) {
            worker.join();
        }
    }

    size_t size() {
        return m_workers.size();
    }

    template<class F, class... Args>
    auto enqueue(F &&f, Args &&... args) -> std::future<decltype(f(args...))> {
        using return_type = decltype(f(args...));
        auto task = std::make_shared<std::packaged_task<return_type()>>([Func = std::forward<F>(f)] { return Func(); });
        std::future<return_type> result = task->get_future();
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (m_stop) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            m_tasks.emplace([task]() {
                (*task)();
            });
        }
        m_condition.notify_one();
        return result;
    }

private:
    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    bool m_stop = false;
};

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
}

void compareQcs(const fs::path &file, qc::QuantumComputation &before, qc::QuantumComputation &after, std::ostream &s,
                std::mutex &c_m) {
    auto beforeIt = before.begin();
    auto afterIt = after.begin();

    size_t beforeIndex = 0;
    size_t afterIndex = 0;

    while (beforeIt != before.end()) {
        //Check if the operations are the same
        bool sameType = beforeIt->get()->getType() == afterIt->get()->getType();
        bool sameTargets = beforeIt->get()->getTargets() == afterIt->get()->getTargets();
        bool controlsSubset = std::includes(beforeIt->get()->getControls().begin(),
                                            beforeIt->get()->getControls().end(),
                                            afterIt->get()->getControls().begin(), afterIt->get()->getControls().end(),
                                            [](const qc::Control &a, const qc::Control &b) {
                                                return a.qubit < b.qubit;
                                            });


        if (sameType && sameTargets && controlsSubset) {
            //Same Gate
            if (beforeIt->get()->getNcontrols() > afterIt->get()->getNcontrols()) {
                //Something was optimized
                //fileName
                std::stringstream ss;
                ss << file.string() << ";"
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

                std::lock_guard<std::mutex> lock(c_m);
                s << ss.str();
            }

            afterIt++;
            afterIndex++;
        } else {
            //Not the same gate. Something was removed
            std::stringstream ss;
            ss << file.string() << ";"
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
            std::lock_guard<std::mutex> lock(c_m);
            s << ss.str();
        }

        beforeIt++;
        beforeIndex++;
    }
}

void
processFile(const fs::path &file, std::ostream &runtimeOut, std::ostream &compareOut, size_t maxNAmpls, std::mutex &r_m,
            std::mutex &m_c) {
    std::stringstream line;
    line << file.string();

    tp start = c::steady_clock::now();
    qc::QuantumComputation qc;

    try {
        qc = qc::QuantumComputation(file.string());
    } catch (std::exception &e) {
        line << "; qfr threw an exception while importing: " << e.what() << ";;;;;;;\n";
        return;
    }

    tp end = c::steady_clock::now();
    long long dur = c::duration_cast<c::microseconds>(end - start).count();

    //parseTime,nQubits,nOpsStart
    line << ";" << dur << ";" << qc.getNqubits() << ";" << qc.getNops();

    qc::QuantumComputation before = qc.clone();

    try {
        runBenchmark(qc, maxNAmpls, line);
    } catch (std::exception &e) {
        line << "error while running benchmark";
        std::cout << file.string() << ", error while running benchmark: " << e.what() << std::endl;
        return;
    }
    std::string runtimeString = line.str();
    if (!runtimeString.empty()) {
        std::lock_guard<std::mutex> lock(r_m);
        runtimeOut << line.str();
    }

    if (COMPARE) {
        try{
            qc::CircuitOptimizer::flattenOperations(before);
        } catch(std::exception &e) {
            line<< "qfr: error while flattening";
            std::cout << file.string() << ", error while flattening: " << e.what() << std::endl;
        }

        try {
            compareQcs(file, before, qc, compareOut, m_c);
        } catch(std::exception &e) {
            line<< "error while comparing";
            std::cout << file.string() << ", error while comparing: " << e.what() << std::endl;
        }

        compareOut.flush();
    }

    end = c::steady_clock::now();
    dur = c::duration_cast<c::microseconds>(end - start).count();
    std::cout << file.string() << ", done in " << static_cast<double>(dur) / 1e6 << "s" << std::endl;
}

void benchmarkParameters(size_t maxNAmpls, double threshold) {
    std::cout << "Benchmarking with maxNAmpls=" << maxNAmpls << " and threshold=" << threshold << std::endl;

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_c);

    char dateTime[80];
    std::setlocale(LC_TIME, "C");
    std::strftime(dateTime, 80, "%Y-%m-%d-%H-%M-%S", &now_tm);

    fs::path benchmarkFolder = BENCHMARK_FOLDER "//" + std::string(dateTime);
    create_directories(benchmarkFolder);

    std::string infoFileName = benchmarkFolder.string() + "//" + INFO_FILENAME;
    std::cout << "Writing to: " << infoFileName << std::endl;

    std::ofstream infoOut(infoFileName, std::ios::out | std::ios::trunc);
    infoOut << INFO_HEADER << std::endl;
    infoOut << GIT_COMMIT_HASH << ";" << maxNAmpls << ";" << threshold << ";" << std::string(dateTime) << ";";
    infoOut.flush();
    infoOut.close();

    std::string runtimeFileName = benchmarkFolder.string() + "//" + RUNTIME_FILENAME;
    std::cout << "Writing to: " << runtimeFileName << std::endl;

    std::ofstream runtimeOut(runtimeFileName, std::ios::out | std::ios::trunc);
    runtimeOut << RUNTIME_HEADER << std::endl;
    std::mutex runtimeMutex;

    std::ofstream compareOut;
    std::mutex compareMutex;

    if (COMPARE) {
        std::string compareFileName = benchmarkFolder.string() + "//" + REDUCTION_FILENAME;
        compareOut.open(compareFileName, std::ios::out | std::ios::trunc);
        compareOut << REDUCTION_HEADER << std::endl;
        std::cout << "Writing to: " << compareFileName << std::endl;
    }

    auto fileGen = QASMFileGenerator(QASMFileGenerator::MQT);

    size_t i = 0;
    size_t limit = 1800;

    Complex::setEpsilon(threshold);

    ThreadPool pool(MULTITHREAD ? std::thread::hardware_concurrency() : 1);
    std::cout << "Using " << pool.size() << " threads" << std::endl;
    std::vector<std::future<void>> futures;

    while (fileGen.next() && i++ < limit) {
        const fs::path &file = fileGen.get();

        futures.emplace_back(pool.enqueue([file, &runtimeOut, &compareOut, maxNAmpls, &runtimeMutex, &compareMutex] {
            processFile(file, runtimeOut, compareOut, maxNAmpls, runtimeMutex, compareMutex);
        }));
    }

    std::stringstream ss;
    for (auto &future: futures) {
        future.wait();
    }

    compareOut.close();
    runtimeOut.close();

    now_c = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    now_tm = *std::localtime(&now_c);
    std::strftime(dateTime, 80, "%H:%M:%S", &now_tm);
    std::cout << "Finished at " << std::string(dateTime) << std::endl;

    std::strftime(dateTime, 80, "%Y-%m-%d-%H-%M-%S", &now_tm);
    std::ofstream infoAppend(infoFileName, std::ios::out | std::ios::app);
    infoAppend << dateTime << std::endl;
    infoAppend.flush();
    infoAppend.close();
}

TEST_CASE("Test Circuit Performance", "[!benchmark]") {
    size_t maxNAmpls = GENERATE(static_cast<size_t>(512), 1024, 4096);
    double threshold = GENERATE(1e-8);

    benchmarkParameters(maxNAmpls, threshold);
}

TEST_CASE("Try graphstate_circuits", "[.]") {
    size_t maxNAmpls = GENERATE(static_cast<size_t>(4096));
    double threshold = GENERATE(1e-8);

    std::cout << RUNTIME_HEADER << std::endl;

    auto mqtPaths = QASMFileGenerator(QASMFileGenerator::MQT);

    while (mqtPaths.next()) {
        const fs::path &file = mqtPaths.get();
        if (file.string().find("graphstate") == std::string::npos)
            continue;

        std::cout << file.string();

        tp start = c::steady_clock::now();
        qc::QuantumComputation qc;

        try {
            qc = qc::QuantumComputation(file.string());
        } catch (std::exception &e) {
            std::cout << "; qfr threw an exception while importing: " << e.what() << ";;;;;;;\n";
            return;
        }

        tp end = c::steady_clock::now();
        long long dur = c::duration_cast<c::microseconds>(end - start).count();

        //parseTime,nQubits,nOpsStart
        std::cout << ";" << dur << ";" << qc.getNqubits() << ";" << qc.getNops();

        qc::QuantumComputation before = qc.clone();
        double oldThreshold = Complex::getEpsilon();
        Complex::setEpsilon(threshold);
        runBenchmark(qc, maxNAmpls, std::cout);
        Complex::setEpsilon(oldThreshold);
    }

}