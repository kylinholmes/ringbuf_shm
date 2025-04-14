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

    while(true) {
        long long p2;
        auto used = rb.pop(p2);
        if(used) {
            continue;
        }
        auto now = not_ns_ts();
        std::cout << "now:" << now << ", p1:" << p2 << ", gap:" << now - p2 << std::endl;
    }
}

