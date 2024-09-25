#pragma once

#include "get_awaiter.h"
#include <type_traits>

__CPP_CORO_NS_BEGIN

template<typename T, typename = std::void_t<>>
struct is_awaitable_cancelable : std::false_type {};

template<typename T>
struct is_awaitable_cancelable<T, std::void_t<decltype(get_awaiter(std::declval<T>())),
    decltype(get_awaiter(std::declval<T>()).await_cancellable())>>
    : std::true_type
{};

template<typename T>
constexpr bool is_awaitable_cancelable_v = is_awaitable_cancelable<T>::value;

__CPP_CORO_NS_END