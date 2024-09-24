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
        return count_-- > 1;
    }

    void on_sub_awaitble_completed(size_t index) noexcept
    {
        if(count_-- == 1)
        {
            await_coroutine_.resume();
        }
        if (!cancel_request_->is_cancelled()) {
            result_index = index;
            cancel_request_->request_cancel();
        }

    }

    cancel_request* get_cancel_request() { return cancel_request_; }

    size_t get_result_index() const { return result_index; }

    size_t get_cur_index() { return cur_index++; }

protected:
    size_t count_;
    std::coroutine_handle<> await_coroutine_;
    cancel_request* cancel_request_;
    size_t result_index;

    size_t cur_index = 0;
};

__CPP_CORO_NS_END