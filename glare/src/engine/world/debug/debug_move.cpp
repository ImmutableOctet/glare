#include "debug_move.hpp"

#include <engine/world/world.hpp>
#include <engine/transform.hpp>

#include <app/input/types.hpp>
//#include <math/math.hpp>

#include <app/input/keycodes.hpp>

// Debugging related:
#include <iostream>

namespace engine
{
	namespace debug
	{
		void DebugMove::update(World& world, const KeyboardState& keyboard_state)
		{
			auto& registry = world.get_registry();

			registry.view<DebugMove>().each([&](auto entity, auto& debug_move)
			{
				auto transform = world.get_transform(entity);

				debug_move.apply(world, entity, transform, keyboard_state);
			});
		}

		void DebugMove::apply(World& world, Entity entity, Transform& transform, const KeyboardState& input)
		{
			//return;

			auto delta = world.delta();

			auto h_movement_speed = movement_speed;
			auto v_movement_speed = movement_speed;

			// Horizontal:
			math::Vector movement = {};

			if (input.get_key(SDL_SCANCODE_W))
			{
				movement.z -= 1.0f;
			}

			if (input.get_key(SDL_SCANCODE_S))
			{
				movement.z += 1.0f;
			}

			if (input.get_key(SDL_SCANCODE_A))
			{
				movement.x -= 1.0f;
			}

			if (input.get_key(SDL_SCANCODE_D))
			{
				movement.x += 1.0f;
			}

			if (input.get_key(SDL_SCANCODE_LSHIFT))
			{
				//h_movement_speed *= 2.0;
				h_movement_speed *= 0.5;
			}

			transform.move((movement * h_movement_speed * delta), true);

			// Vertical:
			movement = {};

			if (input.get_key(SDL_SCANCODE_SPACE))
			{
				movement.y += 1.0f;
			}

			if (input.get_key(SDL_SCANCODE_LCTRL))
			{
				movement.y -= 1.0f;
			}

			transform.move((movement * v_movement_speed * delta), false);
		}
	}
}