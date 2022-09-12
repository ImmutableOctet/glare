#pragma once

#include "render_phase.hpp"
#include "types.hpp"

#include <util/memory.hpp>

#include <graphics/bullet_debug.hpp>

//#include <graphics/types.hpp>
#include <engine/types.hpp>

namespace engine
{
	// Handles deferred shading/lighting phase, using previous phases' gbuffer data and render-state context as input.
	class BulletDebugRenderPhase : public RenderPhase
	{
		protected:
			std::shared_ptr<graphics::Shader> debug_lines_shading;

			// Handled as a `std::unique_ptr` due to Bullet's address stability guarantees.
			std::unique_ptr<graphics::BulletDebugDrawer> debug_drawer;

			bool enabled = true;
		public:
			BulletDebugRenderPhase(const std::shared_ptr<graphics::Shader>& debug_lines_shading);
			BulletDebugRenderPhase(const std::shared_ptr<graphics::Context>& ctx, std::string_view shader_preprocessor);

			BulletDebugRenderPhase(BulletDebugRenderPhase&&) noexcept = default;

			const RenderParameters& operator()(const RenderParameters& parameters);

			inline graphics::BulletDebugDrawer& get_debug_drawer() { return *debug_drawer; }

			inline void enable()
			{
				enabled = true;
			}

			inline void disable()
			{
				enabled = false;
			}

			inline bool is_enabled() const
			{
				return enabled;
			}
	};
}