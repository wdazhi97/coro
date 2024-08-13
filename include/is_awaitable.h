#pragma once

#include "get_awaiter.h"
#include <type_traits>

__CPP_CORO_NS_BEGIN

template<typename T, typename = std::void_t<>>
struct is_awaitable : std::false_type {};

template<typename T>
struct is_awaitable<T, std::void_t<decltype(get_awaiter(std::declval<T>()))>>
    :std::true_type
{};

template<typename T>
constexpr bool is_awaitable_v = is_awaitable<T>::value;

__CPP_CORO_NS_END