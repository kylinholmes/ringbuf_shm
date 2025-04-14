#include "shm_helper.hpp"
#include "ringbuf.hpp"
#include <cstdio>
#include <thread>

int main() {
    using namespace std::chrono_literals;
    auto rb = ringbuf::ringbuf_t<4096, shm_helper::shm_t>("my_shm");

    while(true) {
        int p2;
        auto used = rb.pop(p2);
        if(used) {
            printf("Buffer is empty, used size: %d\n", used);
            std::this_thread::sleep_for(500ms);
            continue;
        }
        printf("%d\n", p2);
    }
}

