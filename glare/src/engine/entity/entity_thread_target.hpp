#pragma once

#include "types.hpp"
#include "entity_thread_range.hpp"

#include <variant>
#include <utility>
#include <string>
#include <string_view>

namespace engine
{
	struct EntityThreadTarget
	{
		using Empty = std::monostate;

		using Type = std::variant
		<
			Empty,
			EntityThreadRange,
			EntityThreadID
		>;

		EntityThreadTarget(Type value=Empty{});

		EntityThreadTarget(EntityThreadRange thread_range);
		EntityThreadTarget(EntityThreadID thread_id);
		EntityThreadTarget(std::string_view thread_name);
		EntityThreadTarget(const std::string& thread_name);

		EntityThreadTarget(const EntityThreadTarget&) = default;
		EntityThreadTarget(EntityThreadTarget&&) noexcept = default;

		EntityThreadTarget& operator=(const EntityThreadTarget&) = default;
		EntityThreadTarget& operator=(EntityThreadTarget&&) noexcept = default;

		EntityThreadTarget& operator=(EntityThreadRange thread_range);
		EntityThreadTarget& operator=(EntityThreadID thread_id);
		EntityThreadTarget& operator=(std::string_view thread_name);
		EntityThreadTarget& operator=(const std::string& thread_name);

		inline operator const Type& () const
		{
			return value;
		}

		bool empty() const;

		bool operator==(const EntityThreadTarget&) const noexcept = default;
		bool operator!=(const EntityThreadTarget&) const noexcept = default;

		Type value;
	};
}