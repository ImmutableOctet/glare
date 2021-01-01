#pragma once

#include <engine/types.hpp>
#include <engine/messages/messages.hpp>
#include <graphics/types.hpp>

#include <app/delta_time.hpp>

#include <vector>
#include <string_view>

#include "entity.hpp"
#include "camera.hpp"
#include "light.hpp"

#include "graphics_entity.hpp"

namespace app
{
	namespace input
	{
		class InputHandler;
	}
}

namespace graphics
{
	class Canvas;
	class Shader;
}

namespace engine
{
	class World
	{
		protected:
			const app::DeltaTime& delta_time;

			Registry registry;
			EventHandler event_handler;

			// Scene root-node; parent to all world-bound entities.
			Entity root   = null;
			Entity camera = null;
		public:
			std::vector<Entity> cameras;

			World(const app::DeltaTime& delta_time);

			void update();

			template <typename EventType, auto fn, typename obj_type>
			inline void register_event(obj_type& obj)
			{
				event_handler.sink<EventType>().connect<fn>(obj);
			}

			template <typename EventType, auto fn>
			inline void register_event()
			{
				register_event<EventType, fn>(*this);
			}

			// Renders the scene using the last bound camera. If no camera has been bound/assinged, then this routine will return 'false'.
			// Returns 'false' if an essential rendering component is missing.
			bool render(graphics::Canvas& canvas, bool forward_rendering);

			// Renders the scene using the camera specified.
			// Returns 'false' if an essential rendering component is missing. (e.g. 'camera')
			bool render(graphics::Canvas& canvas, Entity camera, bool forward_rendering);

			//void on_child_removed(const Event_ChildRemoved& e);

			Transform get_transform(Entity entity);

			Entity get_parent(Entity entity) const;
			void set_parent(Entity entity, Entity parent);

			Entity get_by_name(std::string_view name); // const;

			inline Registry& get_registry() { return registry; }
			inline EventHandler& get_event_handler() { return event_handler; }
			
			// Retrieves the root scene-node; parent to all world-scoped entities.
			inline Entity get_root() const { return root; }

			// The actively bound camera. (Does not always represent the rendering camera)
			inline Entity get_camera() const { return camera; }

			inline float delta() const { return delta_time; }
			inline operator Entity() const { return get_root(); }

			void add_camera(Entity camera);
			void remove_camera(Entity camera);

			void on_mouse_input(const app::input::MouseState& mouse);
			void on_keyboard_input(const app::input::KeyboardState& keyboard);
		private:
			// Renders models with the given draw-mode.
			void draw_models(graphics::CanvasDrawMode draw_mode, graphics::Canvas& canvas, graphics::Shader& shader);
	};

	using Scene = World;
}