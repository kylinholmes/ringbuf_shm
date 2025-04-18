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
        auto func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
        rb.push([task_ptr](){
            try {
                (*task_ptr)();
            } catch (const std::exception& e) {
                std::cerr << "Task exception: " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "Task exception: unknown" << std::endl;
            }
        });
        return task_ptr->get_future();
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
    auto f1 = tp.push([](int a, int b) {
        return a + b;
    }, 1, 2);
    auto f2 = tp.push([](std::string a, std::string b) {
        return a + b;
    }, "3", "4");
    std::cout << "f1:" << f1.get() << ", f2:" << f2.get() << std::endl;

}