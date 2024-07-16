#pragma once

#include "types.hpp"

namespace engine
{
	struct EntityThreadRange
	{
		EntityThreadIndex start_index = ENTITY_THREAD_INDEX_INVALID;
		EntityThreadCount thread_count = {};

		inline EntityThreadIndex begin_point() const
		{
			return start_index;
		}

		inline EntityThreadIndex end_point() const
		{
			if (is_invalid())
			{
				return (start_index);
			}

			return (start_index + thread_count);
		}

		inline bool is_invalid() const
		{
			return (start_index == ENTITY_THREAD_INDEX_INVALID);
		}

		inline std::size_t size() const
		{
			if (is_invalid())
			{
				return 0;
			}

			return static_cast<std::size_t>(thread_count);
		}

		inline bool empty() const
		{
			return (size() == 0);
		}

		inline explicit operator bool() const
		{
			return (!empty());
		}

		bool operator==(const EntityThreadRange&) const noexcept = default;
		bool operator!=(const EntityThreadRange&) const noexcept = default;
	};
}