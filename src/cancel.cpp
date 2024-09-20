//
// Created by dazhiwang on 2024/8/22.
//
// cppcoro.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <thread>
#include <chrono>
#include <coroutine>
#include <random>
#include <future>

#include "when_all_ready_task.h"
#include "cancellation_task.hpp"

using namespace cppcoro;

struct outer_task
{
    struct promise_type
    {
        outer_task get_return_object(){ return outer_task{};}
        std::suspend_never initial_suspend() noexcept { return {};}
        std::suspend_never final_suspend() noexcept { return {};}
        void unhandled_exception()
        {
            std::cout << "unhandled_exception" << std::endl;
        }
        void return_void()
        {
        }
    };
};

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
                    //std::cout << "reach the time " << this->sleepTime <<  " " << std::chrono::system_clock::now() <<std::endl;
                    if(!coroutine_handle.done()) {
                        coroutine_handle.resume();
                    }
                }).detach();
            }
            int await_resume() {
                std::cout << "task will resume after " << sleepTime  << "s" << std::endl;
                return sleepTime;
            }
        };
        return awaiter{};
    }
};




cancellation_task<int> cancel(cancel_token token) {
    //std::cout << "start coroutine " << std::chrono::system_clock::now() << std::endl;
    co_await wait_time_task();
    //std::cout << "end coroutine " << std::chrono::system_clock::now() << std::endl;
}

cppcoro::cancellation_task<void> cancel2(cancel_token token)
{
    //std::cout << "start cancel 2" << std::chrono::system_clock::now() << std::endl;
    co_await cancel(token);
    //std::cout << "end cancel 2" << std::chrono::system_clock::now() << std::endl;
}

outer_task outer_tasks()
{
    std::cout << "outer coro begin ...." << std::endl;

    cancel_request req;
    auto token = req.token();
    std::thread([req]() mutable {
        //std::cout << std::this_thread::get_id() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        req.request_cancel();
        //std::cout << "request finished" << std::chrono::system_clock::now() << std::endl;
    }).detach();
    auto [task1, task2, task3] = co_await when_all_ready(
            cancel(token),
            cancel(token),
            cancel(token)
    );

    std::cout << "outer coro end ...." << std::endl;
    co_return;
}

int main()
{
    std::cout << "main begin ...." << std::endl;
    auto outer_task_test = outer_tasks();
    while(true) {
        // do something
    }
}

