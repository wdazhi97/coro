// cppcoro.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

// ReSharper disable All
#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <coroutine>
#include <sync_task.hpp>
#include <task.hpp>
#include <when_all_ready_task.h>

#include "cancellation_task.hpp"
#include "when_any_ready_task.h"


static int task_index = 0;
static std::vector<int> time_vector { 0, 1, 2, 3, 4, 5, 6, 7};

class wait_time_task
{
public:
    auto operator co_await() const &noexcept
    {
        struct awaiter {
            int sleepTime = 0;
            int cur_task_index = 0;

            bool await_ready() {
                return false;
            }
            void await_suspend(std::coroutine_handle<> coroutine_handle) {
                sleepTime = time_vector[task_index];
                cur_task_index = task_index;
                task_index++;

                std::cout << "task " << cur_task_index << " start, will resume after " << sleepTime << " second "<< std::endl;

                std::thread([this, coroutine_handle]() {
                    std::this_thread::sleep_for(std::chrono::seconds(this->sleepTime));
                    if(!coroutine_handle.done()) {
                        std::cout << std::endl;
                        std::cout << "task " << cur_task_index << " reach the time" << std::endl;
                        coroutine_handle.resume();
                    }
                }).detach();
            }
            int await_resume() {
                // std::cout << "task " << cur_task_index << " resume " << std::endl;
                return sleepTime;
            }
        };
        return awaiter{};
    }
};

cppcoro::cancellation_task<int> sub_task_level2(cancel_token token)
{
    auto result = co_await wait_time_task();
    co_return result;
}

cppcoro::cancellation_task<int> sub_task_level1(cancel_token token)
{
    auto result = co_await sub_task_level2(token);
    result += co_await wait_time_task();
    co_return result;
}

cppcoro::cancellation_task<std::tuple<int, int>> test_when_all_task(cancel_token token)
{
    auto when_all_results = co_await cppcoro::when_all_ready(
        sub_task_level1(token),
        sub_task_level1(token));
    co_return when_all_results;
}

cppcoro::sync_task<void> test() {
    cancel_request req;
    auto token = req.token();

    auto [index, tasks] =
        co_await cppcoro::when_any_ready(&req,
            test_when_all_task(token),
            test_when_all_task(token));
}



int main() {
    auto result = test();
    while(1){};
}


