#pragma once

#include "types.hpp"

#include <engine/action.hpp>
#include <engine/service.hpp>
#include <engine/transform.hpp>

#include "world_events.hpp"
#include "world_properties.hpp"

//#include <graphics/types.hpp> // Not actually needed. (`ColorRGB` is actually located in math)
#include <app/delta_time.hpp>

#include <math/math.hpp>
#include <util/json.hpp>

#include <string_view>
#include <filesystem>
#include <utility>
#include <variant>
#include <optional>

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
	class Config;
	struct CollisionComponent;

	class World : public Service
	{
		public:
			using UpdateRate = app::DeltaTime::Rate;
		protected:
			Config& config;
			ResourceManager& resource_manager;
			
			app::DeltaTime delta_time;

			// TODO: Allow the user to specify a registry, rather than owning it outright.
			mutable Registry registry;

			// Scene root-node; parent to all world-bound entities.
			Entity root   = null;

			// First entity of stage mesh. (Child of 'root')
			Entity stage  = null;

			// Currently-active/last-bound camera.
			Entity camera = null;

			WorldProperties properties;
		public:
			World(Config& config, ResourceManager& resource_manager, UpdateRate update_rate);

			// Constructs a 'World' object, then immediately loads a map from the 'path' specified.
			World(Config& config, ResourceManager& resource_manager, UpdateRate update_rate, const filesystem::path& path);

			virtual ~World();

			// TODO: Re-route most of these template functions to their `Service` equivalents.

			template <typename EventType, auto fn, typename obj_type>
			inline void register_event(obj_type& obj)
			{
				event_handler.sink<EventType>().connect<fn>(obj);
			}

			template <typename EventType, auto fn, typename obj_type>
			inline void unregister_event(obj_type& obj)
			{
				event_handler.sink<EventType>().disconnect<fn>(obj);
			}

			template <typename EventType, auto Fn>
			inline void register_event()
			{
				register_event<EventType, Fn>(*this);
			}

			template <typename EventType, auto Fn>
			inline void unregister_event()
			{
				unregister_event<EventType, Fn>(*this);
			}

			template <typename obj_type>
			inline void unregister(obj_type& obj)
			{
				event_handler.disconnect(obj);
			}

			template <typename subscriber_type>
			inline World& subscribe(subscriber_type& sub)
			{
				sub.subscribe(*this);

				return *this;
			}

			template <typename subscriber_type>
			inline World& unsubscribe(subscriber_type& sub)
			{
				unregister(sub);

				return *this;
			}

			template <typename EventType, auto fn>
			inline void register_free_function()
			{
				event_handler.sink<EventType>().connect<fn>();
			}

			template <typename EventType, auto fn>
			inline void unregister_free_function()
			{
				event_handler.sink<EventType>().disconnect<fn>();
			}

			Registry& get_registry() override;
			const Registry& get_registry() const override;

			Entity load(const filesystem::path& root_path, bool override_current=false, const std::string& json_file="map.json");

			void update(app::Milliseconds time);

			void handle_transform_events(float delta);

			//void on_child_removed(const Event_ChildRemoved& e);

			void update_camera_parameters(int width, int height);

			Transform apply_transform(Entity entity, const math::TransformVectors& tform);

			// See `transform_and_reset_collision` for details.
			void apply_transform_and_reset_collision(Entity entity, const math::TransformVectors& tform_data);

			Transform set_position(Entity entity, const math::Vector& position);

			Transform get_transform(Entity entity);

			// Calls `callback` with the `Transform` of `entity` while handling updates
			// to the collision-component's activation-status and world-transform.
			// For most purposes, it's recommended to use `get_transform` normally.
			// This works as a utility function for object creation and reset routines.
			template <typename callback_fn>
			inline void transform_and_reset_collision(Entity entity, callback_fn&& callback)
			{
				auto* collision = get_collision_component(entity);

				math::Matrix tform_matrix;

				bool collision_active;

				// Store activity status and deactivate.
				if (collision)
				{
					collision_active = _transform_and_reset_collision_store_active_state(collision);
				}

				// Execute `callback` and store the resulting matrix if needed:
				{
					auto tform = get_transform(entity);

					callback(tform);

					// Only store the transformation matrix if a collision component exists:
					if (collision)
					{
						tform_matrix = tform.get_matrix();
					}
				}

				// Revert the collision component's activity state and update its world-transform.
				if (collision)
				{
					_transform_and_reset_collision_revert_active_state(collision, tform_matrix, collision_active);
				}
			}

			math::Vector get_up_vector(math::Vector up={ 0.0f, 1.0f, 0.0f }) const;

			Entity get_parent(Entity entity) const;

			void set_parent
			(
				Entity entity, Entity parent,
				bool defer_action=false,
				
				bool _null_as_root=true, bool _is_deferred=false
			);

			// Returns a label for the entity specified.
			// If no `NameComponent` is associated with the entity, the entity number will be used.
			std::string label(Entity entity);

			// Returns the name associated to the `entity` specified.
			// If no `NameComponent` is associated with the entity, an empty string will be returned.
			std::string get_name(Entity entity);

			void set_name(Entity entity, const std::string& name);

			// Retrieves the first entity found with the `name` specified.
			// NOTE: Multiple entities can share the same name.
			Entity get_by_name(std::string_view name); // const;

			// Retrieves the first bone child-entity found with the name specified.
			Entity get_bone_by_name(Entity entity, std::string_view name, bool recursive=true);

			// Retrieves the first child-entity found with the name specified, regardless of other attributes/components. (includes both bone & non-bone children)
			Entity get_child_by_name(Entity entity, std::string_view child_name, bool recursive=true);

			inline ResourceManager& get_resource_manager() { return resource_manager; }

			inline const Config& get_config() const { return config; }
			
			// Retrieves the root scene-node; parent to all world-scoped entities.
			inline Entity get_root() const { return root; }

			// The actively bound camera. (Does not always represent the rendering camera)
			inline Entity get_camera() const { return camera; }
			void set_camera(Entity camera);

			inline const app::DeltaTime& get_delta_time() const { return delta_time; }

			Entity get_player(PlayerIndex player=engine::PRIMARY_LOCAL_PLAYER) const;

			math::Vector get_gravity() const;
			void set_gravity(const math::Vector& gravity);

			// Returns a normalized down vector.
			// (Direction vector of 'gravity')
			math::Vector down() const;

			const WorldProperties& get_properties() const { return properties; }

			inline float delta() const { return delta_time; }
			inline operator Entity() const { return get_root(); }

			void on_transform_change(const OnTransformChanged& tform_change);
			void on_entity_destroyed(const OnEntityDestroyed& destruct);

			// Same as `defer`, but implicitly forwards `this` prior to any arguments following the target function; useful for deferring member functions.
			template <typename fn_t, typename... arguments>
			inline void later(fn_t&& f, arguments&&... args)
			{
				defer(std::forward<fn_t>(f), this, std::forward<arguments>(args)...);
			}
		private:
			CollisionComponent* get_collision_component(Entity entity);

			bool _transform_and_reset_collision_store_active_state(CollisionComponent* collision);
			void _transform_and_reset_collision_revert_active_state(CollisionComponent* collision, const math::Matrix& tform_matrix, bool collision_active);
	};

	using Scene = World;
}