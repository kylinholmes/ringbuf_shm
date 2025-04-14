/**
 * @file ringbuf.hpp
 * @author kylin (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-04-15
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef RINGBUF_H
#define RINGBUF_H
// #include "shm_helper.hpp"

#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <cstddef>
#include <cstdint>
#include <utility>


namespace ringbuf {


#pragma pack(push, 1) 
struct persist_t {
    size_t head;     // Index of the head of the buffer
    size_t tail;     // Index of the tail of the buffer
    size_t buffer_size; // Maximum size of the buffer
    size_t padding;
};
#pragma pack(pop)


struct simple_buf {
    uint8_t* ptr; // Pointer to the buffer
    size_t size; // Size of the buffer
    simple_buf(size_t n) : ptr(new uint8_t[n]), size(n) {}
    static simple_buf* create([[maybe_unused]]const char* __key, size_t size) noexcept {
        return new simple_buf(size);
    }
    void destroy() {
        delete[] ptr;
    }
    ~simple_buf() {
        destroy();
    }
    simple_buf(const simple_buf&) = delete; // Disable copy constructor
    simple_buf& operator=(const simple_buf&) = delete; // Disable copy assignment
};


template<size_t MAX_SIZE=4096, typename Allocator=simple_buf>
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
        persist->buffer_size = allocator->size - sizeof(persist_t);
        printf("[ringbuf_t] head:%zu, tail:%zu, cap:%zu, hptr:%p, ptr:%p\n", persist->head, persist->tail, persist->buffer_size, reinterpret_cast<void*>(persist), reinterpret_cast<void*>(buffer));
    }

    ~ringbuf_t() {
        allocator->destroy();
        buffer=nullptr;
    }

    template<size_t N>
    auto push(const char (&data)[N]) {
        return push( reinterpret_cast<uint8_t*>((void*)data), N);
    }

    /**
     * @brief 
     * 
     * @param data 
     * @param N 
     * @return int return 0 if push success, or current used size in buffer
     */
    int push(uint8_t* data, size_t N) {
        size_t t = persist->tail;
        auto used = (t + MAX_SIZE - persist->head) % MAX_SIZE;
        if (used + N < MAX_SIZE) {
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
            return 0;
        } else {
            // Buffer is full
            return used;
        }
    }

    template<size_t N>
    auto pop(const char (&data)[N]) {
        return pop( reinterpret_cast<uint8_t*>((void*) data), N);
    }

    /**
     * @brief 
     * 
     * @param data 
     * @param N 
     * @return int return 0 if pop success, or current used size in buffer
     */
    int pop(uint8_t* data, size_t N) {
        size_t h = persist->head;
        auto used = (persist->tail - h + MAX_SIZE) % MAX_SIZE;
        if (used >= N) {
            auto k = h + N;
            if(k > MAX_SIZE) {
                k = k - MAX_SIZE;
                memcpy(data, buffer + h, k);
                memcpy(data + k, buffer, N - k);
                persist->head = N - k;
            } else {
                memcpy(data, buffer + h, N);
                persist->head += N;
            }
            return 0;
        } else {
            // Buffer is empty or not enough data to pop
            return used;
        }
    }

    size_t size() const {
        return (persist->tail - persist->head + MAX_SIZE) % MAX_SIZE;
    }
    size_t capacity() const {
        return persist->buffer_size;
    }


    class sender_t {
        ringbuf_t<MAX_SIZE, Allocator>& rb;
        public:
        sender_t(ringbuf_t<MAX_SIZE, Allocator>& rb) : rb(rb) {}
        int send(const uint8_t* data, size_t size) {
            return rb.push(data, size);
        }
        template<size_t N>
        int send(const char (&data)[N]) {
            return rb.push(reinterpret_cast<uint8_t*>((void*)data), N);
        }
        template<typename T>
        int send(const T& data) {
            return rb.push(reinterpret_cast<uint8_t*>((void*)&data), sizeof(T));
        }
    };
    class receiver_t {
        ringbuf_t<MAX_SIZE, Allocator>& rb;
        public:
        receiver_t(ringbuf_t<MAX_SIZE, Allocator>& rb) : rb(rb) {}
        int receive(uint8_t* data, size_t size) {
            return rb.pop(data, size);
        }
        template<size_t N>
        int receive(char (&data)[N]) {
            return rb.pop(reinterpret_cast<uint8_t*>((void*)data), N);
        }
        template<typename T>
        int receive(T& data) {
            return rb.pop(reinterpret_cast<uint8_t*>(&data), sizeof(T));
        }
    };

    auto make_pair() {
        return std::make_pair(sender_t(*this), receiver_t(*this));
    }
};

}

#endif // RINGBUF_H