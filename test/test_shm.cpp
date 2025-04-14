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

int test_shm() {
    // Create a shared memory object
    auto shm = std::make_unique<shm_helper::shm_t*>(shm_helper::shm_t::create("ringbuf_shm"));
    if (*shm == nullptr) {
        printf("Failed to create shared memory, errno: %d, %s\n", errno, strerror(errno));
        return -1;
    }
    auto buffer = (*shm)->ptr;
    auto str = "fuck share memory";
    memcpy(buffer, str, strlen(str));
    (*shm)->unlink();
    return 0;
}

int test_ring_simple_buffer() {
    ringbuf::ringbuf_t<64, ringbuf::simple_buf> simp1e_buf("/simple_buffer");

    // push faster than pop, full first
    for(auto i=0;i < 100; i++) {
        auto used = simp1e_buf.push("1234");
        if(used) {
            printf("puts failed, used_buf_size%d\n", used);
            break; // less than each push bytes, nerver pop failed
        }
        char s[3]; 
        used = simp1e_buf.pop(s);
        if(used) {
            printf("pop failed, used_buf_size:%d\n", used);
            continue;
        }
    }

    // pop faster than push, empty first
    for(auto i=0;i < 100; i++) {
        auto used = simp1e_buf.push("1234"); // push 5bytes each times
        if(used) {
            printf("puts failed, used_buf_size%d\n", used);
        }
        char s[6]; 
        used = simp1e_buf.pop(s);
        if(used) {
            printf("pop failed, rem_buf_size:%d\n", used);
            break; // greater than each push bytes, nerver push failed
        }
    }

    // pop faster than push, nerver stop, empty occur more
    for(auto i=0;i < 1000; i++) {
        auto used = simp1e_buf.push("1234");
        if(used) {
            printf("puts failed, used_buf_size%d\n", used);
        }
        char s[6];
        used = simp1e_buf.pop(s);
        if(used) {
            printf("%d, pop failed, rem_buf_size:%d\n", i, used);
        }
    }
    return 0;
}

int test_ring_buffer() {
    ringbuf::ringbuf_t<64> ping2pong("/ping2pong");
    // ringbuf::ringbuf_t<> pong2ping("/pong2ping");
    auto ret = ping2pong.push("hello share memory\0");
    if(!ret) {
        puts("push failed");
        return -1;
    }
    char s[18];
    ret = ping2pong.pop(s);
    if (!ret) {
        puts("pop failed");
        return -1;
    }
    printf("%s", s);
    return 0;
}

int main() {
    // test_shm();
    // test_ring_simple_buffer();
    test_ring_buffer();
    
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