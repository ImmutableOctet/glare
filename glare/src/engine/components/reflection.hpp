#pragma once

#include <engine/reflection.hpp>

#include "name_component.hpp"
#include "type_component.hpp"
#include "instance_component.hpp"
#include "forwarding_component.hpp"
#include "relationship_component.hpp"
#include "transform_component.hpp"
#include "transform_history_component.hpp"
#include "model_component.hpp"

#include <graphics/model.hpp>

namespace engine
{
	GENERATE_SINGLE_FIELD_COMPONENT_REFLECTION(NameComponent, name);
	GENERATE_SINGLE_FIELD_COMPONENT_REFLECTION(TypeComponent, type);
	GENERATE_SINGLE_FIELD_COMPONENT_REFLECTION(InstanceComponent, instance);
	GENERATE_SINGLE_FIELD_COMPONENT_REFLECTION(ForwardingComponent, root_entity);

	// TODO: Reflect portions of the public API.
	GENERATE_EMPTY_TYPE_REFLECTION(RelationshipComponent);

	template <>
	void reflect<TransformComponent>()
	{
		engine_meta_type<TransformComponent>()
			.data<&TransformComponent::translation>("translation"_hs)
			.data<&TransformComponent::scale>("scale"_hs)
			.data<&TransformComponent::basis>("basis"_hs)

			/*
			// TODO: Implement proper non-default constructor for `TransformComponent`.
			.ctor
			<
				decltype(TransformComponent::translation),
				decltype(TransformComponent::scale),
				decltype(TransformComponent::basis)
			>()
			*/
		;
	}

	template <>
	void reflect<ModelComponent>()
	{
		engine_meta_type<ModelComponent>()
			.data<&ModelComponent::model>("model"_hs)
			.data<&ModelComponent::color>("color"_hs)

			// Getter/setter properties (no unique address):
			.data<&ModelComponent::set_visible,            &ModelComponent::get_visible>("visible"_hs)
			.data<&ModelComponent::set_always_transparent, &ModelComponent::get_always_transparent>("always_transparent"_hs)
			.data<&ModelComponent::set_casts_shadow,       &ModelComponent::get_casts_shadow>("casts_shadow"_hs)
			.data<&ModelComponent::set_receives_shadow,    &ModelComponent::get_receives_shadow>("receives_shadow"_hs)
			.data<&ModelComponent::set_receives_light,     &ModelComponent::get_receives_light>("receives_light"_hs)

			.ctor<decltype(ModelComponent::model), decltype(ModelComponent::color)>()
			.ctor<decltype(ModelComponent::model)>()
		;
	}

	template <>
	void reflect<TransformHistoryComponent>()
	{
		engine_meta_type<TransformHistoryComponent>()
			.data<nullptr, &TransformHistoryComponent::get_prev_vectors>("prev_vectors"_hs)
			.data<nullptr, &TransformHistoryComponent::get_prev_position>("prev_position"_hs)
			.data<nullptr, &TransformHistoryComponent::get_prev_rotation>("prev_rotation"_hs)
			.data<nullptr, &TransformHistoryComponent::get_prev_scale>("prev_scale"_hs)
			.data<nullptr, &TransformHistoryComponent::get_history>("history"_hs)
			.data<nullptr, &TransformHistoryComponent::size>("size"_hs)
			.data<nullptr, &TransformHistoryComponent::capacity>("capacity"_hs)

			.func<&TransformHistoryComponent::get_avg_position>("get_avg_position"_hs)
			.func<&TransformHistoryComponent::get_avg_rotation>("get_avg_rotation"_hs)
			.func<&TransformHistoryComponent::get_avg_scale>("get_avg_scale"_hs)
			//.func<static_cast<void(*)(const TransformHistoryComponent::OptimizedTransform&, bool)>(&TransformHistoryComponent::push_snapshot)>("push_snapshot"_hs)
		;
	}

	inline void reflect_core_components()
	{
		reflect<NameComponent>();
		reflect<TypeComponent>();
		reflect<InstanceComponent>();
		reflect<ForwardingComponent>();
		reflect<RelationshipComponent>();
		reflect<TransformHistoryComponent>();

		// TODO: Implement reflection for `Transform` type as well.
		reflect<TransformComponent>();

		reflect<ModelComponent>();
	}
}