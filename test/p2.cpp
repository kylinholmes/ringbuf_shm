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

    while(true) {
        long long p2;
        auto used = rb.pop(p2);
        if(used) {
            printf("Buffer is empty, used size: %d\n", used);
            std::this_thread::sleep_for(1s);
            continue;
        }
        auto now = not_ns_ts();
        printf("now:%lld, p1:%lld, gap:%lld\n", now, p2, now - p2);
    }
}

