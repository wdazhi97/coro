//
// Created by wdazhi on 2024/8/19.
//

#ifndef MTCORO_TASK_HPP
#define MTCORO_TASK_HPP
#include <coroutine>
#include <iostream>
#include "macro.h"
__CPP_CORO_NS_BEGIN
template<class T>
class task;


class promise_base{
    class final_awaiter{
    public:
        bool await_ready() noexcept {return false;}

        template<class promise>
        auto await_suspend(std::coroutine_handle<promise> h) noexcept{
            return h.promise().pre_handle;
        }
        void await_resume() noexcept{}
    };
public:

    std::suspend_always initial_suspend() noexcept {return{};}
    final_awaiter final_suspend() noexcept {return{};}

    void set_pre_handle(std::coroutine_handle<> h){
        pre_handle = h;
    }

protected:
    std::coroutine_handle<> pre_handle;
};

template<class T>
class task_promise_type final : public promise_base {
public:

    void  return_value(T &&in_value) {
        if(exception != nullptr){
            std::rethrow_exception(exception);
        }
        //std::cout << in_value << std::endl;
        value =  in_value;
    }

    task<T> get_return_object();

    void unhandled_exception() {
        exception = std::current_exception();
    }

    T& result() &{
        //std::cout << "lvaluse" << std::endl;
        return value;
    }
    T&& result() && {
        //std::cout << "rvalue" << std::endl;
        return std::move(value);
    }

    ~task_promise_type(){
        //std::cout << "promise des" << std::endl;
    }

private:


    T value;
    std::exception_ptr exception = nullptr;
};

template<>
class task_promise_type<void> final : public promise_base{
public:
    task<void> get_return_object();

    void return_value() noexcept {}

    void result() {

    }

    void unhandled_exception(){
        exception = std::current_exception();
    }
private:
    std::exception_ptr exception = nullptr;
};

template<class T>
class task{
public:
    using promise_type = task_promise_type<T>;
    using handle_type= std::coroutine_handle<promise_type>;
    task(handle_type h):m_handle(h) {

    }

    task(const task & ) = delete;

    task& operator=(const task &) = delete;

    task(task && other) {
        m_handle = other.m_handle;
        other.m_handle = nullptr;
    }

    task& operator=(const task && other){
        m_handle = other.m_handle;
        other.m_handle = nullptr;
        return *this;
    }

    auto operator co_await () const & {
        return awaiter{m_handle};
    }

    auto operator co_await () const && {
        return awaiter{m_handle};
    }

private:

    class awaiter{
    public:
        bool await_ready(){
            return m_coroutine && m_coroutine.done();
        }

        awaiter(handle_type h) : m_coroutine(h){}

        std::coroutine_handle<promise_type> await_suspend(std::coroutine_handle<> awaiting){
            m_coroutine.promise().set_pre_handle(awaiting);
            return m_coroutine;
        }

        T await_resume() {
            return m_coroutine.promise().result();
        }

    protected:
        handle_type m_coroutine;
    };

    handle_type m_handle;

};

template<class T>
task<T> task_promise_type<T>::get_return_object()
{
    return std::coroutine_handle<task_promise_type<T>>::from_promise(*this);
}

task<void> task_promise_type<void>::get_return_object() {
    return std::coroutine_handle<task_promise_type<void>>::from_promise(*this);
}
__CPP_CORO_NS_END
#endif //MTCORO_TASK_HPP
