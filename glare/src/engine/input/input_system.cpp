#include "input_system.hpp"
#include "input_component.hpp"
#include "buttons.hpp"
#include "events.hpp"

#include <engine/service.hpp>

#include <app/input/events.hpp>
#include <app/input/mouse.hpp>
#include <app/input/input_handler.hpp>
#include <app/input/gamepad_buttons.hpp>
#include <app/input/gamepad_profile.hpp>
#include <app/input/keyboard_motion.hpp>
#include <app/input/virtual_button.hpp>

#include <game/screen.hpp>

#include <util/map.hpp>
#include <math/math.hpp>

// Debugging related:
#include <util/log.hpp>
#include <format>
#include <magic_enum/magic_enum_format.hpp>

namespace engine
{
	// Internal routine to translate from a device-specific Hat-based 'analog event' into an engine-defined `Analog` value.
	template <typename NativeAnalogType, NativeAnalogType runtime_analog_offset, typename ProfileType, typename AnalogEventType>
	static std::optional<Analog> translate_hat_analog(const ProfileType& profile, const AnalogEventType& analog_event)
	{
		const auto analog_raw = static_cast<std::size_t>(analog_event.analog);
		const auto runtime_offset = static_cast<std::size_t>(runtime_analog_offset); // NativeAnalogType::RuntimeAnalogOffset

		if (analog_raw < runtime_offset)
		{
			// Statically-defined analogs are not supported by this routine.
			return std::nullopt;
		}

		const auto hat_as_analog = static_cast<NativeAnalogType>(analog_raw); // - runtime_offset

		const auto& mapping = profile.analog_mapping;

		auto hat_mapping_it = mapping.find(hat_as_analog);
		
		if (hat_mapping_it == mapping.end())
		{
			return std::nullopt;
		}

		return static_cast<Analog>(hat_mapping_it->second);
	}

	// Implementation of threshold detection for virtual buttons.
	template <typename ProfileType, typename AnalogEventType>
	static void handle_virtual_button_simulation_impl
	(
		InputSystem& input_system,
		InputSource source, InputSystem::StateIndex state_index,
		const ProfileType& profile, const AnalogEventType& event_data, const math::Vector2D& value
	)
	{
		const auto& virtual_button_mapping = profile.virtual_button_mapping;
		const auto& analog = event_data.analog;

		const auto vb_it = virtual_button_mapping.find(analog);

		if (vb_it == virtual_button_mapping.end())
		{
			return;
		}

		const auto& virtual_buttons = vb_it->second;

		for (const auto& virtual_button : virtual_buttons)
		{
			const auto button = static_cast<Button>(virtual_button.engine_button);

			if (virtual_button.is_down(value))
			{
				input_system.on_button_down(source, state_index, button);
			}
			else
			{
				const auto& state = input_system.peek_state_data(state_index);

				if (state.previous.held.get_button(button))
				{
					input_system.on_button_up(source, state_index, button);
				}
			}
		}
	}

	math::Vector2D InputSystem::mouse_motion_to_analog_input(int mouse_x, int mouse_y, float mouse_sensitivity, int screen_width, int screen_height)
	{
		auto half_screen_width = static_cast<float>(screen_width / 2);
		auto half_screen_height = static_cast<float>(screen_height / 2);

		return math::Vector2D
		{
			math::clamp(((static_cast<float>(mouse_x) * mouse_sensitivity) / half_screen_width), -1.0f, 1.0f),
			math::clamp(((static_cast<float>(-mouse_y) * mouse_sensitivity) / half_screen_height), -1.0f, 1.0f)
		};
	}

	InputSystem::InputSystem(Service& service, InputHandler& input_handler, const game::Screen& screen, bool subscribe_immediately)
		: BasicSystem(service), input_handler(input_handler), screen(screen)
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

		// Mouse:
		service.register_event<app::input::OnMouseButtonDown,         &InputSystem::on_mouse_button_down>(*this);
		service.register_event<app::input::OnMouseButtonUp,           &InputSystem::on_mouse_button_up>(*this);
		service.register_event<app::input::OnMouseMove,               &InputSystem::on_mouse_move>(*this);
		service.register_event<app::input::OnMouseScroll,             &InputSystem::on_mouse_scroll>(*this);
		service.register_event<app::input::OnMouseVirtualAnalogInput, &InputSystem::on_mouse_virtual_analog_input>(*this);

		// Keyboard:
		service.register_event<app::input::OnKeyboardButtonDown,  &InputSystem::on_keyboard_button_down>(*this);
		service.register_event<app::input::OnKeyboardButtonUp,    &InputSystem::on_keyboard_button_up>(*this);
		service.register_event<app::input::OnKeyboardAnalogInput, &InputSystem::on_keyboard_analog_input>(*this);

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
		auto state_index = resolve_state_index(gamepad);

		if (!state_index)
		{
			return;
		}

		if (opt_state_index)
		{
			if ((*state_index) != (*opt_state_index))
			{
				return;
			}
		}

		// Ensure we have data allocated for the index retrieved.
		allocate_input_data(*state_index);

		// Bind `gamepad_index` to the state-index.
		bind_gamepad(gamepad.index(), *state_index);
	}

	void InputSystem::handle_gamepad_mappings(GamepadIndex gamepad_index, std::optional<StateIndex> opt_state_index)
	{
		const auto& gamepads = input_handler.get_gamepads();
		const auto& gamepad  = gamepads.get_gamepad(gamepad_index);
		
		handle_gamepad_mappings(gamepad, opt_state_index);
	}

	void InputSystem::handle_virtual_button_simulation
	(
		InputSource source, StateIndex state_index, const MouseProfile& profile,
		const MouseAnalogEvent& event_data, const math::Vector2D& value
	)
	{
		handle_virtual_button_simulation_impl(*this, source, state_index, profile, event_data, value);
	}

	void InputSystem::handle_virtual_button_simulation
	(
		InputSource source, StateIndex state_index, const GamepadProfile& profile,
		const GamepadAnalogEvent& event_data, const math::Vector2D& value
	)
	{
		handle_virtual_button_simulation_impl(*this, source, state_index, profile, event_data, value);
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
			// NOTE: Multiple gamepad devices can map to the same index/player.
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

	void InputSystem::on_update(const OnServiceUpdate& data)
	{
		if (!allowed_service(*data.service))
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
				service->event<OnInput>(service, std::monostate{}, index, state_data.next, state_data.previous);

				// Copy the current state into the previous state.
				state_data.previous = state_data.next;

				// Force-update the 'pressed' and 'released' statuses to no longer be active:
				state_data.next.pressed.clear();
				state_data.next.released.clear();

				// Reset the 'state changed' flag.
				state_data.state_has_changed = false;
			}

			//print("MOVE: {}", state_data.next.directional_input.movement);

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
			service->event<OnButtonPressed>(service, source, state_index, state, button);
		}

		// Trigger the continuous 'button held/down' event.
		service->event<OnButtonDown>(service, source, state_index, state, button);
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
		service->event<OnButtonReleased>(service, source, state_index, state, button);
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
		service->event<OnAnalogInput>(service, source, state_index, state, analog, value, angle_out);
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

	std::optional<Button> InputSystem::translate_button(const Mouse& mouse, const MouseButtonEvent& button_event) const
	{
		const auto* profile = mouse.get_profile();

		if (!profile)
		{
			//print("MOUSE PROFILE MISSING.");

			return std::nullopt;
		}

		// No need to cast, since the event (currently) uses a proper enum-class type.
		// TODO: Look into whether we want to continue using the enum type in this context.
		const auto& mouse_button_raw = button_event.button;
		//auto mouse_button_raw = static_cast<app::input::MouseButtonID>(button_event.button);

		const auto& button_mapping = profile->button_mapping;
		const auto it = button_mapping.find(mouse_button_raw);
		
		if (it != button_mapping.end())
		{
			return static_cast<Button>(it->second);
		}

		return std::nullopt;
	}

	std::optional<Button> InputSystem::translate_button(const Keyboard& keyboard, const KeyboardButtonEvent& button_event) const
	{
		const auto* profile = keyboard.get_profile();

		if (!profile)
		{
			//print("KEYBOARD PROFILE MISSING.");

			return std::nullopt;
		}

		const auto& keyboard_button = button_event.button;

		const auto& button_mapping = profile->button_mapping;
		const auto it = button_mapping.find(keyboard_button);

		if (it != button_mapping.end())
		{
			return static_cast<Button>(it->second);
		}

		return std::nullopt;
	}

	std::optional<Button> InputSystem::translate_button(const GamepadProfile& gamepad_profile, const GamepadButtonEvent& button_event) const
	{
		return translate_button_impl<GamepadProfile, GamepadButtonEvent, app::input::GamepadButtonBits>(gamepad_profile, button_event);
	}

	std::optional<Button> InputSystem::translate_button(const Gamepad& gamepad, const GamepadButtonEvent& button_event) const
	{
		const auto& gamepads = input_handler.get_gamepads();
		const auto* profile = gamepads.get_profile(button_event.device_index);

		if (!profile)
		{
			//print("GAMEPAD PROFILE MISSING.");

			return std::nullopt;
		}

		return translate_button(*profile, button_event);
	}

	std::optional<Analog> InputSystem::translate_analog(const GamepadProfile& gamepad_profile, const GamepadAnalogEvent& analog_event) const
	{
		return translate_analog_impl(gamepad_profile, analog_event);
	}

	std::optional<Analog> InputSystem::translate_analog(const Gamepad& gamepad, const GamepadAnalogEvent& analog_event) const
	{
		const auto& gamepads = input_handler.get_gamepads();
		const auto* profile = gamepads.get_profile(analog_event.device_index);

		if (!profile)
		{
			//print("GAMEPAD PROFILE MISSING.");

			return std::nullopt;
		}

		return translate_analog(*profile, analog_event);
	}

	std::optional<Analog> InputSystem::translate_analog(const KeyboardProfile& keyboard_profile, const KeyboardAnalogEvent& analog_event) const
	{
		// NOTES:
		// * Statically-defined analogs are not currently supported for `Keyboard` devices.
		// * The `translate_hat_analog` function handles cases where `analog_event.analog` is not within the Hat-designated range/offset.
		return translate_hat_analog<app::input::KeyboardMotion, app::input::KeyboardMotion::RuntimeAnalogOffset>(keyboard_profile, analog_event); // 0
	}

	std::optional<Analog> InputSystem::translate_analog(const MouseProfile& mouse_profile, const MouseAnalogEvent& analog_event) const
	{
		return translate_analog_impl(mouse_profile, analog_event);
	}

	void InputSystem::on_mouse_button_down(const app::input::OnMouseButtonDown& data)
	{
		if (!is_supported_event(data))
		{
			return;
		}

		const auto& mouse = input_handler.get_mouse();

		if (auto button = translate_button(mouse, data))
		{
			if (auto state_index = resolve_state_index(mouse))
			{
				on_button_down(mouse, *state_index, *button);
			}
		}
	}

	void InputSystem::on_mouse_button_up(const app::input::OnMouseButtonUp& data)
	{
		if (!is_supported_event(data))
		{
			return;
		}

		const auto& mouse = input_handler.get_mouse();

		if (auto button = translate_button(mouse, data))
		{
			if (auto state_index = resolve_state_index(mouse))
			{
				on_button_up(mouse, *state_index, *button);
			}
		}
	}

	void InputSystem::on_mouse_move(const app::input::OnMouseMove& data)
	{
		assert(data.analog == app::input::MouseMotion::Movement);

		on_mouse_analog_input_impl
		(
			data, [this](const auto& mouse, const auto& profile, const auto& event_data, auto state_index, auto engine_analog)
			{
				const auto& [screen_width, screen_height] = this->screen.get_size();

				return mouse_motion_to_analog_input
				(
					event_data.x, event_data.y, profile.sensitivity,
					screen_width, screen_height
				);
			}
		);
	}

	void InputSystem::on_mouse_scroll(const app::input::OnMouseScroll& data)
	{
		assert(data.analog == app::input::MouseMotion::Scroll);

		on_mouse_analog_input_impl
		(
			data, [this](const auto& mouse, const auto& profile, const auto& event_data, auto state_index, auto engine_analog)
			{
				return math::Vector2D { math::sign(event_data.wheel_x), math::sign(event_data.wheel_y) };
			}
		);
	}

	void InputSystem::on_mouse_virtual_analog_input(const app::input::OnMouseVirtualAnalogInput& data)
	{
		assert(data.analog >= app::input::MouseMotion::RuntimeAnalogOffset);

		on_mouse_analog_input_impl
		(
			data, [this](const auto& mouse, const auto& profile, const auto& event_data, auto state_index, auto engine_analog)
			{
				return event_data.value;
			},

			true // false
		);
	}

	void InputSystem::on_keyboard_button_down(const app::input::OnKeyboardButtonDown& data)
	{
		if (!is_supported_event(data))
		{
			return;
		}

		const auto& keyboard = input_handler.get_keyboard();

		if (auto button = translate_button(keyboard, data))
		{
			if (auto state_index = resolve_state_index(keyboard))
			{
				on_button_down(keyboard, *state_index, *button);
			}
		}
	}

	void InputSystem::on_keyboard_button_up(const app::input::OnKeyboardButtonUp& data)
	{
		if (!is_supported_event(data))
		{
			return;
		}

		const auto& keyboard = input_handler.get_keyboard();

		if (auto button = translate_button(keyboard, data))
		{
			if (auto state_index = resolve_state_index(keyboard))
			{
				on_button_up(keyboard, *state_index, *button);
			}
		}
	}

	void InputSystem::on_keyboard_analog_input(const app::input::OnKeyboardAnalogInput& data)
	{
		if (!is_supported_event(data))
		{
			return;
		}

		const auto& keyboard = input_handler.get_keyboard();

		const auto* profile = keyboard.get_profile();

		if (!profile)
		{
			// No Hat definitions available.
			return;
		}

		if (auto analog = translate_analog(*profile, data))
		{
			if (auto state_index = resolve_state_index(keyboard))
			{
				on_analog_input(keyboard, *state_index, *analog, data.value, data.angle());
			}
		}
	}

	void InputSystem::on_gamepad_connected(const app::input::OnGamepadConnected& data)
	{
		// Debugging related:
		//print("Device connected: {}", data.device_index);

		handle_gamepad_mappings(data.device_index);
	}
	
	void InputSystem::on_gamepad_disconnected(const app::input::OnGamepadDisconnected& data)
	{
		// Debugging related:
		//print("Device disconnected: {}", data.device_index);

		unbind_gamepad(data.device_index);
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

		const auto* profile = gamepads.get_profile(gamepad_id);

		if (!profile)
		{
			//print("GAMEPAD PROFILE MISSING.");

			return;
		}

		if (auto analog = translate_analog(*profile, data))
		{
			if (auto state_index = get_gamepad_state_index(gamepad_id))
			{
				const auto& value = data.value;
				const auto angle = data.angle();

				on_analog_input(gamepad, *state_index, *analog, value, angle);

				handle_virtual_button_simulation(gamepad, *state_index, *profile, data, value);
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

	const InputSystem::Mouse& InputSystem::get_mouse() const
	{
		return input_handler.get_mouse();
	}

	const InputSystem::Keyboard& InputSystem::get_keyboard() const
	{
		return input_handler.get_keyboard();
	}

	const InputSystem::MouseProfile* InputSystem::get_profile(const Mouse& mouse) const
	{
		return mouse.get_profile();
	}

	const InputSystem::KeyboardProfile* InputSystem::get_profile(const Keyboard& keyboard) const
	{
		return keyboard.get_profile();
	}

	const InputSystem::PlayerDeviceMap& InputSystem::get_player_device_map() const
	{
		return input_handler.get_player_device_map();
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

	bool InputSystem::is_supported_event(const MouseStateEvent& mouse_event) const
	{
		return (mouse_event.device_index == Mouse::DEFAULT_MOUSE_DEVICE_INDEX);
	}

	bool InputSystem::is_supported_event(const KeyboardStateEvent& keyboard_event) const
	{
		return (keyboard_event.device_index == Keyboard::DEFAULT_KEYBOARD_DEVICE_INDEX);
	}
}