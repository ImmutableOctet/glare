#include "debug.hpp"

#include "relationship.hpp"
#include "world/world.hpp"

#include "events/events.hpp"

// Testing related:
#include "world/spin_component.hpp"

namespace engine
{
	void print_children(World& world, Entity entity, bool recursive, bool summary_info, bool recursive_labels, const std::string& prefix)
	{
		auto& registry = world.get_registry();
		auto* rel = registry.try_get<engine::Relationship>(entity);

		if (!rel)
		{
			if (summary_info)
			{
				print("{} No relationship detected.", prefix);
			}

			return;
		}

		auto child_count = rel->children();

		if (child_count == 0)
		{
			if (summary_info)
			{
				print("{} No children detected.", prefix);
				//print("No children detected for {}", world.label(entity));
			}

			return;
		}

		if (summary_info)
		{
			auto total_nodes = rel->count_nodes(registry);

			print("Number of children listed: {} ({} recursively)", child_count, total_nodes);
			//print("{} List of children:", prefix);
			print("List:");
		}

		rel->enumerate_child_entities(registry, [recursive, recursive_labels, &prefix, &world](engine::Entity child, engine::Entity next_child)
		{
			auto child_label = world.label(child);

			print("{} {}", prefix, child_label);

			if (recursive)
			{
				if (recursive_labels)
				{
					print_children(world, child, true, false, true, (prefix + "->" + child_label + "->"));
				}
				else
				{
					print_children(world, child, true, false, false, ("-" + prefix));
				}
			}

			return true;
		});
	}

	template <typename EventType>
	void DebugListener::enable()
	{
		world.register_event
		<
			EventType,
			
			static_cast<void (DebugListener::*)(const EventType&)>
			(&DebugListener::operator())
		>
		(*this);
	}

	DebugListener::DebugListener(World& world)
		: world(world)
	{
		enable<OnStageLoaded>();
		enable<OnEntityCreated>();
		enable<OnParentChanged>();
	}

	DebugListener::~DebugListener()
	{
		world.unsubscribe(*this);
	}

	Registry& DebugListener::get_registry() const
	{
		return get_world().get_registry();
	}

	std::string DebugListener::label(Entity entity)
	{
		return world.label(entity);
	}

	void DebugListener::operator()(const OnStageLoaded& stage_info)
	{
		assert(stage_info.stage != null);

		if (stage_info.path)
		{
			print("Stage \"{}\" loaded successfully.", stage_info.path->string());
			//util::log::console->info("Stage \"{}\" loaded successfully.", stage_info.path->string());
		}

		auto stage = stage_info.stage;

		print_children(world, stage);

		auto model_group = world.get_child_by_name
		(
			stage,
			//"Torus"
			"Geosphere"
		);

		if (model_group != null)
		{
			auto& registry = get_registry();

			//registry.emplace<SpinBehavior>(model_group);
		}

		//world. SpinBehavior
	}

	void DebugListener::operator()(const OnEntityCreated& entity_info)
	{
		print
		(
			"Entity created: {} (type: {}) (parent: {})",
			label(entity_info.entity),
			static_cast<std::uint8_t>(entity_info.get_type()),
			label(entity_info.parent)
		);
	}

	void DebugListener::operator()(const OnParentChanged& data)
	{
		print
		(
			"Parent of {} changed from {} to {}.",
			
			label(data.entity),
			
			label(data.from_parent),
			label(data.to_parent)
		);
	}
}