#pragma once

#include <engine/world/physics/types.hpp>

#include <graphics/model.hpp>
#include <graphics/collision_geometry.hpp>

#include <optional>

namespace engine
{
	// Internal storage mechanism for 'collision shapes' and associated mesh-data (if any).
	// This is different from a 'collision object' instance (sometimes referred to as a 'collider'), which can be attached to an `Entity` in a Game World.
	// Such 'colliders' are created by attaching a `CollisionComponent` object to an `Entity`, using these `CollisionData` objects as their underlying storage. (see constructors for `CollisionComponent`)
	struct CollisionData
	{
		using Shape    = CollisionShape;    // std::shared_ptr<btCollisionShape>;
		using Raw      = CollisionRaw;      // btCollisionShape;
		using Geometry = CollisionGeometry; // graphics::Model::CollisionGeometry;

		static CollisionShape build_mesh_shape(const Geometry& geometry_storage, bool optimize=true);

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