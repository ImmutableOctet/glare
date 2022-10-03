#include "input_system.hpp"
#include "input_component.hpp"
#include "buttons.hpp"
#include "events.hpp"

#include <engine/service.hpp>

#include <app/input/events.hpp>
#include <app/input/input_handler.hpp>
#include <app/input/gamepad_buttons.hpp>
#include <app/input/gamepad_profile.hpp>

#include <util/map.hpp>

// Debugging related:
#include <util/log.hpp>
#include <format>
#include <magic_enum/magic_enum_format.hpp>

namespace engine
{
	InputSystem::InputSystem(Service& service, InputHandler& input_handler, bool subscribe_immediately)
		: BasicSystem(service), input_handler(input_handler)
	{
		states.reserve(4);
		gamepad_assignment.reserve(4);

		if (subscribe_immediately)
		{
			subscribe();
		}
	}

	bool InputSystem::on_subscribe(Service& service)
	{
		auto& registry = service.get_registry();

		// Core registry events:
		registry.on_construct<InputComponent>().connect<&InputSystem::on_create_input>(*this);
		registry.on_destroy<InputComponent>().connect<&InputSystem::on_destroy_input>(*this);
		registry.on_update<InputComponent>().connect<&InputSystem::on_change_input>(*this);

		// Meta:
		service.register_event<OnServiceUpdate, &InputSystem::on_update>(*this);

		// Gamepad:
		service.register_event<app::input::OnGamepadConnected,    &InputSystem::on_gamepad_connected>(*this);
		service.register_event<app::input::OnGamepadDisconnected, &InputSystem::on_gamepad_disconnected>(*this);
		service.register_event<app::input::OnGamepadButtonDown,   &InputSystem::on_gamepad_button_down>(*this);
		service.register_event<app::input::OnGamepadButtonUp,     &InputSystem::on_gamepad_button_up>(*this);
		service.register_event<app::input::OnGamepadAnalogInput,  &InputSystem::on_gamepad_analog_input>(*this);

		return true;
	}

	void InputSystem::on_create_input(Registry& registry, Entity entity)
	{
		const auto& input_comp = registry.get<InputComponent>(entity);
		const auto& input_index = input_comp.input_index;

		auto& state = allocate_input_data(input_index);

		// Increment the number of active listeners.
		if ((state.active_listeners++) == 0)
		{
			on_start_listening(input_index, state);
		}
	}

	void InputSystem::on_destroy_input(Registry& registry, Entity entity)
	{
		const auto& input_comp  = registry.get<InputComponent>(entity);
		const auto& input_index = input_comp.input_index;

		auto& state = states[input_index];

		assert(state.active_listeners > 0);

		if ((--state.active_listeners) == 0)
		{
			on_stop_listening(input_index, state);
		}
	}

	void InputSystem::on_change_input(Registry& registry, Entity entity)
	{
		// Updating an `InputComponent` is considered a forbidden operation.
		// You will need to destroy the component, then re-attach a new instance.
		assert(false);
	}

	void InputSystem::handle_gamepad_mappings(const Gamepad& gamepad, std::optional<StateIndex> opt_state_index)
	{
		const auto& device_mapping = input_handler.get_player_device_map();

		// TODO: Look into optimizing this via heterogeneous lookup.
		if (auto it = device_mapping.find(gamepad.get_device_name()); it != device_mapping.end()) // get_device_name_as_view()
		{
			auto state_index = static_cast<StateIndex>(it->second);

			if (opt_state_index)
			{
				if (state_index != (*opt_state_index))
				{
					return;
				}
			}

			// Ensure we have data allocated for the index retrieved.
			allocate_input_data(state_index);

			// Bind `gamepad_index` to the state-index.
			bind_gamepad(gamepad.index(), state_index);
		}
	}

	void InputSystem::handle_gamepad_mappings(GamepadIndex gamepad_index, std::optional<StateIndex> opt_state_index)
	{
		const auto& gamepads = input_handler.get_gamepads();
		const auto& gamepad  = gamepads.get_gamepad(gamepad_index);
		
		handle_gamepad_mappings(gamepad, opt_state_index);
	}

	void InputSystem::on_start_listening(StateIndex state_index, StateData& state)
	{
		const auto& gamepads = input_handler.get_gamepads();

		// Handle gamepad device mappings:
		gamepads.enumerate_gamepads([this, state_index](const Gamepad& gamepad) -> bool
		{
			// TODO: Determine if we really need to limit to `state_index`.
			handle_gamepad_mappings(gamepad, state_index);

			// Continue enumeration regardless of outcome.
			// NOTE: Multiple gamepad devices can map to the player, etc.
			return true;
		});
	}

	void InputSystem::on_stop_listening(StateIndex state_index, StateData& state)
	{
		// Handle gamepad device mappings:
		
		// NOTE: This does not call `unbind_gamepad`, opting instead to erase from the map directly.
		util::erase_by_value(gamepad_assignment, state_index, false);

		// Clear the `state` data specified.
		clear_input_data(state);
	}

	InputSystem::StateData& InputSystem::allocate_input_data(StateIndex index)
	{
		// Resize if `state_index` is beyond the scope of `next_state`.
		if (index >= states.size())
		{
			states.resize((index + 1));
		}

		return states[index];
	}

	void InputSystem::clear_input_data(StateIndex index)
	{
		assert(index < states.size());

		clear_input_data(states[index]);
	}

	void InputSystem::clear_input_data(StateData& state)
	{
		state = {};
	}

	std::optional<Button> InputSystem::translate_button(const Gamepad& gamepad, const GamepadButtonEvent& button_event) const
	{
		auto& gamepads = input_handler.get_gamepads();
		const auto* profile = gamepads.get_profile(button_event.device_index);

		if (!profile)
		{
			//print("GAMEPAD PROFILE MISSING.");

			return std::nullopt;
		}

		auto gamepad_button_bit = static_cast<app::input::GamepadButtonBits>(button_event.button);

		auto& button_mapping = profile->button_mapping;
		auto it = button_mapping.find(gamepad_button_bit);

		if (it != button_mapping.end())
		{
			return static_cast<Button>(it->second);
		}

		return std::nullopt;
	}

	std::optional<Analog> InputSystem::translate_analog(const Gamepad& gamepad, const GamepadAnalogEvent& analog_event) const
	{
		auto& gamepads = input_handler.get_gamepads();
		const auto* profile = gamepads.get_profile(analog_event.device_index);

		if (!profile)
		{
			//print("GAMEPAD PROFILE MISSING.");

			return std::nullopt;
		}

		const auto& gamepad_analog = analog_event.analog;

		auto& analog_mapping = profile->analog_mapping;
		auto it = analog_mapping.find(gamepad_analog);

		if (it != analog_mapping.end())
		{
			return static_cast<Analog>(it->second);
		}

		return std::nullopt;
	}

	void InputSystem::on_gamepad_connected(const app::input::OnGamepadConnected& data)
	{
		// Debugging related:
		print("Device connected: {}", data.device_index);

		handle_gamepad_mappings(data.device_index);
	}
	
	void InputSystem::on_gamepad_disconnected(const app::input::OnGamepadDisconnected& data)
	{
		// Debugging related:
		print("Device disconnected: {}", data.device_index);

		unbind_gamepad(data.device_index);
	}

	void InputSystem::on_update(const OnServiceUpdate& data)
	{
		if (allowed_service(*data.service))
		{
			return;
		}

		// State index counter.
		StateIndex index = 0;

		// Enumerate all input states, checking for changes:
		for (auto& state_data : states)
		{
			if (state_data.state_has_changed)
			{
				// Trigger the 'state changed' event; `OnInput`.
				service->event<OnInput>(std::monostate{}, index, state_data.next, state_data.previous);

				// Copy the current state into the previous state.
				state_data.previous = state_data.next;

				// Force-update the 'pressed' and 'released' statuses to no longer be active:
				state_data.next.pressed.clear();
				state_data.next.released.clear();

				// Reset the 'state changed' flag.
				state_data.state_has_changed = false;
			}

			// Must increment; `continue` not allowed.
			index++;
		}
	}

	void InputSystem::on_state_update(StateData& state_data)
	{
		state_data.state_has_changed = true;
	}

	void InputSystem::on_button_down(InputSource source, StateIndex state_index, Button button)
	{
		auto& state_data = states[state_index];
		auto& state = state_data.next;

		// Must be called; general update routine.
		on_state_update(state_data);

		// This button is being held down or pressed, so it can't be considered 'released'.
		state.released.set_button(button, false);

		// Check whether this button has already been marked as held:
		if (state.held.get_button(button))
		{
			// Since we're already set as 'holding this button', the
			// button has already been marked as 'pressed' as well, so
			// we can therefore set this as no longer being pressed.
			state.pressed.set_button(button, false);
		}
		else
		{
			// This button has been pressed, update the next state to reflect that.
			state.pressed.set_button(button, true);

			// This button is being held down, update the next state to reflect that.
			state.held.set_button(button, true);

			// Trigger the 'button pressed' event.
			service->event<OnButtonPressed>(source, state_index, state, button);
		}

		// Trigger the continuous 'button held/down' event.
		service->event<OnButtonDown>(source, state_index, state, button);
	}

	void InputSystem::on_button_up(InputSource source, StateIndex state_index, Button button)
	{
		auto& state_data = states[state_index];
		auto& state = state_data.next;

		// Must be called; general update routine.
		on_state_update(state_data);

		// This button is no longer being held down.
		state.held.set_button(button, false);

		// This button is no longer being pressed.
		state.pressed.set_button(button, false);

		// Set the 'released' status. (Valid until next frame/update)
		state.released.set_button(button, true);

		// Trigger the 'button released' event.
		service->event<OnButtonReleased>(source, state_index, state, button);
	}

	void InputSystem::on_analog_input(InputSource source, StateIndex state_index, Analog analog, const math::Vector2D& value, std::optional<float> angle)
	{
		auto& state_data = states[state_index];
		auto& state = state_data.next;

		// Must be called; general update routine.
		on_state_update(state_data);

		state.directional_input.set_analog(analog, value);

		//auto angle_out = state.directional_input.angle_of(analog);
		auto angle_out = (angle.has_value() ? (*angle) : state.directional_input.angle_of(value));

		// Trigger the 'analog input' event.
		service->event<OnAnalogInput>(source, state_index, state, analog, value, angle_out);
	}

	void InputSystem::on_gamepad_button_down(const app::input::OnGamepadButtonDown& data)
	{
		const auto& gamepads   = input_handler.get_gamepads();
		const auto& gamepad_id = data.device_index;
		auto& gamepad          = gamepads.get_gamepad(gamepad_id);

		if (auto button = translate_button(gamepad, data))
		{
			if (auto state_index = get_gamepad_state_index(gamepad_id))
			{
				on_button_down(gamepad, *state_index, *button);
			}
		}
	}
	
	void InputSystem::on_gamepad_button_up(const app::input::OnGamepadButtonUp& data)
	{
		const auto& gamepads   = input_handler.get_gamepads();
		const auto& gamepad_id = data.device_index;
		auto& gamepad          = gamepads.get_gamepad(gamepad_id);

		if (auto button = translate_button(gamepad, data))
		{
			if (auto state_index = get_gamepad_state_index(gamepad_id))
			{
				on_button_up(gamepad, *state_index, *button);
			}
		}
	}
	
	void InputSystem::on_gamepad_analog_input(const app::input::OnGamepadAnalogInput& data)
	{
		const auto& gamepads = input_handler.get_gamepads();
		const auto& gamepad_id = data.device_index;
		auto& gamepad = gamepads.get_gamepad(gamepad_id);

		if (auto analog = translate_analog(gamepad, data))
		{
			if (auto state_index = get_gamepad_state_index(gamepad_id))
			{
				on_analog_input(gamepad, *state_index, *analog, data.value, data.angle());
			}
		}
	}

	void InputSystem::bind_gamepad(GamepadIndex gamepad_index, StateIndex state_index)
	{
		gamepad_assignment[gamepad_index] = state_index;
	}

	void InputSystem::unbind_gamepad(GamepadIndex gamepad_index)
	{
		gamepad_assignment.erase(gamepad_index);
	}

	InputState InputSystem::get_input_state(StateIndex index) const
	{
		const auto& state_data = peek_state_data(index);

		return state_data.next;
	}

	const InputSystem::StateData& InputSystem::peek_state_data(StateIndex index) const
	{
		assert(index < states.size());

		return states[index];
	}

	std::optional<InputSystem::StateIndex> InputSystem::get_gamepad_state_index(GamepadIndex gamepad_id) const
	{
		auto it = gamepad_assignment.find(gamepad_id);

		if (it != gamepad_assignment.end())
		{
			return it->second;
		}

		return std::nullopt;
	}
}