// cppcoro.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <thread>
#include <chrono>
#include <coroutine>
#include <random>
#include <future>

#include "when_all_ready_task.h"

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

int getrandint(int min, int max) {
    std::random_device rd;  //如果可用的话，从一个随机数发生器上获得一个真正的随机数
    std::mt19937 gen(rd()); //gen是一个使用rd()作种子初始化的标准梅森旋转算法的随机数发生器
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}

static int global_task_count = 1;

template<typename T>
class async_awaiter_base {
public:
    int sleepTime = 0;
    int cur_task_count = 0;

    bool await_ready() {
        return false;
    }

    void await_suspend(std::coroutine_handle<> coroutine_handle) {
        sleepTime = getrandint(5, 10);
        std::thread([this, coroutine_handle]() {
           std::this_thread::sleep_for(std::chrono::seconds(this->sleepTime));
           coroutine_handle.resume();
        }).detach();
        std::cout << "task " << cur_task_count <<  " suspend " << std::endl;
    }

    void await_resume() {
        std::cout << "task " << cur_task_count <<  " resume after " << sleepTime  << "s" << std::endl;
    }

    async_awaiter_base(){
        cur_task_count = global_task_count;
        global_task_count++;
    }
};


template<typename T>
class async_awaiter : public async_awaiter_base<T> {

};

template<>
class async_awaiter<int> : public async_awaiter_base<int> {
public:
    int await_resume() {
        std::cout << "task " << cur_task_count <<  " resume after " << sleepTime  << "s" << std::endl;
        return sleepTime;
    }
};

template<>
class async_awaiter<float> : public async_awaiter_base<float> {
public:
    float await_resume() {
        std::cout << "task " << cur_task_count <<  " resume after " << sleepTime  << "s" << std::endl;
        return (float) sleepTime;
    }
};

template<>
class async_awaiter<std::string> : public async_awaiter_base<float> {
public:
    explicit async_awaiter(std::string&& result) : _result(result) {}
    std::string _result;

    std::string await_resume() {
        std::cout << "task " << cur_task_count <<  " resume after " << sleepTime  << "s" << std::endl;
        return _result;
    }
};

template<typename T>
auto operator co_await(async_awaiter<T> awaiter) {
    return awaiter;
}

outer_task outer_tasks()
{
    std::cout << "outer coro begin ...." << std::endl;

    auto [task1, task2, task3] = co_await when_all_ready(
        async_awaiter<int> (),
        async_awaiter<float> (),
        async_awaiter<std::string> ("test await")
        );

    std::cout << "task1 result " << task1 << std::endl;
    std::cout << "task2 result " << task2 << std::endl;
    std::cout << "task3 result " << task3 << std::endl;
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

