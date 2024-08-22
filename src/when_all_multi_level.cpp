// cppcoro.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <thread>
#include <chrono>
#include <ctime>
#include <random>
#include <coroutine>

#include "when_all_ready_task.h"

using namespace cppcoro;

struct main_task
{
    struct promise_type
    {
        main_task get_return_object(){ return main_task{};}
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

int getrandint(int min, int max) {
    std::random_device rd;  //如果可用的话，从一个随机数发生器上获得一个真正的随机数
    std::mt19937 gen(rd()); //gen是一个使用rd()作种子初始化的标准梅森旋转算法的随机数发生器
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}

class wait_task
{
public:
    struct promise_type
    {
        std::suspend_never initial_suspend() noexcept {  return {}; }
        std::suspend_never final_suspend() noexcept {
            pre_handle.resume();
            return {};
        }
        wait_task get_return_object() noexcept { return std::coroutine_handle<promise_type>::from_promise(*this); };
        void unhandled_exception() {};
        void return_void() noexcept {}

        std::coroutine_handle<> pre_handle;

        void set_pre_handle(std::coroutine_handle<> h){
            pre_handle = h;
        }
    };
    using handle_type= std::coroutine_handle<promise_type>;

public:

    class awaiter{
    public:
        awaiter(handle_type h) : m_coroutine(h){}

        bool await_ready(){
            return false;
        }

        void await_suspend(std::coroutine_handle<> awaiting){
            m_coroutine.promise().set_pre_handle(awaiting);
        }

        void await_resume() {
        }

    protected:
        handle_type m_coroutine;
    };

    awaiter operator co_await() const &noexcept
    {
        return awaiter{m_handle};
    }

    wait_task(handle_type handle) : m_handle(handle) {}
    handle_type m_handle;
};

template<typename T>
struct sleepawaiter {
    int sleepTime = 0;

    bool await_ready() {
        return false;
    }
    void await_suspend(std::coroutine_handle<T> coroutine_handle) {
        sleepTime = getrandint(5, 10);
        std::thread([this, coroutine_handle]() {
           std::this_thread::sleep_for(std::chrono::seconds(this->sleepTime));
           coroutine_handle.resume();
        }).detach();
        // std::cout << "set resume after " << std::endl;
    }
    int await_resume() {
        // std::cout << "task resume after " << sleepTime  << "s" << std::endl;
        return sleepTime;
    }
};

static int global_func_count = 1;

wait_task inner_func(int cur_global_func_count) {
    std::cout << "inner func " << cur_global_func_count << " start" << std::endl;
    co_await sleepawaiter<wait_task::promise_type>();
    // std::cout << "inner func " << cur_global_func_count << " end" << std::endl;
}

wait_task outer_func() {
    int cur_global_func_count = global_func_count;
    global_func_count++;

    std::cout << "outer func " << cur_global_func_count << " start" << std::endl;
    const time_t t1 = time(nullptr);
    co_await inner_func(cur_global_func_count);

    const time_t t2 = time(nullptr);
    std::cout << "outer func " << cur_global_func_count << " resume after " << t2 - t1 <<  " s" << std::endl;
    co_await sleepawaiter<wait_task::promise_type>();

    const time_t t3 = time(nullptr);
    std::cout << "outer func " << cur_global_func_count << " end after another " << t3 - t2 <<  " s" << std::endl;
}

// 可以打开，功能是OK的
// wait_time_task wait_time_func() {
//     std::cout << "wait_time_func" << std::endl;
//     co_await sleepawaiter<wait_time_task::promise_type>();
//
//     co_await sleepawaiter<wait_time_task::promise_type>();
// }

main_task outer_tasks()
{
    std::cout << "outer coro begin ...." << std::endl;
    const time_t start_time = time(nullptr);
    co_await when_all_ready(outer_func(), outer_func());

    const time_t end_time = time(nullptr);
    std::cout << "outer coro end after " << end_time - start_time << " s .... " << std::endl;
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

