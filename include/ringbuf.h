#ifndef RINGBUF_H
#define RINGBUF_H
#include "shm_helper.h"

#include <stdexcept>
#include <cstddef>
#include <cstdint>


namespace ringbuf {

template<size_t MAX_SIZE=4096, typename Allocator=shm_helper::shm_t>
struct ringbuf_t {
    uint8_t *buffer; // Pointer to the buffer
    size_t head;     // Index of the head of the buffer
    size_t tail;     // Index of the tail of the buffer
    size_t max_size; // Maximum size of the buffer
    // size_t size;     // Current size of the buffer
    Allocator* allocator; // Allocator for shared memory

    ringbuf_t(const char* key) : head(0), tail(0), max_size(MAX_SIZE) {
        // buffer = new uint8_t[max_size]; 
        allocator = Allocator::create(key, max_size);
        if (allocator == nullptr) {
            throw std::runtime_error("Failed to create shared memory");
        }
        buffer = static_cast<uint8_t*>(allocator->ptr);
    }

    ~ringbuf_t() {
        allocator->destroy();
        buffer=nullptr;
    }

    template<typename T, size_t N = sizeof(T)>
    bool push(T data) {
        auto size = (tail + max_size - head) % max_size;
        if (size + N < max_size) {
            for (size_t i = 0; i < N; i++) {
                buffer[tail] = ((uint8_t*)&data)[i];
                tail = (tail + 1) > max_size ? 0 : (tail + 1);
            }
            return true;
        } else {
            return false;
        }
    }

    bool pop(size_t N, uint8_t* data) {
        for (size_t i = 0; i < N; i++) {
            if (head == tail) {
                return false; // Buffer is empty
            }
            data[i] = buffer[head];
            head = (head + 1) > max_size ? 0 : (head + 1);
        }
        return true;
    }

};

};

#endif // RINGBUF_H