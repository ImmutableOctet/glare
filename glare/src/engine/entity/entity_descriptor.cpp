#include "entity_descriptor.hpp"
#include "entity_factory_context.hpp"

#include <engine/meta/hash.hpp>

namespace engine
{
	EntityDescriptor::EntityDescriptor(std::string_view path)
		: shared_storage
		(
			(path.empty())
			? MetaSymbolID {}
			: static_cast<MetaSymbolID>(hash(path).value())
		)
	{}

	EntityDescriptor::EntityDescriptor(const EntityFactoryContext& factory_context)
		: EntityDescriptor(factory_context.paths.instance_path.string())
	{}

	MetaTypeDescriptor& EntityDescriptor::generate_empty_command(const MetaType& command_type, Entity source, Entity target)
	{
		using namespace engine::literals;

		//auto command_content = CommandContent(command_type);
		auto& command_content = shared_storage.allocate<MetaTypeDescriptor>(command_type);

		command_content.set_variable(MetaVariable("source"_hs, Entity(null)));
		command_content.set_variable(MetaVariable("target"_hs, Entity(null)));

		return command_content;
	}

	bool EntityDescriptor::set_state(Registry& registry, Entity entity, std::optional<EntityStateIndex> previous_state, std::string_view state_name) const
	{
		return set_state_by_id(registry, entity, previous_state, hash(state_name));
	}

	bool EntityDescriptor::set_state_by_id(Registry& registry, Entity entity, std::optional<EntityStateIndex> previous_state, EntityStateID state_id) const
	{
		auto state_index = get_state_index(state_id);

		if (!state_index)
		{
			return false;
		}

		return set_state_by_index(registry, entity, previous_state, *state_index);
	}

	bool EntityDescriptor::set_state_by_index(Registry& registry, Entity entity, std::optional<EntityStateIndex> previous_state, EntityStateIndex state_index) const
	{
		if (state_index >= states.size())
		{
			return false;
		}

		const EntityState* from = nullptr;

		if (previous_state)
		{
			if ((*previous_state) >= states.size())
			{
				return false;
			}

			from = &(states[*previous_state].get(*this));
		}

		const auto& to = states[state_index].get(*this);

		to.update(*this, registry, entity, state_index, from, previous_state);

		return true;
	}

	const EntityThreadDescription& EntityDescriptor::get_thread(EntityThreadIndex thread_index) const
	{
		const auto& threads = get_threads();

		assert(thread_index < threads.size());

		return threads[thread_index]; // .at(thread_index);
	}

	const EntityThreadDescription* EntityDescriptor::get_thread_by_id(EntityThreadID thread_id) const
	{
		if (const auto thread_index = get_thread_index(thread_id))
		{
			return &(get_thread(*thread_index));
		}

		return {};
	}

	// TODO: Optimize via map lookup, etc.
	std::optional<EntityThreadIndex> EntityDescriptor::get_thread_index(EntityThreadID thread_id) const
	{
		const auto& threads = get_threads();

		//for (const auto& thread : threads)
		for (std::size_t i = 0; i < threads.size(); i++)
		{
			const auto& thread = threads[i];

			if (thread.thread_id == thread_id)
			{
				return static_cast<EntityThreadIndex>(i);
			}
		}

		return std::nullopt;
	}

	std::optional<EntityThreadID> EntityDescriptor::get_thread_id(EntityThreadIndex thread_index) const
	{
		const auto& thread = get_thread(thread_index);

		return thread.thread_id;
	}
}