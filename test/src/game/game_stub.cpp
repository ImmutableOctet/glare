// Stub (empty) implementation of game-level functions.
// (Used to circumvent problems during the link step)

#include "game_stub.hpp"

//#include <engine/input/reflection.hpp>
#include <engine/reflection/reflect.hpp>

#include <util/api.hpp>

namespace game
{
	// InputAnalogStates:
	InputAnalogStates::DirectionVector InputAnalogStates::get_analog(Analog analog) const { return {}; }
	void InputAnalogStates::set_analog(Analog analog, const DirectionVector& value) {}
	void InputAnalogStates::emit_continuous_input_events(engine::Service& service, engine::InputStateIndex input_state_index, const engine::InputState& input_state) const {}
}

namespace engine
{
	template <>
	GLARE_GAME_EXPORT void reflect<Button>()
	{
		engine::impl::reflect_default_impl<Button>();
	}

	template <>
	GLARE_GAME_EXPORT void reflect<Analog>()
	{
		engine::impl::reflect_default_impl<Analog>();
	}
}