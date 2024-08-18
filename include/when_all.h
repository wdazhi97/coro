#pragma once

#include <vector>
#include "when_all_ready_task.h"
#include "fmap.h"

__CPP_CORO_NS_BEGIN

template<
	typename... AwaitableS,
	std::enable_if_t<
		std::conjunction_v<is_awaitable<unwrap_reference_t<std::remove_reference_t<AwaitableS>>>...>,
		int> = 0>
[[nodiscard]] auto when_all(AwaitableS&&... awaitables)
{
	return fmap([](auto&& taskTuple)
	{
		return std::apply([](auto&&... tasks) {
			return std::make_tuple(static_cast<decltype(tasks)>(tasks).non_void_result()...);
		}, static_cast<decltype(taskTuple)>(taskTuple));
	}, when_all_ready(std::forward<AwaitableS>(awaitables)...));
}

//////////
// when_all() with vector of awaitable

template<
	typename Awaitable,
	typename RESULT = typename awaitable_traits<unwrap_reference_t<Awaitable>>::await_result_t,
	std::enable_if_t<std::is_void_v<RESULT>, int> = 0>
[[nodiscard]]
auto when_all(std::vector<Awaitable> awaitables)
{
	return fmap([](auto&& taskVector) {
		for (auto& task : taskVector)
		{
			task.result();
		}
	}, when_all_ready(std::move(awaitables)));
}

template<
	typename Awaitable,
	typename RESULT = typename awaitable_traits<unwrap_reference_t<Awaitable>>::await_result_t,
	std::enable_if_t<!std::is_void_v<RESULT>, int> = 0>
[[nodiscard]]
auto when_all(std::vector<Awaitable> awaitables)
{
	using result_t = std::conditional_t<
		std::is_lvalue_reference_v<RESULT>,
		std::reference_wrapper<std::remove_reference_t<RESULT>>,
		std::remove_reference_t<RESULT>>;

	return fmap([](auto&& taskVector) {
		std::vector<result_t> results;
		results.reserve(taskVector.size());
		for (auto& task : taskVector)
		{
			if constexpr (std::is_rvalue_reference_v<decltype(taskVector)>)
			{
				results.emplace_back(std::move(task).result());
			}
			else
			{
				results.emplace_back(task.result());
			}
		}
		return results;
	}, when_all_ready(std::move(awaitables)));
}

__CPP_CORO_NS_END
