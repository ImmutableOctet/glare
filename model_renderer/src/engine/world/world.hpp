#pragma once

#include <engine/types.hpp>
#include <vector>

#include "entity.hpp"
#include "graphics_entity.hpp"
#include "camera.hpp"

namespace app
{
	namespace input
	{
		class InputHandler;
		struct MouseState;
		struct KeyboardState;
	}
}

namespace engine
{
	class World
	{
		protected:
			Registry registry;

			EventHandler event_handler;

			template <typename EventType, auto fn>
			inline void register_event()
			{
				event_handler.sink<EventType>().connect<fn>(*this);
			}
		public:
			std::vector<Entity> cameras;

			World();

			void update(float delta=1.0f);

			//void on_child_removed(const Event_ChildRemoved& e);

			Transform get_transform(Entity entity);

			inline Registry& get_registry() { return registry; }
			inline EventHandler& get_event_handler() { return event_handler; }

			void on_mouse_input(const app::input::MouseState& mouse);
			void on_keyboard_input(const app::input::KeyboardState& keyboard);
	};
}