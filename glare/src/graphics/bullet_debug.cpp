#include "bullet_debug.hpp"

#include "context.hpp"

#include <bullet/btBulletCollisionCommon.h>
#include <math/bullet.hpp>

namespace graphics
{
	BulletDebugDrawer::~BulletDebugDrawer()
	{
		//ctx->release_mesh(std::move(gpu_state));
	}

	void BulletDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
	{
		SimpleColoredVertex from_v;
		SimpleColoredVertex to_v;

		auto color_native = math::to_vector(color);

		from_v.position = math::to_vector(from);
		from_v.color = color_native;

		to_v.position = math::to_vector(to);
		to_v.color    = color_native;

		auto& vertices_out = point_data.vertices;

		vertices_out.push_back(from_v);
		vertices_out.push_back(to_v);
	}

	void BulletDebugDrawer::clearLines()
	{
		point_data.vertices.clear();
	}

	void BulletDebugDrawer::flushLines()
	{
		// TODO: Optimize.
		gpu_state = Mesh::Generate<VertexType>(ctx, point_data, Primitive::Line, false);
	}
}