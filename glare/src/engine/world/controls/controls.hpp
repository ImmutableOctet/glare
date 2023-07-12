#pragma once

#include <engine/types.hpp>
#include <engine/meta/types.hpp>

#include <math/types.hpp>

#include <string>
#include <string_view>

namespace graphics
{
	struct Animation;
}

namespace engine
{
	struct RelationshipComponent;
	struct Transform;
	struct AnimationComponent;

	namespace display
	{
		struct empty_child_display { void operator()(World& world, Entity child, const RelationshipComponent& relationship) {} };
		struct on_child_show_all { auto operator()(Entity child) { return true; } };
		
		bool vector(std::string_view display_text, math::Vector4D& v, bool allow_modification=false);
		void vector(std::string_view display_text, const math::Vector4D& v);

		bool vector(std::string_view display_text, math::Vector3D& v, bool allow_modification=false);
		void vector(std::string_view display_text, const math::Vector3D& v);

		bool vector(std::string_view display_text, math::Vector2D& v, bool allow_modification=false);
		void vector(std::string_view display_text, const math::Vector2D& v);

		void transform(const math::Vector& position, const math::Vector& rotation, const math::Vector& scale);
		void transform(const math::TransformVectors& transform_vectors);

		// TODO: Implement mutating overload.
		void transform(const Transform& t);

		// TODO: Implement optional mutation.
		void transform(Registry& registry, Entity entity); // bool allow_modification=false

		// TODO: Implement optional mutation.
		void transform(World& world, Entity entity); // bool allow_modification=false

		// `display_name` must be NULL-terminated.
		void animation(const graphics::Animation& a, std::string_view display_name="Animation");
		void animator(AnimationComponent& animator);

		void child_tree(World& world, Entity entity);
		void transform_tree(World& world, Entity entity);
		void skeletal_tree(World& world, Entity entity);

		void opaque_value(const MetaAny& value);
		bool opaque_value(MetaAny& value, bool allow_modification=false);

		void opaque_value(std::string_view display_text, const MetaAny& value);
		bool opaque_value(std::string_view display_text, MetaAny& value, bool allow_modification=false);

		bool component_content(Registry& registry, Entity entity, const MetaType& component_type, bool allow_modification=false, bool* component_modified_out=nullptr);

		bool component(Registry& registry, Entity entity, MetaTypeID component_type_id, bool allow_modification=false, bool* component_modified_out=nullptr, bool highlight=false);
		bool component(Registry& registry, Entity entity, const MetaType& component_type, bool allow_modification=false, bool* component_modified_out=nullptr, bool highlight=false);

		std::size_t components
		(
			Registry& registry, Entity entity,

			bool allow_modification=false,

			bool display_from_mutations=true,
			bool display_from_descriptor=true,
			bool check_mutation_status=true
		);
	}

	void animation_control(World& world, Entity entity);
	void hierarchy_control(World& world, Entity entity);
}