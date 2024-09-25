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


static int task_index = 0;
static std::vector<int> time_vector { 4, 2, 3};

class wait_time_task
{
public:

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
                // if (sleepTime == 7) {
                //     coroutine_handle.resume();
                //     return;
                // }

                std::thread([this, coroutine_handle]() {
                    std::this_thread::sleep_for(std::chrono::seconds(this->sleepTime));
                    if(!coroutine_handle.done()) {
                        std::cout << std::endl;
                        std::cout << "task " << cur_task_index << " reach the time" << std::endl;
                        coroutine_handle.resume();
                    }
                    else {
                        // std::cout << "task " << cur_task_index << " reach the time, do nothing" << std::endl;
                    }
                }).detach();
            }
            int await_resume() {
                std::cout << "task " << cur_task_index << " resume " << std::endl;
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




cppcoro::cancellation_task<int> cancel(cancel_token token) {
    auto x = co_await wait_time_task();
    //std::cout << "end coroutine " << std::chrono::system_clock::now() << std::endl;
    co_return x;
}

cppcoro::cancellation_task<int> cancel2(cancel_token token)
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
    // std::cout << "Task0:" << std::get<0>(tasks) << " Task1:"  << std::get<1>(tasks)  << " Task2:" << std::get<2>(tasks) << " " <<std::endl;
    std::cout << std::endl << "when_any finished, success index: " << index << std::endl;
    co_return 1;
}



int main() {
    auto result = test();
    //std::cout << "main resu" <<std::endl;
    while(1){};
}
