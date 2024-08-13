#pragma once
#include "macro.h"
#include <type_traits>

__CPP_CORO_NS_BEGIN
template<typename T>
struct unwrap_reference
{
    using type = T;
};

template<typename T>
struct unwrap_reference<std::reference_wrapper<T>>
{
    using type = T;
};

template<typename T>
using unwrap_reference_t = unwrap_reference<T>::type;

__CPP_CORO_NS_END