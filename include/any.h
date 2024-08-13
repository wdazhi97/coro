#pragma once

#include "macro.h"

__CPP_CORO_NS_BEGIN
struct any 
{
    template<typename T>
    any(T &&) noexcept
    {};
};

__CPP_CORO_NS_END
