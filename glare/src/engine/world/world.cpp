#include "world.hpp"
#include "entity.hpp"
#include "camera.hpp"
#include "physics.hpp"
#include "player.hpp"
#include "stage.hpp"

#include "spin_component.hpp"
#include "target_component.hpp"
#include "follow_component.hpp"
#include "billboard_behavior.hpp"
#include "rave_component.hpp"

#include "debug/debug_camera.hpp"
#include "debug/debug_move.hpp"

#include <math/math.hpp>

#include <util/json.hpp>
#include <util/io.hpp>
#include <util/log.hpp>
#include <util/variant.hpp>

#include <graphics/canvas.hpp>
#include <graphics/shader.hpp>
#include <graphics/world_render_state.hpp>

#include <engine/collision.hpp>
#include <engine/resource_manager.hpp>
#include <engine/free_look.hpp>
#include <engine/relationship.hpp>
#include <engine/transform.hpp>
#include <engine/type_component.hpp>

//#include <engine/name_component.hpp>
#include <engine/components.hpp>
#include <app/input/input.hpp>

#include <algorithm>
#include <fstream>
//#include <filesystem>

#include <bullet/btBulletCollisionCommon.h>

// Debugging related:
#include <iostream>

namespace engine
{
	World::World(Config& config, ResourceManager& resource_manager, UpdateRate update_rate)
		: config(config), resource_manager(resource_manager), delta_time(update_rate)
	{
		register_event<app::input::MouseState,    &World::on_mouse_input>();
		register_event<app::input::KeyboardState, &World::on_keyboard_input>();
		register_event<OnComponentAdd<engine::CollisionComponent>, &World::on_new_collider>();
		register_event<OnTransformChange, &World::on_transform_change>();
		register_event<OnEntityDestroyed, &World::on_entity_destroyed>();

		root = create_pivot(*this);
	}
	
	/*
	void update_world_matrices(Registry& registry, LocalTransform& local_transform, TransformComponent& world_transform, Entity parent=null)
	{
		if (parent != null)
		{
			auto& parent_relationship    = registry.get<Relationship>(parent);
			auto& parent_local_transform = registry.get<LocalTransform>(parent);
			auto& parent_world_transform = registry.get<TransformComponent>(parent);

			update_world_matrices(registry, parent_local_transform, parent_world_transform, parent_relationship.get_parent());
		}
	}
	*/

	World::World(Config& config, ResourceManager& resource_manager, UpdateRate update_rate, const filesystem::path& path)
		: World(config, resource_manager, update_rate)
	{
		load(path);
	}

	Entity World::load(const filesystem::path& root_path, bool override_current, const std::string& json_file)
	{
		auto con = util::log::get_console();

		bool is_child_stage = (!override_current) && (stage != null);
		auto parent = root;

		if (is_child_stage)
		{
			parent = stage;
		}

		auto map_data_path = (root_path / json_file).string();
		
		con->info("Loading map from \"{}\"...", map_data_path);

		con->info("Parsing JSON...");

		std::ifstream map_data_stream(map_data_path);

		try
		{
			util::json map_data = util::json::parse(map_data_stream);

			auto map = Stage::Load(*this, parent, con, root_path, map_data);

			if (!is_child_stage)
			{
				stage = map;
			}

			return map;
		}
		catch (std::exception& e)
		{
			con->error("Error parsing JSON file: {}", e.what());
			ASSERT(false);
		}

		return null;
	}

	void World::update(app::Milliseconds time)
	{
		// Update the delta-timer.
		delta_time << time;

		event_handler.update();

		// Update systems:
		physics.update(*this, delta_time);

		SpinBehavior::update(*this);
		TargetComponent::update(*this);
		SimpleFollowComponent::update(*this);
		BillboardBehavior::update(*this);
		RaveComponent::update(*this);

		handle_transform_events(delta_time);
	}

	void World::handle_transform_events(float delta)
	{
		// Another pass is required, in order to ensure all of the hierarchy has a chance to validate collision representation:
		registry.view<TransformComponent>().each([this](auto entity, auto& tf)
		{
			tf.on_flag(TransformComponent::Flag::EventFlag, [this, entity]()
			{
				this->queue_event<OnTransformChange>(entity);
			});

			//tf.validate(TransformComponent::Dirty::EventFlag);
		});

		/*
		registry.view<TransformComponent, Relationship>().each([&](auto entity, auto& tf, auto& rel)
		{
			auto transform = Transform(registry, entity, rel, tf);

			//transform.validate_collision();
			transform.validate_collision_shallow();
		});
		*/
	}

	bool World::render
	(
		graphics::Canvas& canvas,
		const graphics::Viewport& viewport,
		Entity camera,
		bool multi_pass,
		bool use_active_shader,
		WorldRenderState* render_state,
		graphics::CanvasDrawMode additional_draw_modes,

		bool _combine_view_proj_matrices
	)
	{
		// Deferred referning is current unsupported.
		//ASSERT(forward_rendering);

		if (camera == null)
		{
			return false;
		}

		auto& registry = get_registry();

		//auto& camera_transform = registry.get<engine::TransformComponent>(camera);
		//auto inverse_world = camera_transform.inverse_world;

		auto camera_transform = get_transform(camera);
		auto& camera_params = registry.get<engine::CameraParameters>(camera);

		math::Matrix camera_matrix;

		if (camera_params.free_rotation)
		{
			camera_matrix = camera_transform.get_inverse_matrix();
		}
		else
		{
			camera_matrix = camera_transform.get_camera_matrix();
		}

		auto camera_position = camera_transform.get_position();

		// TODO: Move this aspect-ratio update to an event triggered on window-resize.
		///camera_params.aspect_ratio = window->horizontal_aspect_ratio();

		math::Matrix projection;

		switch (camera_params.projection_mode)
		{
			case CameraProjection::Orthographic:
			{
				float width = static_cast<float>(viewport.get_width());
				float height = static_cast<float>(viewport.get_height());

				float hw = (width / 2.0f);
				float hh = (height / 2.0f);

				//projection = glm::ortho(-hw, hw, hh, -hh, camera_params.near_plane, camera_params.far_plane);
				projection = glm::ortho(-hw, hw, -hh, hh, camera_params.near_plane, camera_params.far_plane);
				//camera_matrix = glm::inverse(camera_transform.get_matrix());
				//camera_matrix = glm::inverse(camera_matrix);

				break;
			}
			default: // case CameraProjection::Perspective:
				projection = glm::perspective(camera_params.fov, camera_params.aspect_ratio, camera_params.near_plane, camera_params.far_plane);

				break;
		}

		if (multi_pass)
		{
			draw_models((graphics::CanvasDrawMode::Opaque | additional_draw_modes), canvas, &projection, &camera_matrix, &camera_position, &properties.ambient_light, use_active_shader, render_state, _combine_view_proj_matrices);
			draw_models((graphics::CanvasDrawMode::Transparent | additional_draw_modes), canvas, &projection, &camera_matrix, &camera_position, &properties.ambient_light, use_active_shader, render_state, _combine_view_proj_matrices);
		}
		else
		{
			draw_models((graphics::Canvas::DrawMode::All | additional_draw_modes), canvas, &projection, &camera_matrix, &camera_position, &properties.ambient_light, use_active_shader, render_state, _combine_view_proj_matrices);
		}

		return true;
	}

	bool World::render_point_shadows
	(
		graphics::Canvas& canvas,
		graphics::Shader& shader,

		Entity camera,

		graphics::TextureArrayRaw* shadow_maps_out,
		graphics::VectorArray* light_positions_out,
		graphics::FloatArray* shadow_far_planes_out
	)
	{
		auto& ctx = canvas.get_context();

		ctx.use(shader, [&, this]()
		{
			registry.view<LightComponent, PointLightShadows>().each([&](auto entity, LightComponent& light_component, PointLightShadows& shadows)
			{
				auto viewport = shadows.shadow_map.get_viewport();
				auto& framebuffer = *shadows.shadow_map.get_framebuffer();

				auto light_tform = get_transform(entity);
				auto light_position = light_tform.get_position();

				ctx.set_viewport(viewport);

				ctx.use(framebuffer, [&, this]()
				{
					ctx.clear(graphics::BufferType::Depth);

					const auto& tforms = shadows.transforms;

					for (std::size_t i = 0; i < tforms.size(); ++i) // auto
						shader["cube_matrices[" + std::to_string(i) + "]"] = tforms.transforms[i];

					auto far_plane = shadows.shadow_perspective.far_plane;

					shader["far_plane"] = far_plane;
					shader["light_position"] = light_position;

					draw_models
					(
						(graphics::Canvas::DrawMode::Shadow), // graphics::Canvas::DrawMode::Opaque
						canvas, {}, {}, {}, {},
						true
					);

					if (shadow_maps_out)
					{
						shadow_maps_out->push_back(shadows.shadow_map.get_depth_map().get());
					}

					if (light_positions_out)
					{
						light_positions_out->push_back(light_position);
					}

					if (shadow_far_planes_out)
					{
						shadow_far_planes_out->push_back(far_plane);
					}
				});
			});
		});

		return true;
	}

	bool World::render_directional_shadows
	(
		graphics::Canvas& canvas,
		graphics::Shader& shader,
		
		Entity camera,
		
		graphics::TextureArrayRaw* shadow_maps_out,
		graphics::VectorArray* light_positions_out,
		graphics::MatrixArray* light_matrices_out
	)
	{
		auto& ctx = canvas.get_context();

		ctx.use(shader, [&, this]()
		{
			registry.view<LightComponent, DirectionLightShadows>().each([&](auto entity, LightComponent& light_component, DirectionLightShadows& shadows)
			{
				auto viewport = shadows.shadow_map.get_viewport();
				auto& framebuffer = *(shadows.shadow_map.get_framebuffer());
				const auto& depth_texture = *(shadows.shadow_map.get_depth_map());

				auto light_tform = get_transform(entity);
				auto light_position = light_tform.get_position();

				//auto light_space_matrix = glm::ortho(-2048.0f, 2048.0f, -2048.0f, 2048.0f, 1.0f, 4000.0f) * glm::lookAt(light_position, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));

				const auto& light_space_matrix = shadows.transforms;
				
				/*
				auto camera_transform = get_transform(camera);

				const auto& light_space_matrix
					= camera_transform.get_camera_matrix();
					//= camera_transform.get_inverse_matrix();
				*/

				//std::cout << light_position << std::endl;

				shader["light_space_matrix"] = light_space_matrix;

				ctx.set_viewport(viewport);

				ctx.use(framebuffer, [&, this]()
				{
					ctx.clear(graphics::BufferType::Depth);

					//ctx.use(depth_texture, [&, this]
					{
						/*
						auto far_plane = shadows.shadow_perspective.far_plane;

						shader["far_plane"] = far_plane;

						shader["light_position"] = light_position;
						*/

						draw_models
						(
							(graphics::Canvas::DrawMode::Shadow), // graphics::Canvas::DrawMode::Opaque
							canvas, {}, {}, {}, {},
							true
						);
					}//);
				});

				if (shadow_maps_out)
				{
					shadow_maps_out->push_back(&depth_texture);
				}

				if (light_positions_out)
				{
					light_positions_out->push_back(light_position);
				}

				if (light_matrices_out)
				{
					light_matrices_out->push_back(light_space_matrix);
				}
			});
		});

		return true;
	}

	void World::draw_models
	(
		graphics::CanvasDrawMode draw_mode,
		graphics::Canvas& canvas,
		// graphics::Shader& shader,
		
		const math::Matrix* projection_matrix,
		const math::Matrix* view_matrix,
		const math::Vector* camera_position,
		const graphics::ColorRGB* ambient_light,
		
		bool use_active_shader,
		
		WorldRenderState* render_state,

		bool combine_matrices
	)
	{
		auto draw = [&, this](auto& shader)
		{
			if ((combine_matrices) && ((projection_matrix) && (view_matrix))) // ((projection_matrix != nullptr) && (view_matrix != nullptr))
			{
				shader["view_projection"] = ((*projection_matrix) * (*view_matrix));
			}
			else
			{
				if (projection_matrix)
				{
					shader["projection"] = *projection_matrix;
				}

				if (view_matrix)
				{
					shader["view"] = *view_matrix;
				}
			}

			if (camera_position)
			{
				shader["view_position"] = *camera_position;
			}

			if (ambient_light)
			{
				shader["ambient_light"] = *ambient_light;
			}

			if (render_state)
			{
				// Point shadows:
				const auto& point_shadow_lp = render_state->point_shadows.light_positions;

				if (point_shadow_lp.has_value())
				{
					const auto& light_pos_v = *point_shadow_lp;

					// TODO: Implement as visit:
					if (util::peek_value<graphics::Vector*>(light_pos_v, [&](const graphics::Vector* vec)
					{
						ASSERT(vec);
					
						shader["point_shadow_light_position"] = *vec;
					}))
					{}
					else if (util::peek_value<graphics::VectorArray*>(light_pos_v, [&](const graphics::VectorArray* vec)
					{
						ASSERT(vec);

						shader["point_shadow_light_position"] = *vec;
					}))
					{}
				}

				const auto& point_shadow_fp = render_state->point_shadows.far_planes;

				if (point_shadow_fp.has_value())
				{
					const auto& far_v = *point_shadow_fp;

					// TODO: Implement as visit:
					if (util::peek_value<float*>(far_v, [&](const float* far_plane)
					{
						ASSERT(far_plane);

						shader["point_shadow_far_plane"] = *far_plane;
					}))
					{}
					else if (util::peek_value<graphics::FloatArray*>(far_v, [&](const graphics::FloatArray* far_plane)
					{
						ASSERT(far_plane);

						shader["point_shadow_far_plane"] = *far_plane;
					}))
					{}
				}

				// Directional shadows:
				const auto& directional_shadow_lp = render_state->directional_shadows.light_positions;

				if (directional_shadow_lp.has_value())
				{
					const auto& light_pos_v = *directional_shadow_lp;

					// TODO: Implement as visit:
					if (util::peek_value<graphics::Vector*>(light_pos_v, [&](const graphics::Vector* vec)
					{
						ASSERT(vec);
					
						shader["directional_shadow_light_position"] = *vec;
					}))
					{}
					else if (util::peek_value<graphics::VectorArray*>(light_pos_v, [&](const graphics::VectorArray* vec)
					{
						ASSERT(vec);

						shader["directional_shadow_light_position"] = *vec;
					}))
					{}
				}

				const auto& directional_shadow_mat = render_state->directional_shadows.light_matrices;

				if (directional_shadow_mat.has_value())
				{
					const auto& mat_v = *directional_shadow_mat;

					// TODO: Implement as visit:
					if (util::peek_value<graphics::Matrix*>(mat_v, [&](const graphics::Matrix* matrix)
					{
						ASSERT(matrix);

						shader["directional_light_space_matrix"] = *matrix;
					}))
					{}
					else if (util::peek_value<graphics::MatrixArray*>(mat_v, [&](const graphics::MatrixArray* matrices)
					{
						ASSERT(matrices);

						shader["directional_light_space_matrix"] = *matrices;
					}))
					{}
				}
			}

			bool _auto_clear_textures = false; // true;

			registry.view<ModelComponent, TransformComponent, Relationship>().each([&](auto entity, auto& model_component, auto& transform, const auto& relationship)
			{
				if (!model_component.visible)
				{
					return;
				}

				if ((draw_mode & graphics::CanvasDrawMode::Shadow) && (!model_component.casts_shadow))
				{
					return;
				}

				/*
				if (model_component.transparent())
				{
					return;
				}
				*/

				if (model_component.model == nullptr)
				{
					return;
				}

				auto model_transform = Transform(registry, entity, relationship, transform);
				auto model_matrix = model_transform.get_matrix(true); // get_local_matrix();
				//auto model_matrix = model_transform.get_local_matrix();

				shader["model"] = model_matrix;

				auto& model = *model_component.model;
				auto color  = model_component.color;

				auto model_draw_mode = draw_mode;

				if (!model_component.receives_shadow)
				{
					model_draw_mode |= (graphics::CanvasDrawMode::IgnoreShadows);
				}

				canvas.draw(model, color, draw_mode, _auto_clear_textures, ((render_state) ? render_state->dynamic_textures : nullptr));
			});
		};

		if ((use_active_shader) || (draw_mode & graphics::CanvasDrawMode::IgnoreShaders))
		{
			auto& shader = canvas.get_shader();

			draw(shader);
		}
		else
		{
			auto& shaders = resource_manager.loaded_shaders;

			for (auto& shader_entry : shaders)
			{
				auto& shader_ptr = shader_entry.second;

				if (!shader_ptr)
				{
					continue;
				}

				auto& ctx = canvas.get_context();
				auto& shader = *shader_ptr;

				ctx.use(shader, [&, this]()
				{
					draw(shader);
				});
			}
		}
	}

	Transform World::apply_transform(Entity entity, const math::TransformVectors& tform)
	{
		auto transform = get_transform(entity);

		transform.apply(tform);

		return transform;
	}

	Transform World::set_position(Entity entity, const math::Vector& position)
	{
		auto transform = get_transform(entity);

		transform.set_position(position);

		return transform;
	}

	Transform World::get_transform(Entity entity)
	{
		return Transform(registry, entity);
	}

	math::Vector World::get_up_vector(math::Vector up) const
	{
		auto& m = registry.get<TransformComponent>(root);

		return (m._w * math::Vector4D{ up, 1.0f });
	}

	Entity World::get_parent(Entity entity) const
	{
		auto* relationship = registry.try_get<Relationship>(entity);

		if (relationship == nullptr)
		{
			return null;
		}

		return relationship->get_parent();
	}

	void World::set_parent(Entity entity, Entity parent, bool _null_as_root)
	{
		/*
		if (_null_as_root)
		{
			if ((parent == null) && (entity != root))
			{
				parent = root;
			}
		}
		*/

		Relationship::set_parent(registry, entity, parent);
	}

	Entity World::get_by_name(std::string_view name)
	{
		auto view = registry.view<NameComponent>();

		for (auto it = view.begin(); it != view.end(); it++)
		{
			Entity entity = *it;

			auto name_comp = registry.get<NameComponent>(entity);

			if (name_comp.name == name)
			{
				return entity;
			}
		}

		return null;
	}

	Entity World::get_player(PlayerIndex player) const
	{
		auto view = registry.view<PlayerState>();

		for (auto it = view.begin(); it != view.end(); it++)
		{
			Entity entity = *it;

			const auto& player_state = registry.get<PlayerState>(entity);

			if (player_state.index == player)
			{
				return entity;
			}
		}

		return null;
	}

	void World::add_camera(Entity camera, bool make_active)
	{
		ASSERT(registry.has<CameraParameters>(camera));

		if ((this->camera == null) || make_active)
		{
			this->camera = camera;
		}

		cameras.push_back(camera);
	}

	void World::remove_camera(Entity camera)
	{
		ASSERT(registry.has<CameraParameters>(camera));

		auto it = std::find(cameras.begin(), cameras.end(), camera);

		if (it != cameras.end())
		{
			cameras.erase(it);
		}

		if (cameras.empty())
		{
			this->camera = null;
		}
		else
		{
			this->camera = cameras[0];
		}
	}

	void World::on_mouse_input(const app::input::MouseState& mouse)
	{
		FreeLook::update(*this, mouse);
	}

	void World::on_keyboard_input(const app::input::KeyboardState& keyboard)
	{
		engine::debug::DebugMove::update(*this, keyboard);
	}

	void World::on_new_collider(const OnComponentAdd<CollisionComponent>& new_col)
	{
		physics.on_new_collider(*this, new_col);
	}

	void World::on_transform_change(const OnTransformChange& tform_change)
	{
		auto con = util::log::get_console();
		auto entity = tform_change.entity;

		// Update shadow-maps for light entities.
		update_shadows(*this, entity); // *point_shadows

		// Debugging related:
		/*
		auto* type_component = registry.try_get<TypeComponent>(entity);
		auto entity_type = ((type_component) ? type_component->type : EntityType::Default);

		std::string name_label;

		auto* name_component = registry.try_get<NameComponent>(entity);

		if (name_component)
		{
			name_label = " \"" + name_component->name +"\"";
		}

		con->info("Entity #{}{} ({}) - Transform Changed: {}", entity, name_label, entity_type, get_transform(entity).get_vectors());
		*/
	}

	void World::on_entity_destroyed(const OnEntityDestroyed& destruct)
	{
		auto entity          = destruct.entity;
		auto parent          = destruct.parent;
		auto type            = destruct.type;
		auto destroy_orphans = destruct.destroy_orphans;

		// Handle collision:
		auto* col = registry.try_get<CollisionComponent>(entity);

		if (col)
		{
			physics.on_destroy_collider(*this, entity, *col);
		}

		// Handle entity relationships:
		auto* relationship = registry.try_get<Relationship>(entity);

		if (relationship) // <-- Null-check for 'relationship' isn't actually necessary. (MSVC complains)
		{
			// Make the assumption this object exists, as if it didn't, we would be in an invalid state.
			auto* parent_relationship = registry.try_get<Relationship>(parent);

			relationship->enumerate_child_entities(registry, [&](Entity child, Entity next_child) -> bool
			{
				if (parent_relationship)
				{
					parent_relationship->add_child(registry, parent, child);
				}
				else
				{
					set_parent(child, null);
				}

				if (destroy_orphans)
				{
					// Queue orphans to be destroyed on next event cycle.
					destory_entity(*this, child, true);
				}

				return true;
			});

			//registry.replace<Relationship>(parent, std::move(parent_relationship)); // [&](auto& r) { r = parent_relationship; }
		}

		registry.destroy(entity);
	}
}