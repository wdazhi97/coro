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

                {std::cout << "await suspend" << this << std::endl;}
                std::thread([this, coroutine_handle]() {
                    std::this_thread::sleep_for(std::chrono::seconds(this->sleepTime));
                    std::cout << "reach the time " << this->sleepTime << std::endl;
                    if(!coroutine_handle.done()) {
                        coroutine_handle.resume();
                    }
                }).detach();
            }
            int await_resume() {
                std::cout << "task resume after " << sleepTime  << "s" << std::endl;
                return sleepTime;
            }

            awaiter()  {std::cout << "wait time task construct" << std::endl;}
            ~awaiter()  {
                sleepTime++;
                std::cout << "wait time task disconstruct" << std::endl;}
        };
        return awaiter{};
    }
};




cppcoro::cancellation_task<int> cancel(cancel_token token, int x ,float y) {
    std::cout << "start cancel 1" << std::endl;
    co_await wait_time_task();
    std::cout << "end cancel 1" << std::endl;
}

cppcoro::cancellation_task<void> cancel2(cancel_token token)
{
    std::cout << "start cancel 2" << std::endl;
    co_await cancel(token, 5, 1.6f);
    std::cout << "end cancel 2" << std::endl;
}

cppcoro::sync_task<int> test() {
    cancel_request req;
    auto token = req.token();
    std::thread([req]() mutable {
        //std::cout << std::this_thread::get_id() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        req.request_cancel();
        //std::cout << "request finished" << std::endl;
    }).detach();
    co_await cancel2(token);
    std::cout << "task finished" << std::endl;
    co_return 1;
}

cppcoro::task<int> foobar()
{
    co_return 1;
}

cppcoro::task<int> bar()
{
    co_await foobar();
}

cppcoro::sync_task<int> foo()
{
    co_await bar();
}

int main() {
    auto result = foo();
}






