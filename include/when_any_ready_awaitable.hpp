#pragma once
#include "macro.h"
#include "when_any_counter.h"
#include <tuple>

__CPP_CORO_NS_BEGIN


// 类模板定义
template<typename TaskContainer>
class when_any_ready_awaitble;

// void类型特化 递归基
template<>
class when_any_ready_awaitble<std::tuple<>>
{
    constexpr when_any_ready_awaitble() noexcept {};
    explicit constexpr when_any_ready_awaitble(std::tuple<>) noexcept {}
    constexpr bool await_ready() { return true; }
    void await_suspend(std::coroutine_handle<>) noexcept {};
    std::tuple<> await_resume() const noexcept { return {}; }
};

// 可变参数模板特化
template<typename ... Tasks>
class when_any_ready_awaitble<std::tuple<Tasks ...>>
{
public:

    using tasks_return_type = std::tuple<typename std::conditional<std::is_void<typename Tasks::return_type>::value,void_value, typename std::remove_reference<typename Tasks::return_type>::type>::type ...>;
    using return_type = std::tuple<size_t, tasks_return_type>;

    explicit when_any_ready_awaitble(cancel_request* cancel_request, Tasks&& ...tasks)
        noexcept(std::conjunction_v<std::is_nothrow_move_constructible<Tasks>...>)
        :counter_(sizeof...(Tasks), cancel_request)
        ,tasks_(std::move(tasks)...)
    {}

    explicit when_any_ready_awaitble(cancel_request* cancel_request, std::tuple<Tasks ...> &&tasks)
        :counter_(sizeof...(Tasks), cancel_request)
        ,tasks_(std::move(tasks))
    {}

    when_any_ready_awaitble(when_any_ready_awaitble &&other)
        :counter_(sizeof...(Tasks), other.counter_.get_cancel_request())
        , tasks_(std::move(other.tasks_))
    {}

    auto operator co_await() & noexcept
    {
        struct awaiter
        {
            awaiter(when_any_ready_awaitble &awaitable) noexcept
                :awaitable_(awaitable) {}

            bool await_ready() const noexcept
            {
                return awaitable_.is_ready();
            }

            bool await_suspend(std::coroutine_handle<> awaitingCoro) noexcept
            {
                return awaitable_.try_await(awaitingCoro);
            }

            return_type await_resume() noexcept
            {
                return std::make_tuple(awaitable_.counter_.get_result_index(), awaitable_.get_result(std::make_integer_sequence<std::size_t, sizeof...(Tasks)>{}));
            }
        private:
            when_any_ready_awaitble &awaitable_;
        };

        return awaiter{ *this };
    }

    auto operator co_await() && noexcept
    {
        struct awaiter
        {
            awaiter(when_any_ready_awaitble &awaitable) noexcept
                :awaitable_(awaitable) {}

            bool await_ready() const noexcept
            {
                return awaitable_.is_ready();
            }

            bool await_suspend(std::coroutine_handle<> awaitingCoro) noexcept
            {
                return awaitable_.try_await(awaitingCoro);
            }

            //返回右值引用有问题
            return_type await_resume() noexcept
            {
                return std::move(std::make_tuple(awaitable_.counter_.get_result_index(), awaitable_.get_result(std::make_integer_sequence<std::size_t, sizeof...(Tasks)>{})));
            }
        private:
            when_any_ready_awaitble &awaitable_;
        };

        return awaiter{ *this };
    }

private:
    bool is_ready() const
    {
        return counter_.is_ready();
    }

    bool try_await(std::coroutine_handle<> awaitingCoro) noexcept
    {
        start_tasks(std::make_integer_sequence<std::size_t, sizeof...(Tasks)>{});

        return counter_.try_await(awaitingCoro);
    }

    template<std::size_t ...INDEX>
    void start_tasks(std::integer_sequence<std::size_t, INDEX...>) noexcept
    {
        ((std::get<INDEX>(tasks_).start(counter_)), ...);
    }

    template<std::size_t ...INDEX>
    tasks_return_type get_result(std::integer_sequence<std::size_t, INDEX...>) noexcept
    {
        return std::make_tuple(std::get<INDEX>(tasks_).non_void_result()...);
    }

public:
    when_any_counter counter_;
    std::tuple<Tasks ...> tasks_;
};

// 容器类型特化
template<typename TaskContainer>
class when_any_ready_awaitble
{
public:
    explicit when_any_ready_awaitble(cancel_request* cancel_request, TaskContainer&& tasks)
        : counter_(tasks.size(), cancel_request)
        , tasks_(std::forward<TaskContainer>(tasks))
    {}

    when_any_ready_awaitble(when_any_ready_awaitble &&other)
        noexcept(std::is_nothrow_move_constructible_v<TaskContainer>)
        :counter_(other.tasks_.size(), other.counter_.get_cancel_request())
        , tasks_(std::move(other.tasks_))
    {}

    when_any_ready_awaitble(const when_any_ready_awaitble &) = delete;
    when_any_ready_awaitble &operator = (const when_any_ready_awaitble &) = delete;

    auto operator co_await() & noexcept
    {
        struct awaiter
        {
            awaiter(when_any_ready_awaitble &awaitable) noexcept
                :awaitable_(awaitable) {}

            bool await_ready() const noexcept
            {
                return awaitable_.is_ready();
            }

            bool await_suspend(std::coroutine_handle<> awaitingCoro) noexcept
            {
                return awaitable_.try_await(awaitingCoro);
            }

            TaskContainer &await_resume() noexcept
            {
                return awaitable_.tasks_;
            }
        private:
            when_any_ready_awaitble &awaitable_;
        };

        return awaiter{ *this };
    }

    auto operator co_await() && noexcept
    {
        struct awaiter
        {
            awaiter(when_any_ready_awaitble &awaitable) noexcept
                :awaitable_(awaitable) {}

            bool await_ready() const noexcept
            {
                return awaitable_.is_ready();
            }

            bool await_suspend(std::coroutine_handle<> awaitingCoro) noexcept
            {
                return awaitable_.try_await(awaitingCoro);
            }

            TaskContainer &&await_resume() noexcept
            {
                return std::move(awaitable_.tasks_);
            }
        private:
            when_any_ready_awaitble &awaitable_;
        };

        return awaiter{ *this };
    }

private:
    bool is_ready() const
    {
        return counter_.is_ready();
    }

    bool try_await(std::coroutine_handle<> awaitingCoro) noexcept
    {
        for (auto &&task : tasks_)
        {
            task.start(counter_);
        }

        return counter_.try_await(awaitingCoro);
    }

public:
    when_any_counter counter_;
    TaskContainer tasks_;
};

__CPP_CORO_NS_END