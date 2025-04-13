#ifndef RINGBUF_H
#define RINGBUF_H
#include "shm_helper.h"

#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <cstddef>
#include <cstdint>


namespace ringbuf {


struct persist_t {
    size_t head;     // Index of the head of the buffer
    size_t tail;     // Index of the tail of the buffer
    size_t max_size; // Maximum size of the buffer
};

template<size_t MAX_SIZE=4096, typename Allocator=shm_helper::shm_t>
struct ringbuf_t {
    // persist_t persist; // Pointer to the persist structure
    // size_t head;     // Index of the head of the buffer
    // size_t tail;     // Index of the tail of the buffer
    // size_t max_size; // Maximum size of the buffer
    persist_t *persist; // Pointer to the persist structure
    uint8_t *buffer; // Pointer to the buffer
    Allocator* allocator; // Allocator for shared memory

    ringbuf_t(const char* key) {
        // buffer = new uint8_t[max_size]; 
        allocator = Allocator::create(key, sizeof(persist_t) + MAX_SIZE);
        if (allocator == nullptr) {
            throw std::runtime_error("Failed to create shared memory");
        }
        persist_t p;
        memcpy(&p, allocator->ptr, sizeof(persist_t));
        buffer = static_cast<uint8_t*>(allocator->ptr) + sizeof(persist_t);
        persist = reinterpret_cast<persist_t*>(allocator->ptr);
        persist->max_size = allocator->size;
        // head = p.head;
        // tail = p.tail;
        // max_size = allocator->size;
        printf("[ringbuf_t] head:%zu, tail:%zu, max_size:%zu\n", persist->head, persist->tail, persist->max_size);
    }

    ~ringbuf_t() {
        allocator->destroy();
        buffer=nullptr;
    }

    template<typename T, size_t N = sizeof(T)>
    bool push(T data) {
        size_t h = persist->head;
        size_t t = persist->tail;
        auto size = (t + MAX_SIZE - h) % MAX_SIZE;
        if (size + N < MAX_SIZE) {
            for (size_t i = 0; i < N; i++) {
                buffer[t] = ((uint8_t*)&data)[i];
                t = (t + 1) > MAX_SIZE ? 0 : (t + 1);
            }
            persist->tail = t;
            return true;
        } else {
            return false;
        }
    }

    bool pop(size_t N, uint8_t* data) {
        size_t h = persist->head;
        size_t t = persist->tail;
        for (size_t i = 0; i < N; i++) {
            if (h == t) {
                persist->head = h;                
                return false; // Buffer is empty
            }
            data[i] = data[h];
            h = (h + 1) > MAX_SIZE ? 0 : (h + 1);
        }
        persist->head = h;
        return true;
    }

};

}

#endif // RINGBUF_H