#pragma once

/*
	BULLET LICENSE INFORMATION:

	Bullet Continuous Collision Detection and Physics Library
	http://bulletphysics.org

	This software is provided 'as-is', without any express or implied warranty.
	In no event will the authors be held liable for any damages arising from the use of this software.
	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it freely,
	subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
	2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
	3. This notice may not be removed or altered from any source distribution.
*/

#include <bullet/btBulletCollisionCommon.h>
//#include <bullet/BulletCollision/CollisionDispatch/btCollisionWorld.h>

// Based code found in `btKinematicCharacterController.cpp` from Bullet's source code:
namespace engine
{
	class btKinematicClosestNotMeRayResultCallback : public btCollisionWorld::ClosestRayResultCallback
	{
		public:
			btKinematicClosestNotMeRayResultCallback(btCollisionObject* me)
				: btCollisionWorld::ClosestRayResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0))
			{
				m_me = me;
			}

			virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
			{
				if (rayResult.m_collisionObject == m_me)
				{
					return 1.0;
				}

				return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
			}

		protected:
			btCollisionObject* m_me;
	};
}