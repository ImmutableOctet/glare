#pragma once

#include <engine/types.hpp>
#include <engine/messages/messages.hpp>

#include <app/delta_time.hpp>

#include <vector>

#include "entity.hpp"
#include "graphics_entity.hpp"
#include "camera.hpp"

namespace app
{
	namespace input
	{
		class InputHandler;
	}
}

namespace engine
{
	class World
	{
		protected:
			const app::DeltaTime& delta_time;

			Registry registry;
			EventHandler event_handler;
		public:
			std::vector<Entity> cameras;

			World(const app::DeltaTime& delta_time);

			void update();

			template <typename EventType, auto fn>
			inline void register_event()
			{
				event_handler.sink<EventType>().connect<fn>(*this);
			}

			//void on_child_removed(const Event_ChildRemoved& e);

			Transform get_transform(Entity entity);

			inline Registry& get_registry() { return registry; }
			inline EventHandler& get_event_handler() { return event_handler; }
			inline float delta() { return delta_time; }

			void on_mouse_input(const app::input::MouseState& mouse);
			void on_keyboard_input(const app::input::KeyboardState& keyboard);
	};

	using Scene = World;
}