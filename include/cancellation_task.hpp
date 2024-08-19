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
class cancellation_promise {
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

    std::suspend_always initial_suspend() noexcept{return{};}
    final_awaiter final_suspend() noexcept { state_ == task_state::Complete; return{};}

    class cancellation_task get_return_object();

    cancellation_promise(cancel_token token) : token_(token) {
        std::cout << "init cancel token" << std::endl;
        if(token_.is_cancelled())
        {
            state_ = task_state::Canceled;
        }
        else
        {
            state_ = task_state::NotStart;
        }
    };

    void unhandled_exception(){};

    void return_type(){};

    auto get_task_state()
    {
        return state_;
    }

    void set_task_state(task_state state)
    {
        state_ = state;
    }

    void set_pre_handle(std::coroutine_handle<> awaiting)
    {
        pre_handle = awaiting;
    }

    void register_callback(const std::function<void()> & call_back)
    {
        call_back_index = token_.register_callback(call_back);
    }

    void set_callback_invalid(){
        token_.set_call_invalid(call_back_index);
    }

    void return_void(){};

private:

    std::coroutine_handle<> pre_handle;
    task_result_state result_state;
    cancel_token token_;
    task_state state_;
    int call_back_index;
};


class cancellation_task {
public:

    using promise_type = cancellation_promise<void>;
    using handle_type = std::coroutine_handle<promise_type>;

    ~cancellation_task(){
        std::cout << "cancel task destroy" << std::endl;
    }

    cancellation_task(handle_type h):m_handle(h){

    };

    /*cancellation_task(cancel_token token): token_(token){
        if(token.is_cancelled())
        {
            state_ = task_state::Canceled;
        }
        state_ = task_state::NotStart;
    };*/
    auto await_suspend(std::coroutine_handle<> awaiting){
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
        return false;
    };

    void await_resume() {
        if(m_handle.promise().get_task_state() != task_state::CancelRequest)
        {
            m_handle.promise().set_callback_invalid();
        }
        std::cout << "cancel task resume " << std::endl;
    };

    void on_cancel_request(){
        std::cout << "start cancel request " << m_handle.address() << std::endl;
        if(m_handle.promise().get_task_state() == task_state::Canceled || m_handle.promise().get_task_state() == task_state::Complete)
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


    handle_type m_handle;
};

    template<class T>
    class cancellation_task cancellation_promise<T>::get_return_object() {
        return std::coroutine_handle<cancellation_promise<void>>::from_promise(*this);
    }

__CPP_CORO_NS_END
#endif //COROUTINE_CANCELABLE_TASK_HPP
