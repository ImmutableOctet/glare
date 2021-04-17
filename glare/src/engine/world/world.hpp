#pragma once

#include <engine/types.hpp>
#include <engine/events/events.hpp>
#include <graphics/types.hpp>

#include <app/delta_time.hpp>

#include <math/math.hpp>

#include <vector>
#include <string_view>
#include <filesystem>
#include <utility>

#include "entity.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "player.hpp"
#include "physics.hpp"

#include "graphics_entity.hpp"

namespace filesystem = std::filesystem;

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
	class ResourceManager;

	class World
	{
		public:
			using UpdateRate = app::DeltaTime::Rate;
		protected:
			ResourceManager& resource_manager;
			
			app::DeltaTime delta_time;

			mutable Registry registry;
			EventHandler event_handler;

			PhysicsSystem physics;

			// Scene root-node; parent to all world-bound entities.
			Entity root   = null;

			// First entity of stage mesh. (Child of 'root')
			Entity stage  = null;

			// Currently-active/last-bound camera.
			Entity camera = null;
		public:
			std::vector<Entity> cameras;

			World(ResourceManager& resource_manager, UpdateRate update_rate);

			// Constructs a 'World' object, then immediately loads a map from the 'path' specified.
			World(ResourceManager& resource_manager, UpdateRate update_rate, const filesystem::path& path);

			Entity load(const filesystem::path& root_path, bool override_current=false, const std::string& json_file="map.json");

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

			template <typename EventType, typename... Args>
			inline void queue_event(Args&&... args)
			{
				event_handler.enqueue<EventType>(std::forward<Args>(args)...);
			}

			template <typename EventType>
			inline void queue_event(EventType&& event_obj)
			{
				event_handler.enqueue(std::forward<EventType>(event_obj));
			}

			template <typename EventType, typename... Args>
			inline void event(Args&&... args)
			{
				event_handler.trigger<EventType>(std::forward<Args>(args)...);
			}

			template <typename EventType>
			inline void event(EventType&& event_obj)
			{
				event_handler.trigger(std::forward<EventType>(event_obj));
			}

			void update(app::Milliseconds time);

			// Renders the scene using the last bound camera. If no camera has been bound/assinged, then this routine will return 'false'.
			// Returns 'false' if an essential rendering component is missing.
			bool render(graphics::Canvas& canvas, const graphics::Viewport& viewport, bool multi_pass=false, bool use_active_shader=false, graphics::CanvasDrawMode additional_draw_modes=graphics::CanvasDrawMode::None, bool _combine_view_proj_matrices=false); // (graphics::CanvasDrawMode::IgnoreShaders)

			// Renders the scene using the camera specified.
			// Returns 'false' if an essential rendering component is missing. (e.g. 'camera')
			bool render(graphics::Canvas& canvas, const graphics::Viewport& viewport, Entity camera, bool multi_pass=false, bool use_active_shader=false, graphics::CanvasDrawMode additional_draw_modes=graphics::CanvasDrawMode::None, bool _combine_view_proj_matrices=false);

			//void on_child_removed(const Event_ChildRemoved& e);

			Transform apply_transform(Entity entity, const math::TransformVectors& tform);
			Transform set_position(Entity entity, const math::Vector& position);

			Transform get_transform(Entity entity);

			math::Vector get_up_vector(math::Vector up={ 0.0f, 1.0f, 0.0f }) const;

			Entity get_parent(Entity entity) const;
			void set_parent(Entity entity, Entity parent, bool _null_as_root=true);

			Entity get_by_name(std::string_view name); // const;

			inline Registry& get_registry() { return registry; }
			inline EventHandler& get_event_handler() { return event_handler; }
			inline ResourceManager& get_resource_manager() { return resource_manager; }

			inline PhysicsSystem& get_physics() { return physics; }
			
			// Retrieves the root scene-node; parent to all world-scoped entities.
			inline Entity get_root() const { return root; }

			// The actively bound camera. (Does not always represent the rendering camera)
			inline Entity get_camera() const { return camera; }

			inline const app::DeltaTime& get_delta_time() const { return delta_time; }

			Entity get_player(PlayerIndex player) const;

			inline math::Vector gravity() const { return physics.get_gravity(); }
			inline float delta() const { return delta_time; }
			inline operator Entity() const { return get_root(); }

			void add_camera(Entity camera, bool make_active=false);
			void remove_camera(Entity camera);

			void on_mouse_input(const app::input::MouseState& mouse);
			void on_keyboard_input(const app::input::KeyboardState& keyboard);
			void on_new_collider(const OnComponentAdd<CollisionComponent>& new_col);
			void on_entity_destroyed(const OnEntityDestroyed& destruct);
		private:
			// Renders models with the given draw-mode.
			void draw_models(graphics::CanvasDrawMode draw_mode, graphics::Canvas& canvas, const math::Matrix& projection_matrix, const math::Matrix& view_matrix, bool use_active_shader=false, bool combine_matrices=false); // graphics::Shader& shader
	};

	using Scene = World;
}