#pragma once

#include <string_view>

#include <app/graphics_application.hpp>

#include "graphics.hpp"

#include <util/memory.hpp>

#include <graphics/types.hpp>

#include <engine/config.hpp>
#include <engine/resource_manager/resource_manager.hpp>

#include <engine/world/world.hpp>
#include <engine/world/behaviors/meta.hpp>

#include <engine/world/render/world_renderer.hpp>

#include <util/shorthand.hpp>

namespace graphics
{
	class GBuffer;
}

namespace engine
{
	struct WorldRenderState;
}

namespace game
{
	using RenderState = engine::WorldRenderState;
	using RenderBuffer = graphics::GBuffer;

	class Game : public app::GraphicsApplication
	{
		public:
			// TODO: Look into whether we should replace `opaque_unique_ptr` with `std::any`.
			// (Has storage and copy/move assignment implications)
			// Represents an automatically managed unique allocation of any type.
			using System = memory::opaque_unique_ptr;
		protected:
			engine::Config cfg;

			Screen screen;
			Effects effects;

			engine::ResourceManager resource_manager;
			
			engine::World world;

			engine::WorldRenderer renderer;

			// Opaque vector of smart pointers to systems and their resources.
			std::vector<System> systems;
		public:
			/*
				Adds `system` to an internal list of active `System` objects,
				then returns a (void) pointer to the underlying opaque object.
				
				If the underlying object-type of `system` is known, then it is safe
				to `reinterpret_cast` this to the appropriate pointer-type.
			*/
			inline auto* add_system(System&& system)
			{
				auto* system_ptr = system.get();

				systems.push_back(std::move(system));

				return system_ptr;
			}

			/*
				Constructs a `SystemType` object using the arguments provided, then adds the
				newly allocated object to the `systems` collection to ensure its lifetime.
				
				This function returns a reference to the allocated `SystemType` object.
				The lifetime of this object is managed internally.
			*/
			template <typename SystemType, typename...Args>
			inline SystemType& emplace_system(Args&&... args)
			{
				auto ptr = add_system(memory::make_opaque<SystemType>(std::forward<Args>(args)...));

				assert(ptr);

				return *reinterpret_cast<SystemType*>(ptr);
			}

			// Alias for `emplace_system`; allocates a system internally and returns a reference to it.
			template <typename SystemType, typename...Args>
			inline SystemType& system(Args&&... args) { return emplace_system<SystemType>(std::forward<Args>(args)...); }
			
			// TODO: Move this implementation to a different file/class.
			template <typename BehaviorType>
			inline void behavior()
			{
				// Check for all possible behavior event-handlers:
				if constexpr (engine::HAS_STATIC_MEMBER_FUNCTION(BehaviorType, on_update))
				{
					world.register_free_function<engine::OnServiceUpdate, engine::behavior_impl::bridge_on_update<BehaviorType>>();
				}
			}

			Game
			(
				std::string_view title,
				
				int width=1600, int height=900,
				UpdateRate update_rate=DEFAULT_FRAMERATE,

				bool vsync=true, bool lock_mouse=true,

				app::WindowFlags window_flags=(app::WindowFlags::OpenGL|app::WindowFlags::Resizable),
				bool imgui_enabled=true,

				unique_ref<engine::RenderPipeline>&& rendering_pipeline=nullptr
			);

			inline engine::World& get_world() { return world; }

			// Proxy for underlying `Window` object's `set_title` member-function.
			// `title` must be zero-terminated.
			void set_title(std::string_view title);
			
			void render() override;
			void update(app::Milliseconds time) override;

			// Implement Game-specific logic using these:
			virtual void on_update(float delta) abstract;
			virtual void on_render(RenderState& render_state) {}
			virtual void on_resize(int width, int height) {}
		protected:
			virtual void on_window_resize(app::Window& window, int width, int height) override;

			RenderState& initialize_render_state(RenderState& render_state);
	};
}