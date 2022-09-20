#include "glare.hpp"

// TODO: Reduce unnecessary includes.
#include <iostream>
#include <string>
#include <string_view>
#include <cmath>
#include <cstdlib>
#include <optional>
#include <cassert>
#include <format>

#include <app/events.hpp>
#include <app/input/keycodes.hpp>
#include <engine/engine.hpp>

#include <util/variant.hpp>
#include <util/log.hpp>

#include <engine/debug.hpp>
#include <engine/world/meta/meta.hpp>
#include <engine/world/debug/debug.hpp>

#include <graphics/context.hpp>
#include <graphics/native/opengl.hpp>
#include <engine/world/render/world_render_state.hpp>

#include <engine/events.hpp>
#include <engine/world/world_events.hpp>
#include <engine/type_component.hpp>
#include <engine/model_component.hpp>

#include <engine/world/graphics_entity.hpp>
#include <engine/world/light.hpp>

#include <engine/world/behaviors/behaviors.hpp>

#include <sdl2/SDL_video.h>
#include <sdl2/SDL_events.h>

#include <imgui/imgui.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <format>

// Debugging related:
#include <engine/world/physics/collision_component.hpp>
#include <engine/world/motion/motion_component.hpp>
#include <engine/types.hpp>
#include <engine/world/zones/zones.hpp>
#include <math/bullet.hpp>
#include <bullet/btBulletCollisionCommon.h>

namespace glare
{
	Glare::Glare():
		Game("Project Glare", 1600, 900, 60, true, true), // false
		dbg_listener(world)
	{
		using namespace graphics;

		// TODO: Look into this again.
		world.register_event<app::input::KeyboardState, &Glare::on_user_keyboard_input>(*this);
		world.register_event<app::input::MouseState, &Glare::on_user_mouse_input>(*this);

		// Normally, we lock when starting the game, but don't here for testing purposes.
		//input.get_mouse().lock();

		world.load("assets/maps/test01");
		//world.load("assets/maps/story/2.ice-world/ice-connector");
		//world.load("assets/maps/test01 - old");
		//world.load("assets/maps/room");
		//world.load("assets/maps/collision_test");

		// Debugging related:
		//GLint value = 0; glGetIntegerv(GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS, &value); // GL_MAX_VERTEX_BINDABLE_UNIFORMS_EXT

		auto& registry = world.get_registry();

		auto cube = engine::load_model
		(
			world, "assets/objects/cube/cube.b3d", engine::null,
			engine::EntityType::Object,
			true, false, 0.0f,
			engine::CollisionGroup::Object,
			engine::CollisionGroup::All,
			engine::CollisionGroup::All
		);

		world.set_name(cube, "Cube");

		attach_collision(world, cube, resource_manager.generate_sphere_collision(2.0f).collision_shape, engine::EntityType::Object);
		auto& cube_c = registry.get<engine::CollisionComponent>(cube);
		cube_c.set_mass(0.5f);

		world.transform_and_reset_collision(cube, [&](auto& cube_t)
		{
			cube_t.set_position({ -6.20467f, 166.5406f, 39.1254f });
		});

		auto cube2 = engine::load_model
		(
			world, "assets/objects/cube/cube.b3d", engine::null,
			engine::EntityType::Object,
			true, false, 0.0f,
			engine::CollisionGroup::Object,
			engine::CollisionGroup::All,
			engine::CollisionGroup::All
		);

		world.set_name(cube2, "Cube2");

		attach_collision(world, cube2, resource_manager.generate_sphere_collision(4.0f).collision_shape, engine::EntityType::Object);
		auto& cube2_c = registry.get<engine::CollisionComponent>(cube2);
		cube2_c.set_mass(2.0f);

		world.transform_and_reset_collision(cube2, [&](auto& cube2_t)
		{
			//cube2_t.set_scale(4.0f);
			cube2_t.set_position({ -6.20467f, 180.5406f, 39.1254f });
		});

		registry.emplace<engine::MotionComponent>(cube2);

		auto& camera_c = registry.get<engine::CollisionComponent>(world.get_camera());

		camera_c.set_mass(2.0f);

		auto zone = create_zone(world, math::AABB { { -144.822, 150.215, -141.586 }, { -74.4793, 71.374, -203.555 } });
		world.set_name(zone, "Zone");
	}

	engine::Transform Glare::get_named_transform(std::string_view name)
	{
		auto entity = world.get_by_name(name);
		auto transform = world.get_transform(entity);

		return transform;
	}

	void Glare::on_user_keyboard_input(const app::input::KeyboardState& keyboard)
	{
		//print("Keyboard input detected.");

		//std::string_view target_obj = engine::DEFAULT_PLAYER_NAME;

		//auto camera_t = world.get_transform(world.get_camera());
		//auto camera_pos = camera_t.get_position();

		//auto transform = world.get_transform(world.get_camera());
		//auto transform = get_named_transform(target_obj);
		//auto angle = glm::normalize(transform.get_position() - camera_pos);
		//float delta = world.delta();

		if (keyboard.get_key(SDL_SCANCODE_R))
		{
			auto box = world.get_by_name("Cube");

			auto t = world.get_transform(box);

			const auto& dt = world.get_delta_time();

			auto m = math::Vector { 0.0f, std::sinf((dt.current_frame_time() / 1000.0f)) * 1.0f * dt, 0.0f };

			t.move(m);
		}

		if (keyboard.get_key(SDL_SCANCODE_U))
		{
			auto box = world.get_by_name("Cube");

			auto t = world.get_transform(box);

			const auto& dt = world.get_delta_time();

			auto m = math::Vector{ 0.0f, 0.25f * dt, 0.0f };

			t.move(m);
		}

		if (keyboard.get_key(SDL_SCANCODE_J))
		{
			auto box = world.get_by_name("Cube");

			auto t = world.get_transform(box);

			const auto& dt = world.get_delta_time();

			auto m = math::Vector{ 0.0f, -0.25f * dt, 0.0f };

			t.move(m);
		}

		if (keyboard.get_key(SDL_SCANCODE_I))
		{
			auto box = world.get_by_name("Cube2");

			auto t = world.get_transform(box);

			const auto& dt = world.get_delta_time();

			auto m = math::Vector{ 0.0f, 0.25f * dt, 0.0f };

			t.move(m);
		}

		if (keyboard.get_key(SDL_SCANCODE_K))
		{
			auto box = world.get_by_name("Cube2");

			auto t = world.get_transform(box);

			const auto& dt = world.get_delta_time();

			//auto m = math::Vector{ 0.0f, -0.25f * dt, 0.0f };
			auto m = math::Vector{ 0.0f, -0.25f * dt, 0.0f };

			t.move(m);
		}
	}

	void Glare::on_user_mouse_input(const app::input::MouseState& mouse)
	{
		if (mouse.left)
		{
			print("Hello world");
		}
	}

	void Glare::on_update(float delta)
	{
		/*
		math::Vector direction = {};

		{
			auto t = world.get_transform(world.get_camera());
			direction = t.get_direction_vector();
		}

		print("Camera direction: {}", direction);
		*/

		engine::position_in_titlebar(*this, world.get_camera(), std::format("FPS: {} | ", graphics.framerate));
	}

	void Glare::on_render(RenderState& render_state)
	{
		render_debug_controls();
	}

	void Glare::render_debug_controls()
	{
		using namespace engine::meta;

		if (!imgui_enabled())
			return;

		meta_controls();

		auto player = world.get_player();
		assert(player != engine::null);

		auto player_model = world.get_child_by_name(player, "model", false);
		assert(player_model != engine::null);

		auto target = player_model;

		animation_control(world, target);

		hierarchy_control(world, world.get_root());
	}

	// Subroutine of `render_debug_controls`.
	void Glare::meta_controls()
	{
		ImGui::Begin("Meta", nullptr, ImGuiWindowFlags_NoBackground|ImGuiWindowFlags_NoTitleBar);

		if (ImGui::Button("Switch to Game"))
		{
			auto& mouse = input.get_mouse();
			mouse.toggle_lock();
		}

		auto& io = ImGui::GetIO();

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

		ImGui::End();
	}

	void Glare::on_keyup(const keyboard_event_t& event)
	{
		switch (event.keysym.sym)
		{
		case SDLK_ESCAPE:
			stop();

			break;
		case SDLK_TAB:
		{
			auto& mouse = input.get_mouse();

			mouse.toggle_lock();

			break;
		}
		case SDLK_SEMICOLON:
			graphics.context->set_flags(graphics::ContextFlags::Wireframe, !graphics.context->get_flag(graphics::ContextFlags::Wireframe));

			break;
		case SDLK_QUOTE:
			

			break;
		case SDLK_q:
		{
			//print("World: {}", world);
			//print_children(world, world.get_root());

			auto zone = world.get_by_name("Zone");
			const auto& collision = world.get_registry().get<engine::CollisionComponent>(zone);

			auto* obj = collision.get_collision_object();

			print("Collision object is located at: {}", math::to_vector(obj->getWorldTransform().getOrigin()));

			break;
		}

		case SDLK_f:
		{
			auto target_entity = world.get_camera(); // world.get_by_name("Moving Sphere"); // "Spinning Cube" // world.get_root()

			if (target_entity != engine::null)
			{
				auto t = world.get_transform(target_entity);

				auto position = t.get_position();
				auto rotation = math::degrees(t.get_rotation());
				auto scale = t.get_scale();

				std::cout << "\nCamera:\n";

				//std::cout << "Position: " << t.get_position() << '\n';
				//std::cout << "Rotation: " << math::degrees(t.get_rotation()) << '\n';
				//std::cout << "Scale: " << t.get_scale() << '\n';
				std::cout << "\"position\": { " << "\"x\": " << position.x << ", " << "\"y\": " << position.y << ", " << "\"z\": " << position.z << " }," << '\n';
				std::cout << "\"rotation\": { " << "\"x\": " << rotation.x << ", " << "\"y\": " << rotation.y << ", " << "\"z\": " << rotation.z << " }," << '\n';
				std::cout << "\"scale\": { " << "\"x\": " << scale.x << ", " << "\"y\": " << scale.y << ", " << "\"z\": " << scale.z << " }" << '\n';
			}

			//engine::SimpleFollowComponent::update(world);

			break;
		}

		case SDLK_h:
		{
			auto& registry = world.get_registry();

			auto light = world.get_by_name("shadow_test");
			//auto& shadows = registry.get<engine::PointLightShadows>(light);

			auto lt = world.get_transform(light);
			auto ct = world.get_transform(world.get_camera());

			lt.set_position(ct.get_local_position());
			lt.set_rotation(ct.get_rotation());

			break;
		}

		case SDLK_c:
			screen.display_mode = static_cast<GBufferDisplayMode>((static_cast<int>(screen.display_mode) + 1) % static_cast<int>(GBufferDisplayMode::Modes));

			break;
		}
	}

	void Glare::on_keydown(const keyboard_event_t& event)
	{
		/*
		switch (event.keysym.sym)
		{
		}
		*/
	}
}