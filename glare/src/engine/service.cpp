#include "service.hpp"
#include "input/raw_input_events.hpp"

#include "meta/indirection.hpp"

#include "meta/meta_type_descriptor.hpp"
#include "meta/meta_evaluation_context.hpp"

#include "components/relationship_component.hpp"
#include "components/name_component.hpp"
#include "components/player_component.hpp"

#include "commands/indirect_component_patch_command.hpp"
#include "commands/component_patch_command.hpp"
#include "commands/component_replace_command.hpp"
#include "commands/function_command.hpp"
#include "commands/expr_command.hpp"
#include "commands/set_parent_command.hpp"

#include "events.hpp"

#include "event_handler.hpp"

#include <app/input/mouse_state.hpp>
#include <app/input/keyboard_state.hpp>
//#include <app/input/gamepad_state.hpp>

#include <algorithm>

#include <util/format.hpp>

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	Service::Service
	(
		Registry& registry,
		SystemManagerInterface& systems,
		const ServicePolicy& policy,

		bool register_input_events,
		bool register_timed_event_wrapper,
		bool register_core_commands,
		bool register_evaluation_commands,
		bool allocate_root_entity,
		bool allocate_universal_variables
	) :
		registry(registry),
		systems(systems),
		active_event_handler(&standard_event_handler),
		policy(policy)
	{
		registry.on_destroy<RelationshipComponent>().connect<&Service::relationship_destroyed_handler>(*this);

		if (register_input_events)
		{
			// Only polled device states for now.
			register_event<app::input::MouseState, &Service::on_mouse_input>(*this);
			register_event<app::input::KeyboardState, &Service::on_keyboard_input>(*this);

			// TODO: Implement exact device events, rather than exclusively polled states.
		}

		if (register_timed_event_wrapper)
		{
			register_event<TimedEvent, &Service::enqueue_timed_event_wrapper>(*this);
		}

		if (register_core_commands)
		{
			register_event<IndirectComponentPatchCommand, &Service::on_indirect_component_patch>(*this);
			register_event<ComponentPatchCommand,         &Service::on_direct_component_patch>(*this);
			register_event<ComponentReplaceCommand,       &Service::on_component_replace>(*this);
			register_event<SetParentCommand,              &Service::on_set_parent>(*this);
		}

		if (register_evaluation_commands)
		{
			register_event<FunctionCommand, &Service::opaque_function_handler>(*this);
			register_event<ExprCommand, &Service::opaque_expression_handler>(*this);
		}

		if (allocate_root_entity)
		{
			root = registry.create();
		}

		if (allocate_universal_variables)
		{
			universal_variables = std::make_shared<UniversalVariables>();
		}
	}

	Service::~Service()
	{
		registry.on_destroy<RelationshipComponent>().disconnect(*this);

		//unregister_event(*this);
	}

	void Service::on_indirect_component_patch(const IndirectComponentPatchCommand& component_patch)
	{
		using namespace engine::literals;

		if (!component_patch.component)
		{
			print_warn("Failed to patch component: Missing component descriptor.");

			return;
		}

		const auto& component = *component_patch.component;

		const auto type = component.get_type();

		auto patch_fn = type.func("indirect_patch_meta_component"_hs);

		if (!patch_fn)
		{
			print_warn("Unable to resolve patch function for type: #{}", type.id());

			return;
		}

		auto& registry = get_registry();

		auto opt_evaluation_context = component_patch.context.get_context();

		auto result = patch_fn.invoke
		(
			{},

			entt::forward_as_meta(registry),
			entt::forward_as_meta(component_patch.target),
			entt::forward_as_meta(component),
			entt::forward_as_meta(component.size()),
			entt::forward_as_meta(0),
			entt::forward_as_meta(component_patch.source),
			entt::forward_as_meta(const_cast<const MetaEvaluationContext*>(&opt_evaluation_context)),
			entt::forward_as_meta(component_patch.event_instance.as_ref())
		);

		if (!result)
		{
			print_warn("Entity #{}: Failed to patch component type: {}", component_patch.target, type.id());
		}
	}

	void Service::on_direct_component_patch(const ComponentPatchCommand& component_patch)
	{
		using namespace engine::literals;

		auto& component = component_patch.component;

		if (!component)
		{
			print_warn("Failed to patch component: Missing component instance.");

			return;
		}

		const auto& entity = component_patch.target;

		const auto type = component.type();

		auto patch_fn = type.func("direct_patch_meta_component"_hs);

		if (!patch_fn)
		{
			print_warn("Unable to resolve patch function for type: #{}", type.id());

			return;
		}

		auto& registry = get_registry();

		auto result = patch_fn.invoke
		(
			{},

			entt::forward_as_meta(registry),
			entt::forward_as_meta(entity),
			entt::forward_as_meta(std::move(component)),
			entt::forward_as_meta(component_patch.use_member_assignment)
		);

		if (!result)
		{
			print_warn("Entity #{}: Failed to patch component type: {}", entity, type.id());
		}
	}

	void Service::on_component_replace(ComponentReplaceCommand& component_replace)
	{
		using namespace engine::literals;

		auto& component = component_replace.component;

		if (!component)
		{
			print_warn("Failed to replace component: Missing component instance.");

			return;
		}

		const auto& entity = component_replace.target;

		const auto type = component.type();

		auto replace_fn = type.func("emplace_meta_component"_hs);

		if (!replace_fn)
		{
			print_warn("Unable to resolve replacement function for type: #{}", type.id());

			return;
		}

		auto& registry = get_registry();

		auto result = replace_fn.invoke
		(
			{},

			entt::forward_as_meta(registry),
			entt::forward_as_meta(entity),
			entt::forward_as_meta(std::move(component))
		);

		if (!result)
		{
			print_warn("Entity #{}: Failed to replace component type: {}", entity, type.id());
		}
	}

	void Service::on_set_parent(const SetParentCommand& parent_command)
	{
		set_parent(parent_command.target, parent_command.parent);
	}

	void Service::opaque_function_handler(const FunctionCommand& function_command)
	{
		on_function_command(function_command);
	}

	void Service::opaque_expression_handler(const ExprCommand& expr_command)
	{
		on_expression_command(expr_command);
	}

	void Service::relationship_destroyed_handler(Registry& registry, Entity entity)
	{
		RelationshipComponent& relationship_comp = registry.get<RelationshipComponent>(entity);

		if (policy.destroy_children_with_parent)
		{
			relationship_comp.enumerate_child_entities
			(
				registry,

				[&registry](Entity child, Entity next_child)
				{
					registry.destroy(child);

					return true;
				}
			);
		}
		else
		{
			const auto parent = relationship_comp.get_parent();

			if (parent == null)
			{
				relationship_comp.enumerate_child_entities
				(
					registry,
			
					[this, &registry](Entity child, Entity next_child)
					{
						remove_parent(child);

						return true;
					}
				);
			}
			else
			{
				relationship_comp.enumerate_child_entities
				(
					registry,
			
					[this, &registry, parent](Entity child, Entity next_child)
					{
						set_parent(child, parent);

						return true;
					}
				);
			}
		}
	}

	void Service::on_function_command(const FunctionCommand& function_command)
	{
		execute_opaque_function
		(
			function_command.function,
			get_registry(),
			function_command.source, // function_command.target
			function_command.context.get_context()
		);
	}

	void Service::on_expression_command(const ExprCommand& expr_command)
	{
		execute_opaque_expression
		(
			expr_command.expr,
			get_registry(),
			expr_command.source, // expr_command.target
			expr_command.context.get_context()
		);
	}

	EventHandler* Service::swap_event_handlers()
	{
		if (active_event_handler == &standard_event_handler)
		{
			return use_forwarding_events();
		}
		else if (active_event_handler == &forwarding_event_handler)
		{
			return use_standard_events();
		}

		return active_event_handler;
	}

	EventHandler* Service::use_standard_events()
	{
		active_event_handler = &standard_event_handler;

		return active_event_handler;
	}

	EventHandler* Service::use_forwarding_events()
	{
		active_event_handler = &forwarding_event_handler;

		return active_event_handler;
	}

	// TODO: Implement thread-safe locking/synchronization event-handler interface.
	// TODO: Determine if some (or all) events should be handled in the fixed update function, rather than the continuous function.
	void Service::update(app::Milliseconds time, float delta)
	{
		// Handle standard events:
		use_forwarding_events();
		standard_event_handler.update();

		// TODO: Implement thread/async-driven timed-event detection and queueing.
		update_timed_events();

		// Handle forwarding events:
		use_standard_events();
		forwarding_event_handler.update();

		// Trigger the standard update event for this service.
		this->event<OnServiceUpdate>(this, time, delta);

		service_event_handler.update();

		handle_deferred_operations();
	}

	void Service::fixed_update(app::Milliseconds time, float delta)
	{
		// Trigger the fixed update event for this service.
		this->event<OnServiceFixedUpdate>(this, time, delta);
	}

	void Service::handle_deferred_operations()
	{
		if (deferred_operations.empty())
		{
			return;
		}

		for (const auto& fn : deferred_operations)
		{
			fn();
		}

		deferred_operations.clear();
	}

	void Service::render(app::Graphics& gfx)
	{
		this->event<OnServiceRender>(this, &gfx);
	}

	Entity Service::get_parent(Entity entity) const
	{
		if (const auto relationship_comp = registry.try_get<RelationshipComponent>(entity))
		{
			return relationship_comp->get_parent();
		}

		return null;
	}

	Entity Service::set_parent(Entity entity, Entity parent)
	{
		auto prev_parent = RelationshipComponent::set_parent(registry, entity, parent);

		// TODO: Look into automating this event.
		event<OnParentChanged>(entity, prev_parent, parent);

		return prev_parent;
	}

	bool Service::remove_child(Entity entity, Entity child)
	{
		if (auto child_relationship_comp = registry.try_get<RelationshipComponent>(child))
		{
			if (child_relationship_comp->get_parent() == entity)
			{
				remove_parent(child);

				return true;
			}
		}

		return false;
	}

	bool Service::remove_parent(Entity entity)
	{
		if (root != null)
		{
			set_parent(entity, root);

			return true;
		}

		if (auto parent = get_parent(entity); parent != null)
		{
			if (RelationshipComponent* parent_relationship_comp = registry.try_get<RelationshipComponent>(parent))
			{
				parent_relationship_comp->remove_child(registry, entity, parent);

				// TODO: Look into automating this event.
				event<OnParentChanged>(entity, parent, null);
			}
		}

		return false;
	}

	std::string Service::label(Entity entity) const
	{
		if (entity == null)
		{
			return "null";
		}

		const auto entity_raw = static_cast<EntityIDType>(entity);

		if (const auto name_comp = registry.try_get<NameComponent>(entity))
		{
			return util::format("{} ({})", name_comp->get_name(), entity_raw);
		}

		return std::to_string(entity_raw);
	}

	std::string_view Service::get_name(Entity entity) const
	{
		if (const auto name_comp = registry.try_get<NameComponent>(entity))
		{
			return { name_comp->get_name() };
		}

		return {};
	}

	bool Service::has_name(Entity entity) const
	{
		return (!get_name(entity).empty());
	}

	bool Service::set_name(Entity entity, std::string_view name)
	{
		if (!name.empty())
		{
			registry.emplace_or_replace<NameComponent>(entity, std::string { name });

			return true;
		}

		return false;
	}

	Entity Service::get_by_name(std::string_view name) const
	{
		auto view = registry.view<NameComponent>();

		for (auto it = view.begin(); it != view.end(); it++)
		{
			Entity entity = *it;

			const auto& name_comp = registry.get<NameComponent>(entity);

			if (name_comp.get_name() == name)
			{
				return entity;
			}
		}

		return null;
	}

	Entity Service::get_child_by_name(Entity entity, std::string_view child_name, bool recursive) const
	{
		if (child_name.empty())
		{
			return null;
		}

		const auto* relationship = registry.try_get<RelationshipComponent>(entity);

		if (!relationship)
		{
			return null;
		}

		Entity out = null;

		relationship->enumerate_child_entities
		(
			registry,

			[&](Entity child, Entity next_child) -> bool
			{
				auto* name_comp = registry.try_get<NameComponent>(child);

				if (name_comp)
				{
					//print("Comparing {} with {}", name_comp->name, child_name);

					if (name_comp->get_name() == child_name)
					{
						out = child;

						return false;
					}
				}
			
				if (recursive)
				{
					if (auto r_out = get_child_by_name(child, child_name, true); r_out != null)
					{
						out = r_out;

						return false;
					}
				}

				return true;
			}
		);

		return out;
	}

	Entity Service::get_player(PlayerIndex player) const
	{
		auto view = registry.view<PlayerComponent>();

		for (auto it = view.begin(); it != view.end(); it++)
		{
			Entity entity = *it;

			const auto& player_state = registry.get<PlayerComponent>(entity);

			if (player_state.player_index == player)
			{
				return entity;
			}
		}

		return null;
	}

	EventHandler& Service::get_active_event_handler()
	{
		assert(active_event_handler);

		return *active_event_handler;
	}

	EventHandler& Service::get_standard_event_handler()
	{
		return standard_event_handler;
	}

	EventHandler& Service::get_forwarding_event_handler()
	{
		return forwarding_event_handler;
	}

	// Retrieves a non-owning pointer to an internal `UniversalVariables` object.
	// If a `UniversalVariables` object does not already exist for this service, this will return `nullptr`.
	Service::UniversalVariables* Service::peek_universal_variables() const
	{
		if (universal_variables)
		{
			return universal_variables.get();
		}

		return nullptr;
	}

	const std::shared_ptr<Service::UniversalVariables>& Service::get_universal_variables()
	{
		if (!universal_variables)
		{
			universal_variables = std::make_shared<UniversalVariables>();
		}

		return universal_variables;
	}

	// Input re-submission callbacks (see class declaration(s) for details):
	void Service::on_mouse_input(const app::input::MouseState& mouse)
	{
		event<OnMouseState>(this, &mouse);
	}

	void Service::on_keyboard_input(const app::input::KeyboardState& keyboard)
	{
		event<OnKeyboardState>(this, &keyboard);
	}

	void Service::enqueue_timed_event_wrapper(const TimedEvent& timed_event)
	{
		enqueue_timed_event(TimedEvent(timed_event));
	}

	void Service::enqueue_timed_event(TimedEvent&& timed_event)
	{
		// If the event's timer isn't already active, start it.
		timed_event.delay.activate();

		pending_timed_events.emplace_back(std::move(timed_event));
	}

	// TODO: Look into whether sorting the `pending_timed_events` collection would improve performance.
	void Service::update_timed_events()
	{
		pending_timed_events.erase
		(
			std::remove_if
			(
				pending_timed_events.begin(),
				pending_timed_events.end(),

				[this](TimedEvent& timed_event)
				{
					using namespace engine::literals;

					if (timed_event.completed())
					{
						trigger_opaque_event(std::move(timed_event.event_instance));

						return true;
					}

					return false;
				}
			),

			pending_timed_events.end()
		);
	}

	bool Service::trigger_opaque_event(MetaAny&& event_instance)
	{
		using namespace engine::literals;

		auto type = event_instance.type();

		if (!type)
		{
			print_warn("Unable to resolve type of generated command.");

			return false;
		}

		auto trigger_fn = type.func("trigger_event_from_meta_any"_hs);

		if (!trigger_fn)
		{
			print_warn("Unable to resolve trigger-function from type: #{}", type.id());

			return false;
		}

		auto result = trigger_fn.invoke
		(
			{},
			entt::forward_as_meta(*this),
			entt::forward_as_meta(std::move(event_instance))
		);

		if (!result)
		{
			print_warn("Failed to trigger event type: #{}", type.id());

			return false;
		}

		return true;
	}
}