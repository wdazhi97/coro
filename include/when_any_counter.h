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

    void on_sub_awaitble_completed() noexcept
    {
        if(count_.fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            await_coroutine_.resume();
        }
        cancel_request_->request_cancel();
    }

    cancel_request* get_cancel_request() { return cancel_request_; }

protected:
    std::atomic<int> count_;     
    std::coroutine_handle<> await_coroutine_;
    cancel_request* cancel_request_;
};

__CPP_CORO_NS_END