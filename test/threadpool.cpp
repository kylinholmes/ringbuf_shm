#include "ringbuf.hpp"
#include <future>
#include <string>
#include <thread>
#include <vector>
#include <functional>

class thread_pool_t {
    ringbuf::ringbuf_t<4096> rb;
    std::vector<std::thread> th;
public:
    thread_pool_t(size_t thread_num) : rb() {
        for (size_t i = 0; i < thread_num; ++i) {
            th.emplace_back([this]() {
                for(const auto& func: rb.iter<std::function<void()>>()) {
                    func();
                }
            });
        }
    }
    template <typename F, typename... Args>
    auto push(F &&f, Args &&...args) -> std::future<decltype(f(args...))> {
        using return_type = decltype(f(args...));
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        auto res = task->get_future();
        rb.push([task]() { (*task)(); });
        return res;
    }
    auto push(std::function<void()> f) {
        rb.push(f);
    }
    ~thread_pool_t() {
        for (auto& t : th) {
            if (t.joinable()) {
                t.join();
            }
        }
    }
};

int main () {
    thread_pool_t tp(4);
    tp.push([a=1, b=2]() {
        std::cout << "thread:" << std::this_thread::get_id() << ", a:" << a << ", b:" << b << std::endl;
    });

}