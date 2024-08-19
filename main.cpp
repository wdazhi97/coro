#include <iostream>
#include <coroutine>
#include <thread>
#include "include/cancellation_task.hpp"

#include "include/task.hpp"
#include "include/sync_task.hpp"


class wait_time_task
{
public:

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
    std::cout << "start cancel 1" << std::endl;
    co_await wait_time_task();
}

cppcoro::cancellation_task cancel2(cancel_token token)
{
    std::cout << "start cancel 2" << std::endl;
    co_await cancel(token);
    std::cout << "end cancel 2" << std::endl;
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
