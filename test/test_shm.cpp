#include "shm_helper.hpp"
#include "ringbuf.hpp"

#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <locale>
#include <memory>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <utility>

typedef enum {
    FROM_PING = 1,
    FROM_PONG = 2,
} msg_type;

auto not_ns_ts() {
    auto ts = std::chrono::high_resolution_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(ts).count();
}

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

int test_ring_buffer_shm() {
    ringbuf::ringbuf_t<4096, shm_helper::shm_t> shm("/shm");
    auto ret = shm.push("hello share memory");
    if(ret) {
        puts("push failed");
        return -1;
    }
    char s[19];
    ret = shm.pop(s);
    if (ret) {
        puts("pop failed");
        return -1;
    }
    s[18] = 0;
    printf("%s\n", s);
    return 0;
}

int test_mulit_thread() {
    ringbuf::ringbuf_t<4096, shm_helper::shm_t> channel{"/channel"};
    auto [sender, receiver] = channel.make_pair();
    struct msg {
        // unix timestamp
        uint32_t timestamp;
        // msg type
        uint32_t type;
    };

    ringbuf::ringbuf_t<4096> repoter{};
    auto [tx, rx] = repoter.make_pair();

    auto producer = std::thread([sender] () {
        while (true) {
            msg m;
            m.timestamp = not_ns_ts();
            m.type = FROM_PING;
            auto ret = sender.send(m);
            if (ret) {
                printf("send failed, ret:%d\n", ret);
            }
            sleep(1);
        }
    });

    auto consumer = std::thread([receiver, tx] () {
        while (true) {
            msg m;
            auto ret = receiver.receive(m);
            if (ret) {
                printf("receive failed, ret:%d\n", ret);
            }
            auto ts =  not_ns_ts();
            auto rtt = ts - m.timestamp;
            tx.send(rtt);
        }
    });

    while(true) {
        uint32_t rtt;
        auto ret = rx.receive(rtt);
        if (ret) {
            printf("receive failed, ret:%d\n", ret);
        }
        printf("rtt: %u\n", rtt);
    }

}

int main() {
    // test_shm();
    // test_ring_simple_buffer();
    // test_ring_buffer_shm();
    test_mulit_thread();

    
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