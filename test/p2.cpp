#include "shm_helper.hpp"
#include "ringbuf.hpp"
#include <cstdio>
#include <iostream>
#include <thread>


auto not_ns_ts() {
    auto ts = std::chrono::high_resolution_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(ts).count();
}

int main() {
    using namespace std::chrono_literals;
    auto rb = ringbuf::ringbuf_t<4096, shm_helper::shm_t>("my_shm");
    std::iostream::sync_with_stdio(0);

    for (const auto& p1: rb.iter<long long>()) {
        auto ts = not_ns_ts();
        std::cout << "now:" << ts << ", p1:" << p1 << ", gap:" << ts - p1 << std::endl;
    }
}

