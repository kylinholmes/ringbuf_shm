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

#include <atomic>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <cstddef>
#include <cstdint>
#include <utility>


namespace ringbuf {


#pragma pack(push, 1) 
struct persist_t {
    std::atomic<size_t> head;     // Index of the head of the buffer
    std::atomic<size_t> tail;     // Index of the tail of the buffer
    std::atomic<size_t> buffer_size; // Maximum size of the buffer
    std::atomic<size_t> padding;
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
        std::cout << "[ringbuf_t] head:" << persist->head.load(std::memory_order_acquire) 
                  << ", tail:" << persist->tail.load(std::memory_order_acquire) 
                  << ", cap:" << persist->buffer_size.load(std::memory_order_acquire) 
                  << ", hptr:" << reinterpret_cast<void*>(persist) 
                  << ", ptr:" << reinterpret_cast<void*>(buffer) << std::endl;
    }
    ringbuf_t(): ringbuf_t("ringbuf_unnamed_shm") {}

    ~ringbuf_t() {
        allocator->destroy();
        buffer=nullptr;
    }

    template<typename T>
    int push(const T& data) {
        return push(reinterpret_cast<uint8_t*>((void*)&data), sizeof(T));
    }

    template<size_t N>
    int push(const char (&data)[N]) {
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
        size_t current_tail;
        size_t new_tail;
        size_t current_head;
        size_t available_space;
        
        do {
            // 获取当前tail和head（acquire保证看到最新值）
            current_tail = persist->tail.load(std::memory_order_acquire);
            current_head = persist->head.load(std::memory_order_acquire);
            
            // 计算可用空间（考虑环形缓冲区回绕）
            available_space = (current_head + MAX_SIZE - current_tail - 1) % MAX_SIZE;
            
            // 检查是否有足够空间
            if (available_space < N) {
                return available_space; // 缓冲区空间不足
            }
            
            // 计算新的tail位置
            new_tail = (current_tail + N) % MAX_SIZE;
            
            // 使用CAS原子更新tail，防止多生产者竞争
        } while (!persist->tail.compare_exchange_weak(
            current_tail, new_tail,
            std::memory_order_release,
            std::memory_order_relaxed));
        
        // 成功获得空间，执行数据拷贝
        if (current_tail + N > MAX_SIZE) {
            // 需要回绕的情况
            size_t first_chunk = MAX_SIZE - current_tail;
            memcpy(buffer + current_tail, data, first_chunk);
            memcpy(buffer, data + first_chunk, N - first_chunk);
        } else {
            // 不需要回绕的情况
            memcpy(buffer + current_tail, data, N);
        }
        
        return 0; // 成功
    }

    template<typename T>
    int pop(T& data) {
        return pop(reinterpret_cast<uint8_t*>((void*)&data), sizeof(T));
    }

    template<size_t N>
    int pop(const char (&data)[N]) {
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
        size_t current_head;
        size_t new_head;
        size_t current_tail;
        size_t available_data;
        
        do {
            // 获取当前head和tail（acquire保证看到最新值）
            current_head = persist->head.load(std::memory_order_acquire);
            current_tail = persist->tail.load(std::memory_order_acquire);
            
            // 计算可用数据量（考虑环形缓冲区回绕）
            available_data = (current_tail + MAX_SIZE - current_head) % MAX_SIZE;
            
            // 检查是否有足够数据
            if (available_data < N) {
                return available_data == 0 ? -1 : static_cast<int>(available_data);
            }
            
            // 计算新的head位置
            new_head = (current_head + N) % MAX_SIZE;
            
            // 使用CAS原子更新head，防止多消费者竞争
        } while (!persist->head.compare_exchange_weak(
            current_head, new_head,
            std::memory_order_release,
            std::memory_order_relaxed));
        
        // 成功获得数据，执行数据拷贝
        if (current_head + N > MAX_SIZE) {
            // 需要回绕的情况
            size_t first_chunk = MAX_SIZE - current_head;
            memcpy(data, buffer + current_head, first_chunk);
            memcpy(data + first_chunk, buffer, N - first_chunk);
        } else {
            // 不需要回绕的情况
            memcpy(data, buffer + current_head, N);
        }
        
        return 0; // 成功
    }

    size_t available() const {
        size_t h = persist->head.load(std::memory_order_acquire);
        size_t t = persist->tail.load(std::memory_order_acquire);
        return (h + MAX_SIZE - t - 1) % MAX_SIZE;
    }
    size_t capacity() const {
        return persist->buffer_size.load(std::memory_order_acquire);
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
        int send(const T& data) const {
            return rb.push(reinterpret_cast<uint8_t*>((void*)&data), sizeof(T));
        }
    };
    class receiver_t {
        ringbuf_t<MAX_SIZE, Allocator>& rb;
        public:
        receiver_t(ringbuf_t<MAX_SIZE, Allocator>& rb) : rb(rb) {}
        int receive(uint8_t* data, size_t size) const {
            return rb.pop(data, size);
        }
        template<size_t N>
        int receive(char (&data)[N]) const {
            return rb.pop(reinterpret_cast<uint8_t*>((void*)data), N);
        }
        template<typename T>
        int receive(T& data) const {
            return rb.pop(reinterpret_cast<uint8_t*>(&data), sizeof(T));
        }
    };

    auto make_pair() {
        return std::make_pair(sender_t(*this), receiver_t(*this));
    }

    template<typename T>
    class iterator_t {
        ringbuf_t<MAX_SIZE, Allocator>& rb;
        T data;
    public:
        iterator_t(ringbuf_t<MAX_SIZE, Allocator>& rb) : rb(rb) {
            while(rb.pop(reinterpret_cast<uint8_t*>(&data), sizeof(T)) != 0);
        }
        iterator_t<T>& operator++() {
            while(rb.pop(reinterpret_cast<uint8_t*>(&data), sizeof(T)) != 0);
            return *this;
        }
        T operator*() const {
            return data;
        }
        iterator_t<T>& begin() {
            return *this;
        }
        iterator_t<T>& end() {
            return *this;
        }
        bool operator!=(const iterator_t<T>& other) const {
            return true;
        }
        bool operator==(const iterator_t<T>& other) const {
            return false;
        }



    };
    
    template<typename T>
    iterator_t<T> iter() {
        return iterator_t<T>{*this};
    }


};

}

#endif // RINGBUF_H