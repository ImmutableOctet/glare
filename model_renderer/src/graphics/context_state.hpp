#pragma once

#include <types.hpp>

#include "shader.hpp"

namespace graphics
{
	class ContextState
	{
		public:
			// The currently bound shader.
			defaultable_ref<Shader> shader;

			void default_all()
			{
				// Change every member to its default state:
				shader.make_default();
			}
	};
}