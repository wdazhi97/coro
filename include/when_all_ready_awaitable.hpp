#pragma once
#include "macro.h"
#include "when_all_counter.h"
#include <tuple>

__CPP_CORO_NS_BEGIN


// 类模板定义
template<typename TaskContainer>
class when_all_ready_awaitable;

// void类型特化 递归基
template<>
class when_all_ready_awaitable<std::tuple<>>
{
    constexpr when_all_ready_awaitable() noexcept {};
    explicit constexpr when_all_ready_awaitable(std::tuple<>) noexcept {}
    constexpr bool await_ready() { return true; }
    void await_suspend(std::coroutine_handle<>) noexcept {};
    std::tuple<> await_resume() const noexcept { return {}; }
};

// 可变参数模板特化
template<typename ... Tasks>
class when_all_ready_awaitable<std::tuple<Tasks ...>>
{
public:

    using return_type = std::tuple<typename std::conditional<std::is_void<typename Tasks::return_type>::value,void_value, typename Tasks::return_type>::type ...>;
    explicit when_all_ready_awaitable(Tasks&& ...tasks)
        noexcept(std::conjunction_v<std::is_nothrow_move_constructible<Tasks>...>)
        :counter_(sizeof...(Tasks))
        ,tasks_(std::move(tasks)...)
    {}

    explicit when_all_ready_awaitable(std::tuple<Tasks ...> &&tasks)
        :counter_(sizeof...(Tasks))
        ,tasks_(std::move(tasks))
    {}

    when_all_ready_awaitable(when_all_ready_awaitable &&other)
        :counter_(sizeof...(Tasks))
        , tasks_(std::move(other.tasks_))
    {}

    auto operator co_await() & noexcept
    {
        struct awaiter
        {
            awaiter(when_all_ready_awaitable &awaitable) noexcept
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
                return awaitable_.get_result(std::make_integer_sequence<std::size_t, sizeof...(Tasks)>{});
            }
        private:
            when_all_ready_awaitable &awaitable_;
        };

        return awaiter{ *this };
    }

    auto operator co_await() && noexcept
    {
        struct awaiter
        {
            awaiter(when_all_ready_awaitable &awaitable) noexcept
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
                return std::move(awaitable_.get_result(std::make_integer_sequence<std::size_t, sizeof...(Tasks)>{}));
            }
        private:
            when_all_ready_awaitable &awaitable_;
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
    return_type get_result(std::integer_sequence<std::size_t, INDEX...>) noexcept
    {
        return std::make_tuple(std::get<INDEX>(tasks_).non_void_result()...);
    }

public:
    when_all_counter counter_;
    std::tuple<Tasks ...> tasks_;
};

// 容器类型特化
template<typename TaskContainer>
class when_all_ready_awaitable
{
public:
    explicit when_all_ready_awaitable(TaskContainer&& tasks)
        : counter_(tasks.size())
        , tasks_(std::forward<TaskContainer>(tasks))
    {}

    when_all_ready_awaitable(when_all_ready_awaitable &&other)
        noexcept(std::is_nothrow_move_constructible_v<TaskContainer>)
        :counter_(other.tasks_.size())
        , tasks_(std::move(other.tasks_))
    {}

    when_all_ready_awaitable(const when_all_ready_awaitable &) = delete;
    when_all_ready_awaitable &operator = (const when_all_ready_awaitable &) = delete;

    auto operator co_await() & noexcept
    {
        struct awaiter
        {
            awaiter(when_all_ready_awaitable &awaitable) noexcept
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
            when_all_ready_awaitable &awaitable_;
        };

        return awaiter{ *this };
    }

    auto operator co_await() && noexcept
    {
        struct awaiter
        {
            awaiter(when_all_ready_awaitable &awaitable) noexcept
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
            when_all_ready_awaitable &awaitable_;
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
    when_all_counter counter_;
    TaskContainer tasks_;
};

__CPP_CORO_NS_END