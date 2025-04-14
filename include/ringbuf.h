#ifndef RINGBUF_H
#define RINGBUF_H
#include "shm_helper.h"

#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <cstddef>
#include <cstdint>
#include <type_traits>


namespace ringbuf {


#pragma pack(push, 1) 
struct persist_t {
    size_t head;     // Index of the head of the buffer
    size_t tail;     // Index of the tail of the buffer
    size_t max_size; // Maximum size of the buffer
};
#pragma pack(pop)

template<size_t MAX_SIZE=4096, typename Allocator=shm_helper::shm_t>
struct ringbuf_t {
    persist_t *persist; // Pointer to the persist structure
    uint8_t *buffer; // Pointer to the buffer
    Allocator* allocator; // Allocator for shared memory

    ringbuf_t(const char* key) {
        // buffer = new uint8_t[max_size]; 
        allocator = Allocator::create(key, sizeof(persist_t) + MAX_SIZE);
        if (allocator == nullptr) {
            throw std::runtime_error("Failed to create shared memory");
        }
        buffer = static_cast<uint8_t*>(allocator->ptr) + sizeof(persist_t);
        persist = reinterpret_cast<persist_t*>(allocator->ptr);
        persist->max_size = allocator->size;
        printf("[ringbuf_t] head:%zu, tail:%zu, max_size:%zu, hptr:%p, ptr:%p\n", persist->head, persist->tail, persist->max_size, reinterpret_cast<void*>(persist), reinterpret_cast<void*>(buffer));
    }

    ~ringbuf_t() {
        allocator->destroy();
        buffer=nullptr;
    }

    template<size_t N>
    bool push(const char (&data)[N]) {
        return push( static_cast<uint8_t*>((void*) data), N);
    }

    bool push(uint8_t* data, size_t N) {
        size_t t = persist->tail;
        auto size = (t + MAX_SIZE - persist->head) % MAX_SIZE;
        if (size + N < MAX_SIZE) {
            auto k = t + N;
            if(k > MAX_SIZE) {
                k = k - MAX_SIZE;
                memcpy(buffer + t, data, k);
                memcpy(buffer, data + k, N - k);
                persist->tail = N - k;
            } else {
                memcpy(buffer + t, data, N);
                persist->tail += N;
            }
            return true;
        } else {
            // Buffer is full
            return false;
        }
    }

    bool pop(size_t N, uint8_t* data) {
        size_t h = persist->head;
        auto size = (h + MAX_SIZE - persist->tail) % MAX_SIZE;
        if (size >= N) {
            auto k = h + N;
            if(k > MAX_SIZE) {
                k = k - MAX_SIZE;
                memcpy(data, buffer + h, k);
                memcpy(data + k, buffer, N - k);
            } else {
                memcpy(data, buffer + h, N);
            }
            persist->head += N;
            return true;
        } else {
            // Buffer is empty
            return false;
        }
    }

};

}

#endif // RINGBUF_H