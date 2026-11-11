#include <fstream>
#include <iostream>
#include <random>`
#include <string>

void generate(const std::string& path, long long count,
              const double w[5], unsigned seed)
{
    std::mt19937 gen(seed);
    std::discrete_distribution<int> dist(w, w + 5);

    std::ofstream out(path);
    if (!out) {
        std::cerr << "Cannot open " << path << "\n";
        return;
    }

    for (long long i = 0; i < count; ++i) {
        switch (dist(gen)) {
            case 0: out << "read 0\n";    break;
            case 1: out << "write 0 1\n"; break;
            case 2: out << "read 1\n";    break;
            case 3: out << "write 1 1\n"; break;
            case 4: out << "string\n";    break;
        }
    }
}

int main() {
    const long long N = 3000000;

    struct Profile { const char* name; double w[5]; };
    const Profile profiles[] = {
        {"a", {5, 40, 5, 5, 45}}, 
        {"b", {20, 20, 20, 20, 20}},
        {"c", {2, 1, 80, 2, 15}}
    };

    unsigned seed = 1;
    for (const auto& p : profiles) {
        for (int threads = 1; threads <= 3; ++threads) {
            for (int t = 0; t < threads; ++t) {
                std::string path = std::string(p.name) + "_t" + 
                                    std::to_string(threads) + "_" +
                                    std::to_string(t) + ".txt";
                generate(path, N / threads, p.w, seed++);
                std::cout << "created " << path << "\n";
            }
        }
    }
}