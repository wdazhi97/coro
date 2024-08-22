//
// Created by dazhiwang on 2024/8/21.
//

#ifndef COROUTINE_WHEN_ANY_COUNTER_HPP
#define COROUTINE_WHEN_ANY_COUNTER_HPP
#include "macro.h"
#include <atomic>
#include <coroutine>

__CPP_CORO_NS_BEGIN

    class when_any_counter
    {
    public:
        when_any_counter(size_t count) noexcept
                :count_(count + 1), await_coroutine_(nullptr) {}

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
        }
    protected:
        std::atomic<int> count_;
        std::coroutine_handle<> await_coroutine_;
    };

__CPP_CORO_NS_END
#endif //COROUTINE_WHEN_ANY_COUNTER_HPP
