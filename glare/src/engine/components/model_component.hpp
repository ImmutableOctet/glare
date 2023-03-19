#pragma once

#include <graphics/math_types.hpp>

#include <cstdint>
#include <memory>

namespace graphics
{
	class Model;
}

namespace engine
{
	struct ModelComponent
	{
		//ModelComponent(std::shared_ptr<graphics::Model> model);

		std::shared_ptr<graphics::Model> model;

		graphics::ColorRGBA color = { 1.0f, 1.0f, 1.0f, 1.0f };

		// Flags:
		bool visible            : 1 = true;
		bool always_transparent : 1 = false;
		
		bool casts_shadow       : 1 = true;

		bool receives_shadow    : 1 = true;
		bool receives_light     : 1 = true;

		bool transparent() const;

		inline bool get_visible() const { return visible; }
		inline void set_visible(bool value) { visible = value; }

		inline bool get_always_transparent() const { return always_transparent; }
		inline void set_always_transparent(bool value) { always_transparent = value; }

		inline bool get_casts_shadow() const { return casts_shadow; }
		inline void set_casts_shadow(bool value) { casts_shadow = value; }

		inline bool get_receives_shadow() const { return receives_shadow; }
		inline void set_receives_shadow(bool value) { receives_shadow = value; }

		inline bool get_receives_light() const { return receives_light; }
		inline void set_receives_light(bool value) { receives_light = value; }
	};
}