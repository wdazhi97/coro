#pragma once

#include "macro.h"
#include "get_awaiter.h"

__CPP_CORO_NS_BEGIN

template<typename T, typename = void>
struct awaitable_traits
{};

template<typename T>
struct awaitable_traits<T, std::void_t<decltype(get_awaiter(std::declval<T>()))>>
{
    using awaiter_t = decltype(get_awaiter(std::declval<T>()));
    using await_result_t = decltype(std::declval<awaiter_t>().await_resume());
};

__CPP_CORO_NS_END
