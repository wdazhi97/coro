#include <iostream>
#include <coroutine>
#include <thread>
#include "include/cancellation_task.hpp"

#include "include/task.hpp"
#include "include/sync_task.hpp"


class wait_time_task
{
public:
    struct promise_type
    {
        std::suspend_always initial_suspend() noexcept {  return {}; }
        std::suspend_never final_suspend() noexcept{ return {};}
        wait_time_task get_return_object() noexcept { return {}; };
        void unhandled_exception() {};
        void return_void() noexcept {}
    };
public:
    auto operator co_await() const &noexcept
    {
        struct awaiter {
            int sleepTime = 0;

            bool await_ready() {
                return false;
            }
            void await_suspend(std::coroutine_handle<> coroutine_handle) {
                sleepTime = rand() % 5 + 5;
                std::thread([this, coroutine_handle]() {
                    std::this_thread::sleep_for(std::chrono::seconds(this->sleepTime));
                    if(!coroutine_handle.done()) {
                        coroutine_handle.resume();
                    }
                }).detach();
                std::cout << "task resume after " << std::endl;
            }
            int await_resume() {
                std::cout << "task resume after " << sleepTime  << "s" << std::endl;
                return sleepTime;
            }
        };
        return awaiter{};
    }
};


cppcoro::cancellation_task cancel(cancel_token token) {
    //co_return;
    co_await wait_time_task();
}

cppcoro::sync_task<int> test() {
    cancel_request req;
    auto token = req.token();
    std::thread([req]() mutable {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        req.request_cancel();
    }).detach();
    co_await cancel(token);
    co_return 1;
}



int main() {
    auto result = test();
    //std::cout << result.result() << std::endl;
    while(1){};
}
