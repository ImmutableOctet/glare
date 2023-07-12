#pragma once

#include "actions/action.hpp"

//#include <engine/timer.hpp>
#include <app/types.hpp>

//#include <util/small_vector.hpp>
#include <vector>

namespace engine
{
	struct HistoryEntry
	{
		//using ContainerType = util::small_vector<HistoryAction, 32>;
		using ContainerType = std::vector<HistoryAction>;

		using TimestampType = app::Milliseconds; // Timer::TimePoint;

		inline std::size_t size() const
		{
			return actions.size();
		}

		inline bool empty() const
		{
			return actions.empty();
		}

		inline auto begin()
		{
			return actions.begin();
		}

		inline auto begin() const
		{
			return actions.begin();
		}

		inline auto end()
		{
			return actions.end();
		}

		inline auto end() const
		{
			return actions.end();
		}

		inline auto cbegin() const
		{
			return actions.cbegin();
		}

		inline auto cend() const
		{
			return actions.cend();
		}

		inline explicit operator bool() const
		{
			return (!empty());
		}

		TimestampType timestamp = {};

		ContainerType actions;
	};
}