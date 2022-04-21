#pragma once

#include <engine/types.hpp>
#include <math/math.hpp>

#include <string>

namespace engine
{
	struct Relationship;
	struct Transform;
	struct Animator;
}

namespace graphics
{
	struct Animation;
}

namespace engine::meta
{
	namespace display
	{
		struct empty_child_display { void operator()(World& world, Entity child, const Relationship& relationship) {} };
		struct on_child_show_all { auto operator()(Entity child) { return true; } };

		std::string name_and_id(World& world, Entity entity);
		void vectors(const math::Vector& pos, const math::Vector& rot, const math::Vector& scale);
		void transform(World& world, Transform& t); // const Transform& t
		void transform(World& world, Entity entity);

		// `display_name` must be NULL-terminated.
		void animation(const graphics::Animation& a, std::string_view display_name="Animation");
		void animator(Animator& animator);

		void child_tree(World& world, Entity entity);
		void transform_tree(World& world, Entity entity);
		void skeletal_tree(World& world, Entity entity);
	}

	void animation_control(World& world, Entity entity);
	void hierarchy_control(World& world, Entity entity);
}