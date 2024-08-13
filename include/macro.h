#pragma once

// cpp coro library namespace macros(begin/end) define.
#define __CPP_CORO_NS_BEGIN \
    namespace cppcoro {     \

#define __CPP_CORO_NS_END  }


#define DISABLE_COPY_CONSTRUCT_ASSIGN(Class) \
Class(const Class &) = delete;               \
Class &operator = (const Class &) = delete;  \