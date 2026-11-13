#pragma once

#include <mutex>
#include <string>

class Data {
    alignas(64) int f[2] = {0, 0};
    std::mutex m;

public:
    int read(int i) {
        std::lock_guard<std::mutex> lk(m);
        return f[i];
    }

    void write(int i, int v) {
        std::lock_guard<std::mutex> lk(m);
        f[i] = v;
    }

    operator std::string() {
        int a, b;
        {
            std::lock_guard<std::mutex> lk(m);
            a = f[0];
            b = f[1];
        }
        return std::to_string(a) + " " + std::to_string(b);
    }
};