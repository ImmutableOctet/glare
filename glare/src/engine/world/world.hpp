#pragma once

#include "types.hpp"

#include <engine/action.hpp>
#include <engine/service.hpp>
#include <engine/transform.hpp>
#include <engine/delta_time.hpp>

#include "world_properties.hpp"
#include "scene_loader_config.hpp"

//#include <graphics/types.hpp> // Not actually needed. (`ColorRGB` is actually located in math)

#include <math/types.hpp>
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
	class SystemManagerInterface;
	class ResourceManager;
	class Config;

	class DeltaSystem;

	struct LightComponent;
	struct DirectionalLightComponent;
	struct PointLightComponent;
	struct SpotLightComponent;
	struct CollisionComponent;

	struct AnimationData;

	struct OnTransformChanged;

	class World : public Service
	{
		public:
			using UpdateRate = DeltaTime::Rate;
			
			World
			(
				Registry& registry,
				SystemManagerInterface& systems,
				Config& config,
				ResourceManager& resource_manager,
				UpdateRate update_rate
			);

			// Constructs a 'World' object, then immediately loads a map from the 'path' specified.
			World
			(
				Registry& registry,
				SystemManagerInterface& systems,
				Config& config,
				ResourceManager& resource_manager,
				UpdateRate update_rate,
				const filesystem::path& path
			);

			virtual ~World();

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

			Entity load
			(
				const filesystem::path& scene_root_path,
				const std::string_view scene_filename="map.json",
				Entity parent=null,
				const SceneLoaderConfig& cfg={},
				SystemManagerInterface* opt_system_manager=nullptr
			);

			void update(app::Milliseconds time);

			// If `entity` has a `ForwardingComponent` attached, this returns the `root_entity` from that component.
			// If no `ForwardingComponent` is found, `entity` is returned back to the caller.
			Entity get_forwarded(Entity entity);

			// Applies the transformation vectors specified to `entity`.
			// This routine does not explicitly handle side effects of collision.
			// See also: `apply_transform_and_reset_collision`
			Transform apply_transform(Entity entity, const math::TransformVectors& tform);

			// See `transform_and_reset_collision` for details.
			void apply_transform_and_reset_collision(Entity entity, const math::TransformVectors& tform_data);

			// Utility function that allows you to manually specify the world-space
			// position of `entity` without needing to manage a `Transform` object.
			// 
			// NOTE: This is generally less efficient than simply using a `Transform` object directly.
			Transform set_position(Entity entity, const math::Vector& position);

			// Simple utility function for constructing a `Transform` object,
			// without needing to specify each dependent component/object-reference.
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

			inline void set_position_and_reset_collision(Entity entity, const math::Vector& position)
			{
				transform_and_reset_collision(entity, [&position](Transform& tform)
				{
					tform.set_position(position);
				});
			}

			math::Vector get_up_vector(math::Vector up={ 0.0f, 1.0f, 0.0f }) const;

			const AnimationData* get_animation_data(Entity entity) const;
			const AnimationData* get_animation_data(Registry& registry, Entity entity) const;

			// Retrieves the first bone child-entity found with the ID specified.
			Entity get_bone_by_id(Entity entity, BoneID bone_id, bool recursive=true);

			// Retrieves the first bone child-entity found with the name specified.
			Entity get_bone_by_name(Entity entity, std::string_view name, bool recursive=true);

			// Retrieves the first bone child-entity found with the index specified.
			Entity get_bone_by_index(Entity entity, BoneIndex bone_index, bool recursive=true);

			ResourceManager& get_resource_manager() override;
			virtual const ResourceManager& get_resource_manager() const override;

			inline const Config& get_config() const { return config; }
			
			// Retrieves the root scene-node; parent to all world-scoped entities.
			inline Entity get_root() const { return root; }

			const DeltaSystem* get_delta_system() const;
			DeltaSystem* get_delta_system();

			const DeltaTime& get_delta_time() const;

			float get_delta() const;

			// The actively bound camera.
			// NOTE: Does not always represent the rendering camera.
			Entity get_camera() const;
			void set_camera(Entity camera);

			math::Vector get_gravity() const;
			void set_gravity(const math::Vector& gravity);

			// Returns a normalized down vector.
			// (Direction vector of 'gravity')
			math::Vector down() const;

			void set_properties(const WorldProperties& properties);
			const WorldProperties& get_properties() const { return properties; }

			inline operator Entity() const { return get_root(); }

			void on_transform_changed(const OnTransformChanged& tform_change);

			// Same as `defer`, but implicitly forwards `this` prior to any arguments following the target function; useful for deferring member functions.
			template <typename fn_t, typename... arguments>
			inline void later(fn_t&& f, arguments&&... args)
			{
				defer(std::forward<fn_t>(f), this, std::forward<arguments>(args)...);
			}

		protected:
			// Ensure the relevant light (sub)-component is associated to `entity`.
			// The return value is an opaque pointer to the component associated with `type`.
			static void* ensure_light_sub_component(Registry& registry, Entity entity, LightType type);

			// Removes all light components except for the component-type associated with `except_type` (if specified).
			static std::size_t remove_light_sub_components(Registry& registry, Entity entity, std::optional<LightType> except_type=std::nullopt);

			Config& config;
			ResourceManager& resource_manager;

			WorldProperties properties;

			void handle_transform_events(float delta);

			// Enforces explicit light-type rules, as well as handling shadow sub-components.
			// (i.e. only one sub-component at a time unless `light_comp.type` == `LightType::Any`)
			// 
			// The return-value indicates how many light sub-components were removed, if any.
			std::size_t update_light_type(Entity entity, LightType light_type);

			// Explicit overload. (Allows user to specify `LightComponent` instance)
			std::size_t update_light_type(Entity entity, LightType light_type, LightComponent& light_comp);

			bool has_light_shadow_sub_components(Entity entity) const;

			// Attempts to attach shadows to `entity` for the `light_type` specified.
			bool attach_shadows(Entity entity, LightType light_type);

			// Attempts to attach shadows to `entity` for
			// each light sub-component currently attached.
			bool attach_shadows(Entity entity);

			void on_instance(Registry& registry, Entity entity);

			void on_light_init(Registry& registry, Entity entity);
			void on_light_update(Registry& registry, Entity entity);
			void on_light_destroyed(Registry& registry, Entity entity);

			void on_directional_light_init(Registry& registry, Entity entity);
			void on_directional_light_update(Registry& registry, Entity entity);
			void on_directional_light_destroyed(Registry& registry, Entity entity);

			void on_point_light_init(Registry& registry, Entity entity);
			void on_point_light_update(Registry& registry, Entity entity);
			void on_point_light_destroyed(Registry& registry, Entity entity);

			void on_spot_light_init(Registry& registry, Entity entity);
			void on_spot_light_update(Registry& registry, Entity entity);
			void on_spot_light_destroyed(Registry& registry, Entity entity);

		private:
			// Cache for `DeltaSystem` instance. (NOTE: Unsafe)
			mutable DeltaSystem* _delta_system = {};

			CollisionComponent* get_collision_component(Entity entity);

			bool _transform_and_reset_collision_store_active_state(CollisionComponent* collision);
			void _transform_and_reset_collision_revert_active_state(CollisionComponent* collision, const math::Matrix& tform_matrix, bool collision_active);
	};
}