// cppcoro.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <thread>
#include <chrono>
#include "when_all_ready_task.h"

using namespace cppcoro;

#include <iostream>
#include <coroutine>
#include <future>
#include <thread>
#include <chrono>

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
                srand(static_cast<unsigned int>(time(nullptr)));
                sleepTime = rand() % 10 + 1;
                std::async([=]() {
                    std::this_thread::sleep_for(std::chrono::seconds(sleepTime));
                    coroutine_handle.resume();
                    });
            }
            int await_resume() {
                std::cout << "task resume after " << sleepTime  << "s" << std::endl;
                return sleepTime;
            }
        };
        return awaiter{};
    }
};

wait_time_task wait_time_task_func()
{
    co_return;
}

outer_task outer_tasks()
{
    std::cout << "outer coro begin ...." << std::endl;
    auto [task1, task2] = co_await when_all_ready(wait_time_task_func(), wait_time_task_func());
    std::cout << "task1 sleep time " << task1.result() << "s" << std::endl;
    std::cout << "task2 sleep time " << task2.result() << "s" << std::endl;
    std::cout << "outer coro end ...." << std::endl;
    co_return;
}



int main()
{
    std::cout << "main begin ...." << std::endl;
    auto outer_task_test = outer_tasks();
    std::cout << "main end ...." << std::endl;
    system("pause");
}

