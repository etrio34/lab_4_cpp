#pragma once

#include <mutex>
#include <string>

// Одне поле разом зі своїм м'ютексом.
// alignas(64) розносить поля по різних рядках кеша (проти false sharing).
struct alignas(64) Field {
    int value = 0;
    std::mutex m;
};

class Data {
    Field f[2];

public:
    int read(int i) {
        std::lock_guard<std::mutex> lk(f[i].m);
        return f[i].value;
    }

    void write(int i, int v) {
        std::lock_guard<std::mutex> lk(f[i].m);
        f[i].value = v;
    }

    // Бере обидва м'ютекси одразу (без deadlock), копіює значення,
    // а рядок будує вже після розблокування.
    operator std::string() {
        int a, b;
        {
            std::scoped_lock lk(f[0].m, f[1].m);
            a = f[0].value;
            b = f[1].value;
        }
        return std::to_string(a) + " " + std::to_string(b);
    }
};
