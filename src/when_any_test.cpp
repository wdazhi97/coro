// cppcoro.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <thread>
#include <chrono>
#include <ctime>
#include <random>
#include <coroutine>
#include <sync_task.hpp>

#include "cancellation_task.hpp"
#include "when_any_ready_task.h"
#include "when_any_task.h"
#include "sync_task.hpp"


static int tmp_time = 0;

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
                sleepTime = 6 + tmp_time;
                tmp_time -= 1;

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

            awaiter()  {}
            ~awaiter()  {
                sleepTime++;
                // std::cout << "wait time task disconstruct" << std::endl;
            }
        };
        return awaiter{};
    }
};




cppcoro::cancellation_task<void> cancel(cancel_token token) {
    std::cout << "start cancel 1" << std::endl;
    co_await wait_time_task();
    std::cout << "end cancel 1" << std::endl;
}

cppcoro::cancellation_task<void> cancel2(cancel_token token)
{
    std::cout << "start cancel 2" << std::endl;
    co_await cancel(token);
    std::cout << "end cancel 2" << std::endl;
}

cppcoro::sync_task<int> test() {
    cancel_request req;
    auto token = req.token();
    // std::thread([req]() mutable {
    //     //std::cout << std::this_thread::get_id() << std::endl;
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    //     req.request_cancel();
    //     //std::cout << "request finished" << std::endl;
    // }).detach();
    auto [index, tasks] = co_await cppcoro::when_any_ready(&req, cancel(token), cancel(token), cancel(token));
    std::cout << "task finished result: " << index << std::endl;
    co_return 1;
}



int main() {
    auto result = test();
    std::cout << "main resu" <<std::endl;
    while(1){};
}
