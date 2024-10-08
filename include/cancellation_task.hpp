//
// Created by dazhiwang on 2024/8/19.
//

#ifndef COROUTINE_CANCELABLE_TASK_HPP
#define COROUTINE_CANCELABLE_TASK_HPP
#include "macro.h"
#include "cancel_request.h"
#include <coroutine>
__CPP_CORO_NS_BEGIN


enum class task_result_state :uint8_t {
    None,
    Cancel,
    Complete
};

enum class task_state :uint8_t {
    NotStart,
    Start,
    CancelRequest,
    Canceled,
    Complete
};

template<class T>
class cancellation_task;

class cancellation_promise_base{
public:
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

    std::suspend_always initial_suspend() noexcept{return{};}
    final_awaiter final_suspend() noexcept {
        state_ = task_state::Complete;
        if(result_state_ == task_result_state::None)
        {
            result_state_ = task_result_state::Complete;
        }
        return{};
    }

    void set_task_state(task_state state)
    {
        state_ = state;
    }

    void set_task_result_state(task_result_state state)
    {
        result_state_ = state;
    }

    void set_pre_handle(std::coroutine_handle<> awaiting)
    {
        pre_handle = awaiting;
    }

    auto get_task_state()
    {
        return state_;
    }

protected:
    task_result_state result_state_;
    std::coroutine_handle<> pre_handle;
    int32_t call_back_index;
    task_state state_;
};

template<class T>
class cancellation_promise : public cancellation_promise_base{
public:

    class cancellation_task<T> get_return_object();

    template<class ...Arg>
    cancellation_promise(cancel_token token, Arg && ...arg) : token_(token) {
        //std::cout << "init cancel token" << std::endl;
        if(token_.is_cancelled())
        {
            state_ = task_state::Canceled;
            result_state_ = task_result_state::Cancel;
        }
        else
        {
            state_ = task_state::NotStart;
            result_state_ = task_result_state::None;
        }
    };

    void unhandled_exception()
    {
        exception = std::current_exception();
    };

    void return_type(T && in_value){
        if(exception != nullptr){
            std::rethrow_exception(exception);
        }
        value_ =  in_value;
    };

    void register_callback(const std::function<void()> & call_back)
    {
        call_back_index = token_.register_callback(call_back);
    }

    void set_callback_invalid(){
        token_.set_call_invalid(call_back_index);
    }

    bool is_cancelled()
    {
        return token_.is_cancelled();
    }

    void  return_value(T &&in_value) {
        if(exception != nullptr){
            std::rethrow_exception(exception);
        }
        //std::cout << in_value << std::endl;
        value_ =  in_value;
    }

    T& result() &{
        //std::cout << "lvaluse" << std::endl;
        return value_;
    }
    T&& result() && {
        //std::cout << "rvalue" << std::endl;
        return std::move(value_);
    }

private:

    cancel_token token_;
    T value_;
    std::exception_ptr exception = nullptr;
};

template<>
class cancellation_promise<void> : public cancellation_promise_base{

public:
    cancellation_task<void> get_return_object();

    template<class... Arg>
    cancellation_promise(cancel_token token, Arg && ...arg) : token_(token) {
        //std::cout << "init cancel token" << std::endl;
        if(token_.is_cancelled())
        {
            state_ = task_state::Canceled;
            result_state_ = task_result_state::Cancel;
        }
        else
        {
            state_ = task_state::NotStart;
            result_state_ = task_result_state::None;
        }
    };

    void register_callback(const std::function<void()> & call_back)
    {
        call_back_index = token_.register_callback(call_back);
    }

    void set_callback_invalid(){
        token_.set_call_invalid(call_back_index);
    }

    void unhandled_exception()
    {
        exception = std::current_exception();
    };

    bool is_cancelled()
    {
        return token_.is_cancelled();
    }

    void result(){

    }
    void return_value(){};

private:
    cancel_token token_;
    std::exception_ptr exception = nullptr;
};

template<class T>
class cancellation_task {
public:

    using promise_type = cancellation_promise<T>;
    using handle_type = std::coroutine_handle<promise_type>;

    ~cancellation_task(){
        //std::cout << "cancel task destroy" << std::endl;
    }

    cancellation_task(handle_type h):m_handle(h){
        //std::cout << "task construct " << std::endl;
    };

    cancellation_task(const cancellation_task & other) = delete;

    cancellation_task& operator=(const cancellation_task&other) = delete;

    cancellation_task(cancellation_task && other){
        //std::cout << "move task" <<std::endl;
        m_handle = other.m_handle;
        other.m_handle = nullptr;
    }


    auto operator co_await() {
        struct awaiter{
            awaiter(cancellation_task * task,const handle_type & m_handle) : task_(task), m_handle(m_handle)
            {
            };

            auto await_suspend(std::coroutine_handle<> awaiting){
                if(m_handle.promise().get_task_state() == task_state::Canceled ||
                    m_handle.promise().is_cancelled())
                {
                    return awaiting;
                }
                else
                {
                    //std::cout << "add call back " << m_handle.address() << std::endl;
                    m_handle.promise().register_callback([=, this]{
                        task_->on_cancel_request();
                    });
                    m_handle.promise().set_task_state(task_state::Start);
                    m_handle.promise().set_pre_handle(awaiting);
                    return std::coroutine_handle<>::from_address(m_handle.address());
                }
            };

            bool await_ready() {
                return m_handle && m_handle.done();
            };

            decltype(auto) await_resume() {
                if(m_handle.promise().get_task_state() != task_state::CancelRequest)
                {
                    m_handle.promise().set_callback_invalid();
                }
                if(m_handle.promise().is_cancelled())
                {
                    m_handle.promise().set_task_result_state(task_result_state::Cancel);
                }
                return m_handle.promise().result();
            };

            void await_cancellable() { }

            cancellation_task * task_;
            handle_type m_handle;
        };
        return awaiter{this, m_handle};
    }

    /*auto await_suspend(std::coroutine_handle<> awaiting){
        if(m_handle.promise().get_task_state() == task_state::Canceled)
        {
            std::cout << "already cancelled" << std::endl;
            return awaiting;
        }
        else
        {
            std::cout << "add call back " << m_handle.address() << std::endl;
            m_handle.promise().register_callback([=, this]{
                on_cancel_request();
            });
            m_handle.promise().set_task_state(task_state::Start);
            m_handle.promise().set_pre_handle(awaiting);
            return std::coroutine_handle<>::from_address(m_handle.address());
        }
    };

    bool await_ready() {
        return m_handle && m_handle.done();
    };

    T await_resume() {
        if(m_handle.promise().get_task_state() != task_state::CancelRequest)
        {
            m_handle.promise().set_callback_invalid();
        }
        if(m_handle.promise().is_cancelled())
        {
            m_handle.promise().set_task_result_state(task_result_state::Cancel);
        }
        return m_handle.promise().result();
    };*/

    void on_cancel_request(){
        if(m_handle.promise().get_task_state() == task_state::Canceled ||
            m_handle.promise().get_task_state() == task_state::Complete)
        {
            return;
        }
        m_handle.promise().set_task_state(task_state::CancelRequest);
        //直接恢复自己
        if(!m_handle.done())
        {
            m_handle.resume();
        }
    };

private:
    handle_type m_handle;
};

    template<class T>
    cancellation_task<T> cancellation_promise<T>::get_return_object() {
        return std::coroutine_handle<cancellation_promise<T>>::from_promise(*this);
    }

    cancellation_task<void> cancellation_promise<void>::get_return_object() {
        return std::coroutine_handle<cancellation_promise<void>>::from_promise(*this);
    }

__CPP_CORO_NS_END
#endif //COROUTINE_CANCELABLE_TASK_HPP
