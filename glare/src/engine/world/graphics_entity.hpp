#pragma once

#include <types.hpp>
#include <graphics/types.hpp>
#include "entity.hpp"

#include <string>

namespace graphics
{
	class Model;
	//class Context;
}

namespace engine
{
	class World;
	//class ResourceManager;

	enum class RenderFlags : std::uint32_t
	{
		None = 0,

		IsHidden   = (1 << 0),
		CastsShadow = (1 << 1),
	};

	FLAG_ENUM(std::uint32_t, RenderFlags);

	struct RenderFlagsComponent
	{
		using Flags = RenderFlags;

		RenderFlags flags = Flags::None;
		//unsigned short depth_sort = 0;

		inline bool is_visible() const { return !(flags & Flags::IsHidden); }
		inline bool casts_shadow() const { return !(flags & Flags::CastsShadow); }
	};

	// Entity with 3D Model component.
	Entity create_model(World& world, pass_ref<graphics::Model> model, Entity parent=null);
	Entity attach_model(World& world, Entity entity, pass_ref<graphics::Model> model, graphics::ColorRGBA color = { 1.0f, 1.0f, 1.0f, 1.0f });

	Entity load_model(World& world, const std::string& path, Entity parent=null); // const std::string_view&
}