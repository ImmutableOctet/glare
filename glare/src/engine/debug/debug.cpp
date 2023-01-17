#include "debug.hpp"

#include <engine/events.hpp>

#include <engine/commands/events.hpp>
#include <engine/commands/print_command.hpp>

#include <engine/components/relationship_component.hpp>

#include <engine/entity/events.hpp>

#include <engine/world/world.hpp>
#include <engine/world/world_events.hpp>
#include <engine/world/physics/collision_events.hpp>

#include <engine/world/animation/components/skeletal_component.hpp>

#include <engine/input/events.hpp>

#include <app/input/events.hpp>

#include <game/game.hpp>

#include <util/format.hpp>
#include <util/log.hpp>

//#include <magic_enum_format.hpp>

// Debugging related:
#include <math/math.hpp>

namespace engine
{
	//struct SkeletalComponent;

	void print_children(World& world, Entity entity, bool recursive, bool summary_info, bool recursive_labels, const std::string& prefix)
	{
		auto& registry = world.get_registry();
		auto* rel = registry.try_get<engine::RelationshipComponent>(entity);

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

			game.set_title(util::format("{}{}: {},{},{}", prefix.value_or(""), world.get_name(entity), position.x, position.y, position.z));
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
		: WorldSystem(world) {}

	void DebugListener::on_subscribe(World& world)
	{
		auto& registry = world.get_registry();

		// Component construction events:
		registry.on_construct<SkeletalComponent>().connect<&DebugListener::on_skeleton>(*this);

		// Standard event types:
		enable<OnStageLoaded>();
		enable<OnEntityCreated>();
		enable<OnParentChanged>();

		//enable<OnCommandExecution>();

		// Disabled for now.
		//enable<OnAABBOverlap>();
		
		////enable<OnCollision>();

		//enable<OnTransformChanged>();
		enable<OnKinematicInfluence>();
		enable<OnKinematicAdjustment>();

		//enable<app::input::OnGamepadConnected>();
		//enable<app::input::OnGamepadDisconnected>();
		//enable<app::input::OnGamepadButtonDown>();
		//enable<app::input::OnGamepadButtonUp>();
		//enable<app::input::OnGamepadAnalogInput>();

		//enable<OnButtonDown>();
		enable<OnButtonReleased>();
		enable<OnButtonPressed>();
		//enable<OnAnalogInput>();

		enable<PrintCommand>();

		enable<OnThreadSpawn>();
		enable<OnThreadComplete>();
		enable<OnThreadTerminated>();
		enable<OnThreadPaused>();
		enable<OnThreadResumed>();
	}

	void DebugListener::on_skeleton(Registry& registry, Entity entity)
	{
		print("Skeleton attached to: {}", entity);
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

	void DebugListener::operator()(const OnCommandExecution& data)
	{
		const auto& command = data.command;
		const auto& command_id = data.command_id;

		if (command.source == command.target)
		{
			print("Command #{} executed by: #{}", command_id.value_or(0), command.target);
		}
		else
		{
			print("Command #{} executed on #{} by #{}", command_id.value_or(0), command.target, command.source);
		}
	}

	void DebugListener::operator()(const OnGravityChanged& data)
	{
		print("Gravity changed from: {}, to: {}", data.old_gravity, data.new_gravity);
	}

	void DebugListener::operator()(const OnTransformChanged& data)
	{
		auto tform = world.get_transform(data.entity);

		auto [position, rotation, scale] = tform.get_vectors();

		print("Transform of {} changed. (Position: [{}], Rotation: [{}], Scale: [{}])", data.entity, position, rotation, scale);
	}

	void DebugListener::operator()(const OnAABBOverlap& data)
	{
		print("AABB Overlap detected: {} in {} [Contacts: {}]", data.entity, data.bounding, data.number_of_contacts);
	}

	void DebugListener::operator()(const OnCollision& data)
	{
		switch (data.contact_type)
		{
			case ContactType::Interaction:
				print("Interaction detected.");

				break;

			//case ContactType::Surface:
			case ContactType::Intersection:
				print("Collision detected between entities {} and {} at {}. (Contact type: {})", data.a, data.b, data.position, static_cast<int>(data.contact_type));

				break;
		}
	}

	void DebugListener::operator()(const OnKinematicInfluence& data)
	{
		print("Kinematic influence: {} influenced by {}", data.target.entity, data.influencer);
	}

	void DebugListener::operator()(const OnKinematicAdjustment& data)
	{
		print("Kinematic adjustment: {} adjusted by {}", data.entity, data.adjusted_by);
	}

	void DebugListener::operator()(const app::input::OnGamepadConnected& data)
	{
		print("Controller #{} connected.", data.device_index);
	}

	void DebugListener::operator()(const app::input::OnGamepadDisconnected& data)
	{
		print("Controller #{} disconnected.", data.device_index);
	}

	void DebugListener::operator()(const app::input::OnGamepadButtonDown& data)
	{
		print("Controller #{} - Button Down: {}", data.device_index, data.button);
	}

	void DebugListener::operator()(const app::input::OnGamepadButtonUp& data)
	{
		print("Controller #{} - Button Up: {}", data.device_index, data.button);
	}

	void DebugListener::operator()(const app::input::OnGamepadAnalogInput& data)
	{
		print("Controller #{} - Analog Input [{}]: {} ({})", data.device_index, static_cast<unsigned int>(data.analog), data.value, math::degrees(data.angle()));
	}

	void DebugListener::operator()(const OnButtonDown& data)
	{
		const auto& player_id = data.state_index;
		const auto& button    = data.button;

		print("Player: {} - Button Held: {}", player_id, util::format("{}", button));
	}

	void DebugListener::operator()(const OnButtonReleased& data)
	{
		const auto& player_id = data.state_index;
		const auto& button = data.button;

		print("Player: {} - Button Released: {}", player_id, util::format("{}", button));
	}
	
	void DebugListener::operator()(const OnButtonPressed& data)
	{
		const auto& player_id = data.state_index;
		const auto& button = data.button;

		print("Player: {} - Button Pressed: {}", player_id, util::format("{}", button));
	}

	void DebugListener::operator()(const OnAnalogInput& data)
	{
		print("Analog input [Player {}] - {}: {} ({})", data.state_index, util::format("{}", data.analog), data.value, math::degrees(data.angle));
	}

	void DebugListener::operator()(const PrintCommand& data)
	{
		print(data.message);
	}

	void DebugListener::operator()(const OnThreadSpawn& thread_details)
	{
		if (thread_details.thread_id)
		{
			print("Entity #{}: Thread #{} Spawned (ID: #{})", thread_details.entity, thread_details.thread_index, *thread_details.thread_id);
		}
		else
		{
			print("Entity #{}: Thread #{} Spawned", thread_details.entity, thread_details.thread_index);
		}
	}

	void DebugListener::operator()(const OnThreadComplete& thread_details)
	{
		if (thread_details.thread_id)
		{
			print("Entity #{}: Thread #{} Execution completed (ID: #{})", thread_details.entity, thread_details.thread_index, *thread_details.thread_id);
		}
		else
		{
			print("Entity #{}: Thread #{} Execution completed", thread_details.entity, thread_details.thread_index);
		}
	}

	void DebugListener::operator()(const OnThreadTerminated& thread_details)
	{
		if (thread_details.thread_id)
		{
			print("Entity #{}: Thread #{} Execution terminated (ID: #{})", thread_details.entity, thread_details.thread_index, *thread_details.thread_id);
		}
		else
		{
			print("Entity #{}: Thread #{} Execution terminated", thread_details.entity, thread_details.thread_index);
		}
	}

	void DebugListener::operator()(const OnThreadPaused& thread_details)
	{
		if (thread_details.thread_id)
		{
			print("Entity #{}: Thread #{} Execution paused (ID: #{})", thread_details.entity, thread_details.thread_index, *thread_details.thread_id);
		}
		else
		{
			print("Entity #{}: Thread #{} Execution paused", thread_details.entity, thread_details.thread_index);
		}
	}

	void DebugListener::operator()(const OnThreadResumed& thread_details)
	{
		if (thread_details.thread_id)
		{
			print("Entity #{}: Thread #{} Execution resumed (ID: #{})", thread_details.entity, thread_details.thread_index, *thread_details.thread_id);
		}
		else
		{
			print("Entity #{}: Thread #{} Execution resumed", thread_details.entity, thread_details.thread_index);
		}
	}
}