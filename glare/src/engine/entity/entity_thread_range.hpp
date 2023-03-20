#pragma once

#include "types.hpp"

namespace engine
{
	struct EntityThreadRange
	{
		EntityThreadIndex start_index;
		EntityThreadCount thread_count;

		inline EntityThreadIndex begin_point() const
		{
			return start_index;
		}

		inline EntityThreadIndex end_point() const
		{
			return (start_index + thread_count);
		}

		inline std::size_t size() const
		{
			return static_cast<std::size_t>(thread_count);
		}

		inline bool empty() const
		{
			return (size() == 0);
		}

		bool operator==(const EntityThreadRange&) const noexcept = default;
		bool operator!=(const EntityThreadRange&) const noexcept = default;
	};
}