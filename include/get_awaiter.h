#pragma once

#include "macro.h"
#include "is_awaiter.h"
#include "any.h"

__CPP_CORO_NS_BEGIN
// T 以成员函数形式重载了operator co_wait
template<typename T>
auto get_awaiter_impl(T &&value, int)
noexcept(noexcept(static_cast<T &&>(value).operator co_await()))
->decltype(static_cast<T &&>(value).operator co_await())
{
    return static_cast<T &&>(value).operator co_await();
}

// T 以非成员函数形式重载了operator co_wait
template<typename T>
auto get_awaiter_impl(T &&value, long)
noexcept(noexcept(operator co_await(static_cast<T&&>(value))))
->decltype(operator co_await(static_cast<T &&>(value)))
{
    return operator co_await(static_cast<T &&>(value));
}

// T为awaiter类型
template<typename T, std::enable_if_t<is_awaiter<T &&>::value, int> = 0>
T &&get_awaiter_impl(T &&value, any) noexcept
{
    return static_cast<T &&>(value);
}

// 获取awaiter工具函数
template<typename T>
auto get_awaiter(T &&value) noexcept(noexcept(get_awaiter_impl(static_cast<T &&>(value), 0)))
->decltype(get_awaiter_impl(static_cast<T &&>(value), 0))
{
    return get_awaiter_impl(static_cast<T &&>(value), 0);
}
__CPP_CORO_NS_END