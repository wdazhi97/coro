#pragma once

#include "when_all_task.h"
#include "unwrap_reference.h"
#include "is_awaitable.h"
#include "when_all_ready_awaitable.hpp"
#include "awaitable_traits.h"

__CPP_CORO_NS_BEGIN

template<typename ...Awaitables, std::enable_if_t<
   std::conjunction_v<is_awaitable<unwrap_reference_t<std::remove_reference_t<Awaitables>>> ...>,int> = 0>
    inline auto when_all_ready(Awaitables ...awaitables)
{
    return when_all_ready_awaitble<std ::tuple<when_all_task<
        typename awaitable_traits<unwrap_reference_t<std::remove_reference_t<Awaitables>>>::await_result_t>...>>
        (std::make_tuple(make_when_all_task(std::forward<Awaitables>(awaitables))...));
}

template<typename Awaitable, 
typename Result = typename awaitable_traits<unwrap_reference_t<Awaitable>>::await_result_t>
  auto when_all_ready(std::vector<Awaitable> awaitables)
{
    std::vector<when_all_task<Result>> tasks;
    tasks.reserve(awaitables.size());

    for (auto &awaitable : awaitables)
    {
        tasks.emplace_back(make_when_all_task(std::move(awaitable)));
    }

    return when_all_ready_awaitble<std::vector<when_all_task<Result>>>(std::move(tasks));
}

__CPP_CORO_NS_END
