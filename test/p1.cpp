#include "shm_helper.hpp"
#include "ringbuf.hpp"
#include <cstdio>
#include <thread>

auto not_ns_ts() {
    auto ts = std::chrono::high_resolution_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(ts).count();
}

int main() {
    using namespace std::chrono_literals;
    auto rb = ringbuf::ringbuf_t<4096, shm_helper::shm_t>("my_shm");

    for(auto p1=0; p1 < 1000; p1++) {
        auto ts=  not_ns_ts();
        rb.push(ts);
        printf("%lld\n", ts);
        std::this_thread::sleep_for(1s);
    }
}

