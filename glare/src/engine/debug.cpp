#include "debug.hpp"

#include <format>

#include <game/game.hpp>

#include "events.hpp"
#include "relationship.hpp"

#include "world/world.hpp"
#include "world/world_events.hpp"

// Components:
#include <engine/world/animation/skeletal_component.hpp>

namespace engine
{
	//struct SkeletalComponent;

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
			auto total_children = rel->total_children(registry);

			print("Number of children listed: {} ({} recursively)", child_count, total_children);
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

	void position_in_titlebar(game::Game& game, Entity entity, std::optional<std::string_view> prefix)
	{
		auto& world = game.get_world();

		if (entity != engine::null)
		{
			auto t = world.get_transform(entity);
			auto position = t.get_position();

			game.set_title(std::format("{}{}: {},{},{}", prefix.value_or(""), world.get_name(entity), position.x, position.y, position.z));
		}
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
		auto& registry = world.get_registry();

		// Standard event types:
		enable<OnStageLoaded>();
		enable<OnEntityCreated>();
		enable<OnParentChanged>();

		// Component construction events:
		registry.on_construct<SkeletalComponent>().connect<&DebugListener::on_skeleton>(*this);
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

	void DebugListener::operator()(const OnGravityChanged& data)
	{
		print("Gravity changed from: {}, to: {}", data.old_gravity, data.new_gravity);
	}

	void DebugListener::on_skeleton(Registry& registry, Entity entity)
	{
		print("Skeleton attached to: {}", entity);
	}
}