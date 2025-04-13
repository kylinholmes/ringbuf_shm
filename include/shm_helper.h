/**
 * @file shm_helper.h
 * @author kylin (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-04-14
 * 
 * @copyright Copyright (c) 2025
 * 
 * on Linux test only, not support for macOS
 */
#ifndef SHM_HELPER
#define  SHM_HELPER
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cstddef>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

namespace shm_helper {
    const int MAX_KEY_LENGTH = 256; // Maximum length of the key
    struct shm_t {
        int fd; // File descriptor for the shared memory object
        size_t size; // Size of the shared memory object
        void *ptr; // Pointer to the shared memory object
        char key[MAX_KEY_LENGTH]; // Key for the shared memory object

        shm_t(int fd): fd(fd), size(0), ptr(nullptr) {}
        shm_t() = default;
        static shm_t* create(const char* key, size_t size) noexcept {
            int fd = shm_open(key,  O_RDWR | O_CREAT | O_EXCL , S_IRUSR | S_IWUSR);
            if (fd == -1 && errno == EEXIST) {
                fd = shm_open(key, O_RDWR, S_IRUSR | S_IWUSR);
            }
            if (fd == -1) {
                perror("shm_open failed");
                return nullptr;
            }

            shm_t* shm = new shm_t(fd);
            std::strncpy(shm->key, key, MAX_KEY_LENGTH);
            if(!shm->shm_truncate(size)) {
                close(fd);
                perror("shm_truncate failed");
                delete shm;
                return nullptr;
            }
            printf("[create] fd: %d, key: %s, size: %zu\n", fd, key, size);
            return shm;
        }
        bool shm_truncate(size_t size_in_bytes) noexcept {
            if (fd <= 0) {
                return false;
            }
            struct stat shm_stat;
            fstat(fd, &shm_stat);
            
            if(ptr && size) {
                munmap(ptr, size);
            }

            // default is max(16384, size_in_bytes)
            if (ftruncate(fd, size_in_bytes) == -1) {
                printf("ftruncate failed,fd:%d,size:%ld\n", fd, size_in_bytes);
                return false;
            }
            this->size = size_in_bytes;
            this->ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (ptr == MAP_FAILED) {
                perror("mmap failed");
                return false;
            }
            return true;
        }
        void destroy() noexcept {
            if (ptr != MAP_FAILED) {
                munmap(ptr, size);
            }
            if (fd != -1) {
                close(fd);
                shm_unlink(key);
                printf("[destroy] fd: %d, key: %s\n", fd, key);
            }
        }
        ~shm_t() {
            destroy();
        }
    };

}

#endif