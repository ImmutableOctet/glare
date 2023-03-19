#pragma once

#include "types.hpp"

#include "input_state.hpp"

#include <app/input/types.hpp>
#include <engine/basic_system.hpp>
#include <math/types.hpp>

#include <optional>

// Device-specific input events:
namespace app::input
{
	// Mouse events:
	struct MouseStateEvent;
	struct MouseButtonEvent;
	struct MouseAnalogEvent;
	struct OnMouseButtonDown;
	struct OnMouseButtonUp;
	struct OnMouseMove;
	struct OnMouseScroll;
	struct OnMouseVirtualAnalogInput;

	// Keyboard events:
	struct KeyboardStateEvent;
	struct KeyboardButtonEvent;
	struct OnKeyboardButtonDown;
	struct OnKeyboardButtonUp;
	struct OnKeyboardAnalogInput;

	// Gamepad events:
	struct GamepadButtonEvent;
	struct OnGamepadConnected;
	struct OnGamepadDisconnected;
	struct OnGamepadButtonDown;
	struct OnGamepadButtonUp;
	struct OnGamepadAnalogInput;

	class InputHandler;

	struct GamepadProfile;
	class Gamepad;

	struct MouseProfile;
	class Mouse;

	struct KeyboardProfile;
	class Keyboard;
}

namespace game
{
	class Screen;
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
			using PlayerDeviceMap = app::input::PlayerDeviceMap;

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

			using Gamepad             = app::input::Gamepad;
			using GamepadProfile      = app::input::GamepadProfile;
			using GamepadButtonEvent  = app::input::GamepadButtonEvent;
			using GamepadAnalogEvent  = app::input::OnGamepadAnalogInput;
			using GamepadIndex        = app::input::GamepadDeviceIndex;

			using Mouse               = app::input::Mouse;
			using MouseProfile        = app::input::MouseProfile;
			using MouseStateEvent     = app::input::MouseStateEvent;
			using MouseButtonEvent    = app::input::MouseButtonEvent;
			using MouseAnalogEvent    = app::input::MouseAnalogEvent;
			using MouseIndex          = app::input::MouseDeviceIndex;

			using Keyboard            = app::input::Keyboard;
			using KeyboardProfile     = app::input::KeyboardProfile;
			using KeyboardStateEvent  = app::input::KeyboardStateEvent;
			using KeyboardButtonEvent = app::input::KeyboardButtonEvent;
			using KeyboardAnalogEvent = app::input::OnKeyboardAnalogInput; // app::input::KeyboardAnalogEvent;
			using KeyboardIndex       = app::input::KeyboardDeviceIndex;

			using StateIndex          = InputStateIndex;

			InputSystem(Service& service, InputHandler& input_handler, const game::Screen& screen, bool subscribe_immediately=false);

			// Button/analog translation routines:

			// Translates a button input from a mouse device to an engine-defined equivalent.
			// This returns the engine-defined button if translation was successful.
			std::optional<Button> translate_button(const Mouse& mouse, const MouseButtonEvent& button_event) const;

			// Translates a button input from a keyboard device to an engine-defined equivalent.
			// This returns the engine-defined button if translation was successful.
			std::optional<Button> translate_button(const Keyboard& keyboard, const KeyboardButtonEvent& button_event) const;

			// Translates a button input from a gamepad device to an engine-defined equivalent.
			// This returns the engine-defined button if translation was successful.
			std::optional<Button> translate_button(const GamepadProfile& gamepad_profile, const GamepadButtonEvent& button_event) const;

			// Shorthand overload using a `Gamepad` object, rather than its profile.
			// (Uses `input_handler` to lookup the gamepad's profile by index)
			std::optional<Button> translate_button(const Gamepad& gamepad, const GamepadButtonEvent& button_event) const;

			// Translates a gamepad device's analog input into an engine-defined equivalent.
			// This returns the engine-defined 'analog' if translation was successful.
			std::optional<Analog> translate_analog(const GamepadProfile& gamepad_profile, const GamepadAnalogEvent& analog_event) const;

			// Shorthand overload using a `Gamepad` object, rather than its profile.
			// (Uses `input_handler` to lookup the gamepad's profile by index)
			std::optional<Analog> translate_analog(const Gamepad& gamepad, const GamepadAnalogEvent& analog_event) const;

			// Translates a keyboard's 'analog' input into an engine-defined equivalent.
			// This returns the engine-defined 'analog' if translation was successful.
			std::optional<Analog> translate_analog(const KeyboardProfile& keyboard_profile, const KeyboardAnalogEvent& analog_event) const;

			// Translates a mouse's 'analog' input into an engine-defined equivalent.
			// This returns the engine-defined 'analog' if translation was successful.
			std::optional<Analog> translate_analog(const MouseProfile& mouse_profile, const MouseAnalogEvent& analog_event) const;

			// Mouse events:
			void on_mouse_button_down(const app::input::OnMouseButtonDown& data);
			void on_mouse_button_up(const app::input::OnMouseButtonUp& data);
			void on_mouse_move(const app::input::OnMouseMove& data);
			void on_mouse_scroll(const app::input::OnMouseScroll& data);
			void on_mouse_virtual_analog_input(const app::input::OnMouseVirtualAnalogInput& data);

			// Keyboard events:
			void on_keyboard_button_down(const app::input::OnKeyboardButtonDown& data);
			void on_keyboard_button_up(const app::input::OnKeyboardButtonUp& data);
			void on_keyboard_analog_input(const app::input::OnKeyboardAnalogInput& data);

			// Gamepad events:
			void on_gamepad_connected(const app::input::OnGamepadConnected& data);
			void on_gamepad_disconnected(const app::input::OnGamepadDisconnected& data);
			void on_gamepad_button_down(const app::input::OnGamepadButtonDown& data);
			void on_gamepad_button_up(const app::input::OnGamepadButtonUp& data);
			void on_gamepad_analog_input(const app::input::OnGamepadAnalogInput& data);

			// Binds a gamepad by its device index to a high-level input state via its own index.
			void bind_gamepad(GamepadIndex gamepad_index, StateIndex state_index);

			// Unbinds a gamepad from all high-level state indices.
			void unbind_gamepad(GamepadIndex gamepad_index);

			// Handles button-down changes for the state specified.
			void on_button_down(InputSource source, StateIndex state_index, Button button);

			// Handles button-up changes for the state specified.
			void on_button_up(InputSource source, StateIndex state_index, Button button);

			// Handles analog input for the state specified.
			void on_analog_input(InputSource source, StateIndex state_index, Analog analog, const math::Vector2D& value, std::optional<float> angle=std::nullopt);

			// Retrieves a temporary reference to the data associated with an input-state index.
			const StateData& peek_state_data(StateIndex index) const;
		protected:
			const Mouse& get_mouse() const;
			const Keyboard& get_keyboard() const;

			const MouseProfile* get_profile(const Mouse& mouse) const;
			const KeyboardProfile* get_profile(const Keyboard& keyboard) const;

			// Generates a floating-point vector with values between -1.0 and 1.0, based on the sensitivity and screen size specified.
			// This routine is useful for translating a relative mouse input into a format usable by regular analog input routines. (i.e. gamepad-oriented)
			static math::Vector2D mouse_motion_to_analog_input(int mouse_x, int mouse_y, float mouse_sensitivity, int screen_width, int screen_height);

			const PlayerDeviceMap& get_player_device_map() const;

			// Retrieves the state-index associated with a device.
			// (Lookup is performed using the device's name)
			template <typename DeviceType>
			std::optional<StateIndex> resolve_state_index(const DeviceType& device) const
			{
				const auto& device_mapping = get_player_device_map();

				// TODO: Look into optimizing this via heterogeneous lookup.
				if (const auto it = device_mapping.find(device.get_device_name()); it != device_mapping.end()) // peek_device_name()
				{
					return static_cast<StateIndex>(it->second);
				}

				return std::nullopt;
			}

			// Basic implementation for device-specific analog to engine-defined `Analog` translation, using a `profile` (`InputProfile`) object.
			// TODO: Add handling of Hat-based analogs to this implementation.
			template <typename ProfileType, typename AnalogEventType>
			std::optional<Analog> translate_analog_impl(const ProfileType& profile, const AnalogEventType& analog_event) const
			{
				const auto& analog = analog_event.analog;
				const auto& analog_mapping = profile.analog_mapping;
				
				if (const auto it = analog_mapping.find(analog); it != analog_mapping.end())
				{
					return static_cast<Analog>(it->second);
				}

				return std::nullopt;
			}

			// Basic implementation for device-specific button to engine-defined `Button` translation, using a `profile` (`InputProfile`) object.
			template <typename ProfileType, typename ButtonEventType, typename TrueButtonType=decltype(ButtonEventType::button)>
			std::optional<Button> translate_button_impl(const ProfileType& profile, const ButtonEventType& button_event) const
			{
				const auto native_button = static_cast<TrueButtonType>(button_event.button);
				const auto& button_mapping = profile.button_mapping;

				if (const auto it = button_mapping.find(native_button); it != button_mapping.end())
				{
					return static_cast<Button>(it->second);
				}

				return std::nullopt;
			}

			template <typename MouseAnalogEventType, typename Callback>
			std::optional<StateIndex> on_mouse_analog_input_impl(const MouseAnalogEventType& data, Callback&& callback, bool check_virtual_buttons=true)
			{
				if (!is_supported_event(data))
				{
					return std::nullopt;
				}

				const auto& mouse = get_mouse();
				const auto* profile = get_profile(mouse);

				if (!profile)
				{
					return std::nullopt;
				}

				auto state_index = resolve_state_index(mouse);

				if (!state_index)
				{
					return std::nullopt;
				}

				auto engine_analog = translate_analog(*profile, data);

				if (!engine_analog)
				{
					return std::nullopt;
				}

				auto value = callback(mouse, *profile, data, *state_index, *engine_analog);

				on_analog_input(mouse, *state_index, *engine_analog, value);

				if (check_virtual_buttons)
				{
					handle_virtual_button_simulation(mouse, *state_index, *profile, data, value);
				}

				return state_index;
			}

			// Triggered when this system subscribes to a service.
			bool on_subscribe(Service& service) override;

			// Triggered once per service update; used to handle
			// things like 'state changed' (`OnInput`) events, etc.
			void on_update(const OnServiceUpdate& data);

			// Called by each of the state event-triggers.
			// (Used to update the `state_has_changed` flag, etc.)
			void on_state_update(StateData& state_data);

			// Retrieves gamepad device mappings from `input_handler`.
			// If `opt_state_index` is specified, this will only apply device mappings
			// where the intended state-index/player is equal to the value provided.
			// NOTE: `input_handler` maps state/player indices using the device's name internally.
			void handle_gamepad_mappings(GamepadIndex gamepad_index, std::optional<StateIndex> opt_state_index=std::nullopt);

			// See index-based overload for details. (Convenience overload)
			void handle_gamepad_mappings(const Gamepad& gamepad, std::optional<StateIndex> opt_state_index=std::nullopt);

			// Handles threshold detection for virtual buttons tied to a mouse.
			// Called automatically by `on_mouse_analog_input_impl`.
			void handle_virtual_button_simulation
			(
				InputSource source, StateIndex state_index, const MouseProfile& profile,
				const MouseAnalogEvent& event_data, const math::Vector2D& value
			);

			// Handles threshold detection for virtual buttons tied to a gamepad.
			// Called automatically by `on_gamepad_analog_input`.
			void handle_virtual_button_simulation
			(
				InputSource source, StateIndex state_index, const GamepadProfile& profile,
				const GamepadAnalogEvent& event_data, const math::Vector2D& value
			);

			// Called when the first concurrent listener for `index` is registered.
			void on_start_listening(StateIndex state_index, StateData& state);

			// Called when the last listener for `index` is unregistered.
			void on_stop_listening(StateIndex state_index, StateData& state);

			// Called when an `InputComponent` has been created/attached to `entity`.
			void on_create_input(Registry& registry, Entity entity);

			// Called when an `InputComponent` has been destroyed/detached from `entity`.
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

			// Retrieves the input-state index associated with `gamepad_id`, if applicable.
			std::optional<StateIndex> get_gamepad_state_index(GamepadIndex gamepad_id) const;

			// Reference to the application's input handler.
			// (Responsible for device-specific input)
			InputHandler& input_handler;

			// Reference to the application's screen.
			// (Used for input coordinate normalization)
			// 
			// TODO: Look into lighter-weight option than the `Screen` class.
			const game::Screen& screen;

			// Container of states (previous and next/current) for each bound device.
			std::vector<StateData> states;

			// Map of gamepad indices to their corresponding high-level state indices.
			// TODO: Optimize with `std::vector`.
			std::unordered_map<GamepadIndex, StateIndex> gamepad_assignment;
		private:
			bool is_supported_event(const MouseStateEvent& mouse_event) const;
			bool is_supported_event(const KeyboardStateEvent& keyboard_event) const;
	};
}