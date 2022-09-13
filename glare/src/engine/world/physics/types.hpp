#pragma once

#include <engine/types.hpp>
//#include <engine/world/types.hpp>

//#include <graphics/collision_geometry.hpp>

#include <math/math.hpp>

#include "collision_group.hpp"
#include "collision_body_type.hpp"
#include "motion_flags.hpp"

//#include <bullet/btBulletCollisionCommon.h>
//#include <bullet/btBulletDynamicsCommon.h>

#include <memory>

class btCollisionShape;
class btConcaveShape;
class btConvexShape;

//class btCapsuleShape;
//class btBoxShape;

class btCollisionObject;
class btRigidBody;

class btVector3;
class btTransform;

namespace graphics
{
	struct CollisionGeometry;
}

namespace engine
{
	/*
	enum class BasicCollisionShape
	{
		Box = 1,
		Capsule = 2,
		//Sphere = 3,
	};
	*/

	// TODO: Simplify some of these typedefs.

	// TODO: Rename this to `CollisionRawShape`.
	using CollisionRaw             = btCollisionShape;
	using ConvexCollisionShapeRaw  = btConvexShape;
	using ConcaveCollisionShapeRaw = btConcaveShape;

	using CollisionShape           = std::shared_ptr<CollisionRaw>;
	using ConvexCollisionShape     = std::shared_ptr<ConvexCollisionShapeRaw>;
	using ConcaveCollisionShape    = std::shared_ptr<ConcaveCollisionShapeRaw>;

	// Defined in `resource_manager` module.
	using CollisionGeometry = graphics::CollisionGeometry;
}