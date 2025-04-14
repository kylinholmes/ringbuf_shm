#include "shm_helper.hpp"
#include "ringbuf.hpp"
#include <thread>

int main() {
    using namespace std::chrono_literals;
    auto rb = ringbuf::ringbuf_t<4096, shm_helper::shm_t>("my_shm");

    for(auto p1=0; p1 < 1000; p1++) {
        rb.push(p1);
        std::this_thread::sleep_for(1s);
    }
}

