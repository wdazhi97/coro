#pragma once

#include "macro.h"
#include <type_traits>
#include <coroutine>

__CPP_CORO_NS_BEGIN
template<typename T>
struct is_coroutine_handle:std::false_type
{
};

template<typename Promise>
struct is_coroutine_handle<std::coroutine_handle<Promise>>:std::true_type
{};

// 只有void bool 和协程类型 可以作为suspend的返回类型
template<typename T>
struct is_valid_await_suspend_return_value : 
    std::disjunction<std::is_void<T>, std::is_same<bool, T>, is_coroutine_handle<T>>
{
};

template<typename T, typename = std::void_t<>>
struct is_awaiter :std::false_type {};

// awaiter必须实现await系列接口 并且接口返回值类型正确
template<typename T>
struct is_awaiter<T, std::void_t<decltype(std::declval<T>().await_ready()),
    decltype(std::declval<T>().await_suspend()),
    decltype(std::declval<T>().await_resume())>> :
    std::conjunction<std::is_convertible<bool, decltype(std::declval<T>().await_ready())>,
    is_valid_await_suspend_return_value<decltype(std::declval<T>().await_supend(std::declval<std::coroutine_handle>))>>
{};
__CPP_CORO_NS_END