//
// Created by dazhiwang on 2024/8/21.
//

#ifndef COROUTINE_CANCELLABLE_AWAITER_HPP
#define COROUTINE_CANCELLABLE_AWAITER_HPP
#include "macro.h"
#include "cancel_request.h"
#include <coroutine>
__CPP_CORO_NS_BEGIN

class cancellaction_awaiter {
public:

    cancellaction_awaiter(cancel_token token) : token_(token)
    {

    }

private:
    cancel_token token_;

};

__CPP_CORO_NS_END
#endif //COROUTINE_CANCELLABLE_AWAITER_HPP
