#include <chrono>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include "data.h"

enum class OpType : std::uint8_t { Read, Write, String };

struct Op {
    OpType type;
    std::uint8_t field;
    int value;
};
std::vector<Op> loadOps(const std::string& path, std::size_t expected = 0) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Cannot open " + path);
    }

    std::vector<Op> ops;
    ops.reserve(expected);

    std::string cmd;
    while (in >> cmd) {
        Op op{};
        if (cmd == "read") {
            int f;
            in >> f;
            op.type  = OpType::Read;
            op.field = static_cast<std::uint8_t>(f);
        } else if (cmd == "write") {
            int f, v;
            in >> f >> v;
            op.type  = OpType::Write;
            op.field = static_cast<std::uint8_t>(f);
            op.value = v;
        } else if (cmd == "string") {
            op.type = OpType::String;
        } else {
            throw std::runtime_error("Unknown command: " + cmd);
        }
        ops.push_back(op);
    }
    return ops;
}

std::vector<std::vector<Op>> loadProfile(const std::string& profile,int threads, std::size_t totalOps)
{
    std::vector<std::vector<Op>> result;
    for (int t = 0; t < threads; ++t) {
        std::string path = profile + "_t" + std::to_string(threads) + "_" + std::to_string(t) + ".txt";
        result.push_back(loadOps(path, totalOps / threads));
    }
    return result;
}


void worker(Data& data, const std::vector<Op>& ops, long long& sink) {
    long long sum = 0;
    for (const Op& op : ops) {
        switch (op.type) {
            case OpType::Read:
                sum += data.read(op.field);
                break;
            case OpType::Write:
                data.write(op.field, op.value);
                break;
            case OpType::String: {
                std::string s = data;
                sum += static_cast<long long>(s.size());
                break;
            }
        }
    }
    sink = sum;
}

double runOnce(const std::vector<std::vector<Op>>& ops, long long& checksum) {
    Data data;
    const std::size_t n = ops.size();
    std::vector<long long> sinks(n, 0);
    std::vector<std::thread> threads;
    threads.reserve(n);

    auto start = std::chrono::steady_clock::now();

    for (std::size_t t = 0; t < n; ++t) {
        threads.emplace_back(worker, std::ref(data), std::cref(ops[t]), std::ref(sinks[t]));
    }
    for (auto& th : threads) {
        th.join();
    }

    auto end = std::chrono::steady_clock::now();

    checksum = 0;
    for (long long s : sinks) checksum += s;

    return std::chrono::duration<double>(end - start).count();
}

double measure(const std::vector<std::vector<Op>>& ops, int runs, long long& checksum) {
    runOnce(ops, checksum);
    double total = 0;
    for (int i = 0; i < runs; ++i) {
        total += runOnce(ops, checksum);
    }
    return total / runs;
}


int main() {
    const long long N = 3000000;
    const int runs = 5;
    const char* profiles[] = {"a", "b", "c"};

    double table[3][3];

    for (int p = 0; p < 3; ++p) {
        for (int threads = 1; threads <= 3; ++threads) {
            auto ops = loadProfile(profiles[p], threads, N); 
            long long checksum = 0;
            double avg = measure(ops, runs, checksum);
            table[p][threads - 1] = avg;

            std::cout << "profile " << profiles[p] << ", threads " << threads << ": " << std::fixed << std::setprecision(4) << avg << " s (checksum " << checksum << ")\n";
        }
    }

    std::cout << "\nAverage time (s):\n";
    std::cout << "profile |   1 thread |  2 threads |  3 threads\n";
    for (int p = 0; p < 3; ++p) {
        std::cout << std::setw(7) << profiles[p] << " |";
        for (int t = 0; t < 3; ++t) {
            std::cout << std::setw(11) << std::fixed << std::setprecision(4) << table[p][t] << " |";
        }
        std::cout << "\n";
    }
}
