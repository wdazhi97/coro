#pragma once

#include "macro.h"
#include <atomic>
#include <coroutine>
#include "cancel_request.h"

__CPP_CORO_NS_BEGIN

class when_any_counter
{
public:
    when_any_counter(size_t count, cancel_request* cancel_request) noexcept
    :count_(count + 1), await_coroutine_(nullptr), cancel_request_(cancel_request) {}
    
    bool is_ready() const noexcept
    {
        return await_coroutine_ != nullptr;
    }

    bool try_await(std::coroutine_handle<> awaitCoroutine)
    {
        await_coroutine_ = awaitCoroutine;
        return count_.fetch_sub(1, std::memory_order_acq_rel) > 1;
    }

    void on_sub_awaitble_completed(int index) noexcept
    {
        if(count_.fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            await_coroutine_.resume();
        }
        if (!cancel_request_->is_cancelled()) {
            result_index = index;
            cancel_request_->request_cancel();
        }

    }

    cancel_request* get_cancel_request() { return cancel_request_; }

    int get_result_index() const {return result_index;}

    int get_cur_index() {return cur_index++;}

protected:
    std::atomic<int> count_;     
    std::coroutine_handle<> await_coroutine_;
    cancel_request* cancel_request_;
    int result_index;

    int cur_index = 0;
};

__CPP_CORO_NS_END