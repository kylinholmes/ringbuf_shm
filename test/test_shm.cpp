#include "shm_helper.h"
#include "ringbuf.h"

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <iostream>
#include <thread>
#include <unistd.h>

typedef enum {
    FROM_PING = 1,
    FROM_PONG = 2,
} msg_type;

struct msg {
    // unix timestamp
    uint32_t timestamp;
    // msg type
    uint32_t type;
};

// void producer(ringbuf::ringbuf_t<4096>& rb, ringbuf::ringbuf_t<4096>& rb2) {
//     uint32_t last_ts = 0;
//     msg m;
//     m.timestamp = static_cast<uint32_t>(time(nullptr));
//     m.type = FROM_PING;
//     rb.push(m);
//     last_ts = m.timestamp;
//     for (int i = 0; i < 10000; i++) {
//         auto ret = rb2.pop(sizeof(m), (uint8_t*)&m);
//         if (m.type == FROM_PONG && ret) {
//             auto rtt = m.timestamp - last_ts;
//             printf("rtt: %u\n", rtt);
//             m.timestamp ++;
//             last_ts = m.timestamp;
//             m.type = FROM_PING;
//             rb.push(m);
//         }
//     }
// }

// void consumer(ringbuf::ringbuf_t<4096>& rb, ringbuf::ringbuf_t<4096>& rb2) {
//     msg m;
//     for (int i = 0; i < 10000; i++) {
//         auto ret = rb.pop(sizeof(m), (uint8_t*)&m);
//         if (m.type == FROM_PING && ret)  {
//             m.timestamp++;
//             m.type = FROM_PONG;
//             rb2.push(m);
//         }
//     }
// }

int main() {
    // Create a shared memory object
    auto shm = std::make_unique<shm_helper::shm_t*>(shm_helper::shm_t::create("/ringbuf_shm"));
    if (*shm == nullptr) {
        printf("Failed to create shared memory, errno: %d, %s\n", errno, strerror(errno));
        return -1;
    }
    // auto p = (*shm)->ptr;
    // memcpy(p, str, strlen(str));
    ringbuf::ringbuf_t<> ping2pong("/ping2pong");
    // ringbuf::ringbuf_t<> pong2ping("/pong2ping");
    ping2pong.push("hello share memory");
    auto str = "fuck share memory";
    memcpy(ping2pong.buffer, str, strlen(str));
    // ping2pong.push(2);
    // std::cout << ping2pong.persist->tail << std::endl;
    // // Create a producer and consumer thread
    // std::thread producer_thread(producer, std::ref(ping2pong), std::ref(pong2ping));
    // std::thread consumer_thread(consumer, std::ref(ping2pong), std::ref(pong2ping));
    // // Wait for the threads to finish
    // producer_thread.join();
    // consumer_thread.join();



    return 0;
}