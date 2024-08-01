#pragma once

// TODO: Move to a more appropriate submodule. (i.e. anywhere else but `graphics`)

#include "vertex.hpp"

#include "mesh.hpp"
//#include "drivers/drivers.hpp"

#include <bullet/LinearMath/btIDebugDraw.h>

//#include <vector>

namespace graphics
{
	class Context;

	class BulletDebugDrawer : public btIDebugDraw
	{
		public:
			using VertexType = SimpleColoredVertex;
			using Vertices = MeshData<VertexType>; // std::vector<VertexType>;

			BulletDebugDrawer() = default;
			BulletDebugDrawer(BulletDebugDrawer&&) noexcept = default;

			virtual ~BulletDebugDrawer();

			virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;

			virtual void drawContactPoint(const btVector3&, const btVector3&, btScalar, int, const btVector3&) override {}
			virtual void reportErrorWarning(const char*) override {}
			virtual void draw3dText(const btVector3&, const char*) override {}

			virtual void setDebugMode(int mode) override { draw_mode = mode; }
			virtual int getDebugMode() const { return draw_mode; }

			virtual void clearLines() override;
			virtual void flushLines() override;

			void flush_gpu_state();

			inline Mesh& get_gpu_state() { return gpu_state; }

			inline bool has_points() const { return static_cast<bool>(point_data); }

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
		protected:
			void clear_line_data();

			//MeshComposition gpu_state;

			// TODO: Look into optimization via two or more gpu states. ('double buffering')
			Mesh gpu_state;

			Vertices point_data;
			std::shared_ptr<Context> ctx;

			std::size_t max_buffer_size  = 0;
			std::size_t prev_buffer_size = 0;

			int draw_mode = (DBG_DrawWireframe | DBG_DrawAabb);

			bool enabled              : 1 = true;
	};
}