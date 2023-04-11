#include "entity_thread_target.hpp"

#include <engine/meta/hash.hpp>

#include <util/variant.hpp>

namespace engine
{
	EntityThreadTarget::EntityThreadTarget(Type value)
		: value(std::move(value)) {}

	EntityThreadTarget::EntityThreadTarget(EntityThreadRange thread_range)
		: value(thread_range) {}

	EntityThreadTarget::EntityThreadTarget(EntityThreadID thread_id)
		: value(thread_id) {}

	EntityThreadTarget::EntityThreadTarget(std::string_view thread_name)
		: value(hash(thread_name).value())
	{
		assert(!thread_name.empty());
		//assert(hash(thread_name).value());
	}

	EntityThreadTarget::EntityThreadTarget(const std::string& thread_name)
		: EntityThreadTarget(std::string_view(thread_name)) {}

	EntityThreadTarget& EntityThreadTarget::operator=(EntityThreadRange thread_range)
	{
		value = thread_range;

		return *this;
	}

	EntityThreadTarget& EntityThreadTarget::operator=(EntityThreadID thread_id)
	{
		value = thread_id;

		return *this;
	}

	EntityThreadTarget& EntityThreadTarget::operator=(std::string_view thread_name)
	{
		value = hash(thread_name).value();

		return *this;
	}

	EntityThreadTarget& EntityThreadTarget::operator=(const std::string& thread_name)
	{
		value = hash(thread_name).value();

		return *this;
	}

	bool EntityThreadTarget::empty() const
	{
		bool result = false;

		util::visit
		(
			value,

			[&result](const Empty&)
			{
				result = true;
			},

			[&result](const EntityThreadRange& thread_range)
			{
				result = thread_range.empty();
			},

			[&result](EntityThreadID thread_id)
			{
				result = !static_cast<bool>(thread_id);
			}
		);

		return result;
	}
}