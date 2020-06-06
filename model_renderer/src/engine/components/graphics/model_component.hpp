#pragma once

#include <types.hpp>
#include <graphics/types.hpp>

namespace graphics
{
	class Model;
}

namespace engine
{
	struct ModelComponent
	{
		//ModelComponent(ref<graphics::Model> model);

		ref<graphics::Model> model;

		graphics::ColorRGBA color = { 1.0f, 1.0f, 1.0f, 1.0f };

		bool visible = true;
		bool always_transparent = false;

		inline bool transparent() const;
	};
}