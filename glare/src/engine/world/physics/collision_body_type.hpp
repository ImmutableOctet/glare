#pragma once

namespace engine
{
	enum class CollisionBodyType
	{
		Basic,     // Standard Bullet collision object.
		Static,    // Rigid body type without kinematics/motion-state.
		Kinematic, // Rigid body type with kinematics/motion-state.
		Ghost,     // Bullet ghost object.

		Dynamic = Kinematic,
	};
}