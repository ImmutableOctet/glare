#pragma once

#include <engine/collision.hpp>
#include <graphics/model.hpp>
#include <optional>

// Forward declarations:

// Bullet:
//class btCollisionShape;
class btCapsuleShape;
class btBoxShape;

namespace engine
{
	using CollisionRaw = CollisionComponent::RawShape; // btCollisionShape;
	using CollisionShape = CollisionComponent::Shape; // ref<CollisionRaw>;
	using CollisionGeometry = graphics::Model::CollisionGeometry;

	struct CollisionData
	{
		using Shape = CollisionShape;
		using Raw = CollisionRaw;

		using Geometry = CollisionGeometry;

		static CollisionShape build_mesh_shape(const CollisionGeometry& geometry_storage, bool optimize=true);

		CollisionData() = default;

		CollisionData(CollisionData&&) = default;
		CollisionData(const CollisionData&) = default;

		CollisionData(const Shape& collision_shape);
		CollisionData(Geometry&& geometry_storage, bool optimize = true);

		CollisionData& operator=(CollisionData&&) = default;

		Shape collision_shape;
		std::optional<Geometry> geometry_storage = std::nullopt;

		inline bool has_shape() const { return collision_shape.operator bool(); }
		inline bool has_geometry() const { return geometry_storage.has_value(); }

		inline explicit operator bool() const { return has_shape(); }
		inline bool operator==(const CollisionData& data) const { return (this->collision_shape == data.collision_shape); }
	};
}