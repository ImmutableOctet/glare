#pragma once

#include "types.hpp"
//#include <types.hpp>

#include "input_state.hpp"

#include <app/input/types.hpp>
#include <engine/basic_system.hpp>
#include <math/types.hpp>

#include <optional>

// Device-specific input events:
namespace app::input
{
	struct GamepadButtonEvent;

	struct OnGamepadConnected;
	struct OnGamepadDisconnected;
	struct OnGamepadButtonDown;
	struct OnGamepadButtonUp;
	struct OnGamepadAnalogInput;

	class InputHandler;

	struct GamepadProfile;
	class Gamepad;
}

namespace engine
{
	class Service;

	// Events:
	struct OnServiceUpdate;

	// TODO: Implement continuous events for gamepad's button-down state. (Doesn't seem to be working right)

	// Translates inputs from different sources into a unified event stream.
	class InputSystem : public BasicSystem
	{
		public:
			using InputHandler = app::input::InputHandler;

			// Storage for next and previous input states.
			struct StateData
			{
				// Previous frame/update's input state.
				InputState previous;

				// Next input state; updated as events are received,
				// flushed to current state periodically via state events (`OnInput`).
				InputState next;

				// Indicates the number of active listeners.
				// (Attached `InputComponent` objects)
				std::uint16_t active_listeners = 0;

				// Indicates if `next` has changed from `previous`.
				bool state_has_changed : 1 = false;
			};

			using Gamepad            = app::input::Gamepad;
			using GamepadProfile     = app::input::GamepadProfile;
			using GamepadButtonEvent = app::input::GamepadButtonEvent;
			using GamepadAnalogEvent = app::input::OnGamepadAnalogInput;
			using GamepadIndex       = app::input::GamepadDeviceIndex;
			using StateIndex         = InputStateIndex;

			InputSystem(Service& service, InputHandler& input_handler, bool subscribe_immediately=false);

			// Button/analog translation routines:
			std::optional<Button> translate_button(const Gamepad& gamepad, const GamepadButtonEvent& button_event) const;
			std::optional<Analog> translate_analog(const Gamepad& gamepad, const GamepadAnalogEvent& analog_event) const;

			// Gamepad events:
			void on_gamepad_connected(const app::input::OnGamepadConnected& data);
			void on_gamepad_disconnected(const app::input::OnGamepadDisconnected& data);
			void on_gamepad_button_down(const app::input::OnGamepadButtonDown& data);
			void on_gamepad_button_up(const app::input::OnGamepadButtonUp& data);
			void on_gamepad_analog_input(const app::input::OnGamepadAnalogInput& data);

			void bind_gamepad(GamepadIndex gamepad_index, StateIndex state_index);
			void unbind_gamepad(GamepadIndex gamepad_index);
		protected:
			bool on_subscribe(Service& service) override;

			// Triggered once per service update; used to handle
			// things like 'state changed' (`OnInput`) events, etc.
			void on_update(const OnServiceUpdate& data);

			// Called by each of the state event-triggers.
			// (Used to update the `state_has_changed` flag, etc.)
			void on_state_update(StateData& state_data);

			// Handles button-down changes for the state specified.
			void on_button_down(InputSource source, StateIndex state_index, Button button);

			// Handles button-up changes for the state specified.
			void on_button_up(InputSource source, StateIndex state_index, Button button);

			// Handles analog input for the state specified.
			void on_analog_input(InputSource source, StateIndex state_index, Analog analog, const math::Vector2D& value, std::optional<float> angle=std::nullopt);

			// Retrieves gamepad device mappings from `input_handler`.
			// If `opt_state_index` is specified, this will only apply device mappings
			// where the intended state-index/player is equal to the value provided.
			// NOTE: `input_handler` maps state/player indices using the device's name internally.
			void handle_gamepad_mappings(GamepadIndex gamepad_index, std::optional<StateIndex> opt_state_index=std::nullopt);

			// See index-based overload for details.
			void handle_gamepad_mappings(const Gamepad& gamepad, std::optional<StateIndex> opt_state_index=std::nullopt);

			// Called when the first concurrent listener for `index` is registered.
			void on_start_listening(StateIndex state_index, StateData& state);

			// Called when the last listener for `index` is unregistered.
			void on_stop_listening(StateIndex state_index, StateData& state);

			void on_create_input(Registry& registry, Entity entity);
			void on_destroy_input(Registry& registry, Entity entity);

			// Changes (updates via a `Registry`) are considered forbidden.
			// This method causes a failed assert to trigger.
			void on_change_input(Registry& registry, Entity entity);

			// Ensures data is allocated in `states` for the `index` specified, then returns a temporary reference to this data.
			// If data has already been allocated, this does nothing, then returns a reference to the data requested.
			StateData& allocate_input_data(StateIndex index);

			// Internal routine to clear the contents of a `StateData` object by its index.
			void clear_input_data(StateIndex index);

			// Clears `state` to its default constructed form.
			// NOTE: This does not destruct the underlying object.
			void clear_input_data(StateData& state);

			// Retrieves a copy of the current/next input state.
			InputState get_input_state(StateIndex index) const;

			// Retrieves a temporary reference to the data associated with an input-state index.
			const StateData& peek_state_data(StateIndex index) const;

			// Retrieves the input-state index associated with `gamepad_id`, if applicable.
			std::optional<StateIndex> get_gamepad_state_index(GamepadIndex gamepad_id) const;

			InputHandler& input_handler;

			// Container of states (previous and next/current) for each bound device.
			std::vector<StateData> states;

			// TODO: Optimize with `std::vector`.
			std::unordered_map<GamepadIndex, StateIndex> gamepad_assignment;
	};
}