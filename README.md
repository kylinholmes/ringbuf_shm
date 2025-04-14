# ringbuf_shm
C++ 实现 基于共享内存的环形缓冲区

## Brief
本项目主要包含2个部分
1. ringbuf.hpp: 环形缓冲区的实现
2. shm_helper.hpp: 共享内存的实现

```shell
include
├── ringbuf.hpp
└── shm_helper.hpp
```

```cpp
// ringbuf.hpp
template <size_t MAX_SIZE = 4096, typename Allocator = simple_buf>
struct ringbuf::ringbuf_t {};

// shm_helper.hpp
struct shm_helper::shm_t {};
```
----
其中 `ringbuf_t` 可以传入 `Allocator`，默认用`simple_buf`在堆上申请一片内存, 
```cpp
auto buf = ringbuf_t<>(); // 默认4096字节, 在堆上申请内存
```
可以替换成 `shm_helper::shm_t`，打开或创建共享内存，并映射到进程内
```cpp
auto shm_name = "/ringbuf_shm";
auto shm = ringbuf::ringbuf_t<4096, shm_helper::shm_t>(shm_name);
```

## One More thing
- `simple_buf` 和 `shm_helper::shm_t` 在析构时都会自动回收资源，`shm_t` 会`unmap、close`，但是不会 `unlink`，需要手动调用 `shm_helper::shm_t::unlink` 删除共享内存。


## TODO
- [x] 共享内存的实现
- [x] 环形缓冲区的实现
- [x] 测试
- [ ] 无锁