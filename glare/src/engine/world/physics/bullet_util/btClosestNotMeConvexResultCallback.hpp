#pragma once

#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>

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

// Based on source code in `btDiscreteDynamicsWorld.cpp` file in Bullet's source code:
namespace engine
{
	class btClosestNotMeConvexResultCallback : public btCollisionWorld::ClosestConvexResultCallback
	{
		public:
			btCollisionObject* m_me;
			btScalar m_allowedPenetration;
			btOverlappingPairCache* m_pairCache;
			btDispatcher* m_dispatcher;

		public:
			btClosestNotMeConvexResultCallback(btCollisionObject* me, const btVector3& fromA, const btVector3& toA, btOverlappingPairCache* pairCache, btDispatcher* dispatcher)
				: btCollisionWorld::ClosestConvexResultCallback(fromA, toA),
					m_me(me),
					m_allowedPenetration(0.0f),
					m_pairCache(pairCache),
					m_dispatcher(dispatcher)
			{}

			virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace)
			{
				if (convexResult.m_hitCollisionObject == m_me)
					return 1.0f;

				//ignore result if there is no contact response
				if (!convexResult.m_hitCollisionObject->hasContactResponse())
					return 1.0f;

				btVector3 linVelA, linVelB;
				linVelA = m_convexToWorld - m_convexFromWorld;
				linVelB = btVector3(0, 0, 0);  //toB.getOrigin()-fromB.getOrigin();

				btVector3 relativeVelocity = (linVelA - linVelB);
				//don't report time of impact for motion away from the contact normal (or causes minor penetration)
				if (convexResult.m_hitNormalLocal.dot(relativeVelocity) >= -m_allowedPenetration)
					return 1.f;

				return ClosestConvexResultCallback::addSingleResult(convexResult, normalInWorldSpace);
			}

			virtual bool needsCollision(btBroadphaseProxy* proxy0) const
			{
				//don't collide with itself
				if (proxy0->m_clientObject == m_me)
					return false;

				///don't do CCD when the collision filters are not matching
				if (!ClosestConvexResultCallback::needsCollision(proxy0))
					return false;
				if (m_pairCache->getOverlapFilterCallback()) {
					btBroadphaseProxy* proxy1 = m_me->getBroadphaseHandle();
					bool collides = m_pairCache->needsBroadphaseCollision(proxy0, proxy1);
					if (!collides)
					{
						return false;
					}
				}

				btCollisionObject* otherObj = (btCollisionObject*)proxy0->m_clientObject;

				if (!m_dispatcher->needsCollision(m_me, otherObj))
					return false;

				//call needsResponse, see http://code.google.com/p/bullet/issues/detail?id=179
				if (m_dispatcher->needsResponse(m_me, otherObj))
				{
		#if 0
					///don't do CCD when there are already contact points (touching contact/penetration)
					btAlignedObjectArray<btPersistentManifold*> manifoldArray;
					btBroadphasePair* collisionPair = m_pairCache->findPair(m_me->getBroadphaseHandle(),proxy0);
					if (collisionPair)
					{
						if (collisionPair->m_algorithm)
						{
							manifoldArray.resize(0);
							collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);
							for (int j=0;j<manifoldArray.size();j++)
							{
								btPersistentManifold* manifold = manifoldArray[j];
								if (manifold->getNumContacts()>0)
									return false;
							}
						}
					}
		#endif
					return true;
				}

				return false;
			}
	};
}