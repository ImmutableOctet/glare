#pragma once

#include <math/math.hpp>

namespace engine
{
	class Entity
	{
		protected:
			// TODO: Look into making this a weak pointer, rather than a raw pointer.
			Entity* parent = nullptr;

			math::mat4 transformation = math::mat4(1.0);
		public:
			Entity* get_parent();
		protected:
			Entity(Entity* parent=nullptr);
	};

	/*
	class Actor : public Entity
	{

	};
	*/
}