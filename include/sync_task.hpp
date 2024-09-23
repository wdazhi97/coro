//
// Created by wdazhi on 2024/8/19.
//

#ifndef COROUTINE_SYNC_TASK_HPP
#define COROUTINE_SYNC_TASK_HPP
#include "macro.h"
__CPP_CORO_NS_BEGIN
#include <coroutine>
#include <iostream>
template<class T> class sync_task;

template<class T>
class sync_promise{
public:
    std::suspend_never initial_suspend() noexcept{return{};}
    std::suspend_always final_suspend() noexcept {return{};}

    sync_task<T> get_return_object();

    void unhandled_exception(){}

    void return_value(T && value)
    {
        //std::cout << "sync return " << value << std::endl;
        value_ = value;
        //return{};
    }

    T result(){
        //std::cout << "re " <<  value_ << " " << this << std::endl;
        return value_;
    }
    ~sync_promise(){
        //std::cout << "destructure" << std::endl;
    }

private:



    T value_;
};

template<class T>
class sync_task {
public:
    using promise_type = sync_promise<T>;
    sync_task(std::coroutine_handle<sync_promise<T>> h) : m_handle(h){

    }

    sync_task(const sync_task &) = delete;

    sync_task& operator=(const sync_task &) = delete;

    sync_task(sync_task && other) noexcept :m_handle(other.m_handle) {
        other.m_handle = nullptr;
    }

    void resume(){
        m_handle.resume();
    }
    T result(){
        return m_handle.promise().result();
    }

    auto operator co_await() const {
        struct awaiter {
            explicit awaiter(std::coroutine_handle<promise_type> handle):m_handle(handle){}
            bool await_ready(){
                return m_handle && m_handle.done();
            }
            T await_resume() {return m_handle.promise().result();}
            auto await_suspend(std::coroutine_handle<> h) {return m_handle;}
            std::coroutine_handle<promise_type> m_handle;
        };
        return awaiter(m_handle);
    }

private:
    std::coroutine_handle<promise_type> m_handle;
};

template<class T>
sync_task<T> sync_promise<T>::get_return_object() {
    return std::coroutine_handle<sync_promise<T>>::from_promise(*this);
}
__CPP_CORO_NS_END
#endif //COROUTINE_SYNC_TASK_HPP
