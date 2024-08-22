#pragma once

#include "macro.h"
#include "when_any_counter.h"
#include "void_value.h"
#include "awaitable_traits.h"

#include <coroutine>
#include <exception>
#include <assert.h>
#include <utility>

__CPP_CORO_NS_BEGIN

template<typename TaskContainer>
class when_any_ready_awaitble;

template<typename Result>
class when_any_task_promise final
{
public:
    using corotinue_handle_t = std::coroutine_handle<when_any_task_promise<Result>>;
    when_any_task_promise() noexcept {};
    auto get_return_object() noexcept {
        return corotinue_handle_t::from_promise(*this);
    }

    std::suspend_always initial_suspend() noexcept { return {}; }

    auto final_suspend() noexcept
    {
        class task_finish_notifier
        {
        public:
            bool await_ready() const noexcept { return false; }
            void await_suspend(corotinue_handle_t coro) const noexcept
            {
                return coro.promise().counter_->on_sub_awaitble_completed(coro.promise().index_);
            }
            void await_resume() const noexcept {}
        };

        return task_finish_notifier{};
    }

    void unhandled_exception() noexcept
    {
        exception_ = std::current_exception();
    }

    void return_void()
    {
        assert(false);
    }

    auto yield_value(Result &&result) noexcept
    {
        result_ = std::addressof(result);
        return final_suspend();
    }

    void start(when_any_counter &counter) noexcept
    {
        counter_ = &counter;
        index_ = counter.get_cur_index();
        corotinue_handle_t::from_promise(*this).resume();
    }

    Result &result() &
    {
        rethrow_exception();
        return *result_;
    }

    Result &&result() &&
    {
        rethrow_exception();
        return std::forward<Result>(*result_);
    }
private:
    void rethrow_exception()
    {
        if (exception_)
        {
            std::rethrow_exception(exception_);
        }
    }
private:
    when_any_counter *counter_;
    int index_;
    std::exception_ptr exception_;
    std::add_pointer_t<Result> result_;
};


// void 特化
template<>
class when_any_task_promise<void> final
{
public:
    using corotinue_handle_t = std::coroutine_handle<when_any_task_promise<void>>;
    when_any_task_promise() noexcept {};
    auto get_return_object() noexcept {
        return corotinue_handle_t::from_promise(*this);
    }

    std::suspend_always initial_suspend() noexcept { return {}; }

    auto final_suspend() noexcept
    {
        class task_finish_notifier
        {
        public:
            bool await_ready() const noexcept { return false; }
            void await_suspend(corotinue_handle_t coro) const noexcept
            {
                return coro.promise().counter_->on_sub_awaitble_completed(coro.promise().index_);
            }
            void await_resume() const noexcept {}
        };

        return task_finish_notifier{};
    }

    void unhandled_exception() noexcept
    {
        exception_ = std::current_exception();
    }

    void return_void()
    {
    }

    void start(when_any_counter &counter) noexcept
    {
        counter_ = &counter;
        index_ = counter.get_cur_index();
        corotinue_handle_t::from_promise(*this).resume();
    }

    void result()
    {
        if (exception_)
        {
            std::rethrow_exception(exception_);
        }
    }

private:
    when_any_counter *counter_;
    int index_;
    std::exception_ptr exception_;
};

// task定义
template<typename Result>
class when_any_task
{
public:
    using promise_type = when_any_task_promise<Result>;
    using coroutine_handle_t = typename promise_type::corotinue_handle_t;
    using return_type = Result;

    when_any_task(coroutine_handle_t coro) noexcept
        :coroutine_(coro) {};

    when_any_task(when_any_task &&other) noexcept
        :coroutine_(std::exchange(other.coroutine_, coroutine_handle_t{})) {}

    ~when_any_task()
    {
        if (coroutine_)
        {
            coroutine_.destroy();
        }
    }

    when_any_task(when_any_task &) = delete;
    when_any_task &operator = (const when_any_task &) = delete;

    decltype(auto) result() &
    {
        return coroutine_.promise().result();
    }

    decltype(auto) result() &&
    {
        return std::move(coroutine_.promise()).result();
    }

    decltype(auto) non_void_result() &
    {
        if constexpr (std::is_void_v<decltype(this->result())>)
        {
            this->result();
            return void_value{};
        }
        else
        {
            return this->result();
        }
    }

    decltype(auto) non_void_result() &&
    {
        if constexpr (std::is_void_v<decltype(this->result())>)
        {
            std::move(*this).result();
            return void_value{};
        }
        else
        {
            return std::move(*this).result();
        }
    }
private:
    template<typename TaskContainer>
    friend class when_any_ready_awaitble;
    void start(when_any_counter &counter) noexcept
    {
        coroutine_.promise().start(counter);
    }
private:
    coroutine_handle_t coroutine_;
};

template<typename Awaitable,
         typename Result = typename awaitable_traits<Awaitable &&>::await_result_t,
         std::enable_if_t<!std::is_void_v<Result>, int> = 0>
    when_any_task<Result> make_when_any_task(Awaitable awaitable)
{
    co_yield co_await static_cast<Awaitable &&>(awaitable);
}

template<typename Awaitable,
    typename Result = typename awaitable_traits<Awaitable &&>::await_result_t,
    std::enable_if_t<std::is_void_v<Result>, int> = 0>
    when_any_task<Result> make_when_any_task(Awaitable awaitable)
{
    co_await static_cast<Awaitable &&>(awaitable);
}

template<typename Awaitable,
    typename Result = typename awaitable_traits<Awaitable &&>::await_result_t,
    std::enable_if_t<!std::is_void_v<Result>, int> = 0>
    when_any_task<Result> make_when_any_task(std::reference_wrapper<Awaitable> awaitable)
{
    co_yield co_await awaitable.get();
}

template<typename Awaitable,
    typename Result = typename awaitable_traits<Awaitable &&>::await_result_t,
    std::enable_if_t<std::is_void_v<Result>, int> = 0>
    when_any_task<Result> make_when_any_task(std::reference_wrapper<Awaitable> awaitable)
{
    co_await awaitable.get();
}

__CPP_CORO_NS_END