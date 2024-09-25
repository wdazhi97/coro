#pragma once

#include "macro.h"
#include <atomic>
#include <coroutine>

__CPP_CORO_NS_BEGIN

class when_all_counter
{
public:
    when_all_counter(size_t count) noexcept
    :count_(count + 1), await_coroutine_(nullptr) {}
    
    bool is_ready() const noexcept
    {
        return await_coroutine_ != nullptr;
    }

    bool try_await(std::coroutine_handle<> awaitCoroutine)
    {
        await_coroutine_ = awaitCoroutine;
        return count_-- > 1;
    }

    void on_sub_awaitable_completed() noexcept
    {
        if(count_-- == 1)
        {
            await_coroutine_.resume();
        }
    }
protected:
    size_t count_;
    std::coroutine_handle<> await_coroutine_;
};

__CPP_CORO_NS_END