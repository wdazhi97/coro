#pragma once

#include "when_any_task.h"
#include "unwrap_reference.h"
#include "is_awaitable_cancelable.h"
#include "when_any_ready_awaitable.hpp"
#include "awaitable_traits.h"

__CPP_CORO_NS_BEGIN

template<typename ...Awaitables, std::enable_if_t<
   std::conjunction_v<is_awaitable_cancelable<unwrap_reference_t<std::remove_reference_t<Awaitables>>> ...>,int> = 0>
    inline auto when_any_ready(cancel_request* cancel_request, Awaitables ...awaitables)
{
    return when_any_ready_awaitble<std ::tuple<when_any_task<
        typename awaitable_traits<unwrap_reference_t<std::remove_reference_t<Awaitables>>>::await_result_t>...>>
        (cancel_request, std::make_tuple(make_when_any_task(std::forward<Awaitables>(awaitables))...));
}

template<typename Awaitable, 
typename Result = typename awaitable_traits<unwrap_reference_t<Awaitable>>::await_result_t>
  auto when_any_ready(cancel_request* cancel_request, std::vector<Awaitable> awaitables)
{
    std::vector<when_any_task<Result>> tasks;
    tasks.reserve(awaitables.size());

    for (auto &awaitable : awaitables)
    {
        tasks.emplace_back(make_when_any_task(std::move(awaitable)));
    }

    return when_any_ready_awaitble<std::vector<when_any_task<Result>>>(cancel_request, std::move(tasks));
}

__CPP_CORO_NS_END
