#pragma once

#include "types.hpp"

#include <string_view>
#include <tuple>

namespace engine
{
	struct MetaEvaluationContext;

	/*
		An abstraction for a handling access to a member.
		
		A property may be defined either by specifying a pair of functions,
		or by directly specifying a type and data-member.

		See: `getter_id`, `setter_id`, `data_member_id`
	*/
	struct MetaProperty
	{
		public:
			// Generates a pair of identifiers used to resolve accessor functions.
			static std::tuple
			<
				MetaSymbolID, // getter_id
				MetaSymbolID  // setter_id
			>
			generate_accessor_identifiers(std::string_view symbol);

			// An optional identifier for the underlying type this property is accessing.
			// If this is left unspecified, type checks will be ignored where possible.
			MetaTypeID type_id = {};

			// A function identifier used to retrieve a value from this 'property'.
			// If this function is not specified, calls to `get` will immediately fail.
			MetaFunctionID getter_id = {};

			// A function identifier used to assign the value of this 'property'.
			// If this function is not specified, calls to `set` will immediately fail.
			MetaFunctionID setter_id = {};

			// An optional identifier used to represent the formal name of this property.
			// If specified, this field may also be used as a fallback when calling a `getter` or `setter` function.
			MetaSymbolID data_member_id = {};

			// Attempts to call `getter` as a static member-function.
			MetaAny get() const;

			// Attempts to call `getter` as a static member-function,
			// optionally passing `context` for additional context.
			MetaAny get(const MetaEvaluationContext& context) const;

			/*
				Attempts to call `getter` with `instance`.
				
				If `type_id` is specified, and the type of `instance`
				does not match `type_id`, this will fail.
			*/
			MetaAny get(const MetaAny& instance) const;

			/*
				Attempts to call `getter` with `instance`,
				optionally using `context` for additional context.

				If `type_id` is specified, and the type of `instance`
				does not match `type_id`, this will fail.
			*/
			MetaAny get(const MetaAny& instance, const MetaEvaluationContext& context) const;

			/*
				This overload first attempts to retrieve a component instance of the type
				identified by `type_id` from `entity`, then calls `getter` with that object.
				
				If `type_id` refers to the `Entity` type, or if `type_id` is unspecified,
				this will attempt to call `getter` using `entity`.
				
				If these initial attempts fail, an attempt as a static member-function will be tried.

				All attempts allow for passing of `registry` and `entity` as parameters for additional context.
			*/
			MetaAny get(Registry& registry, Entity entity) const;

			/*
				Attempts to call `getter` with `instance`, optionally using `registry` and `context_entity` for additional context.
				
				If `type_id` is specified, and the type of `instance` does not match `type_id`, this will fail.
			*/
			MetaAny get(const MetaAny& instance, Registry& registry, Entity context_entity) const;

			/*
				Attempts to call `getter` with `instance`, optionally using `registry`, `context_entity` and `context` for additional context.

				If `type_id` is specified, and the type of `instance` does not match `type_id`, this will fail.
			*/
			MetaAny get(const MetaAny& instance, Registry& registry, Entity context_entity, const MetaEvaluationContext& context) const;

			/*
				Attempts to retrieve a component of the type identified by `type_id`
				from `target`, then attempts to access the desired 'property' from that object.
				
				If this fails, or if the desired type is `Entity`, this will fallback to
				attempting to access the 'property' from `target` itself.

				All attempts allow for passing of `registry` and `context_entity` as parameters for additional context.
			*/
			MetaAny get(Entity target, Registry& registry, Entity context_entity) const;

			/*
				Attempts to retrieve a component of the type identified by `type_id`
				from `target`, then attempts to access the desired 'property' from that object.
				
				If this fails, or if the desired type is `Entity`, this will fallback to
				attempting to access the 'property' from `target` itself.

				All attempts allow for passing of `registry`, `context_entity` and `context` as parameters for additional context.
			*/
			MetaAny get(Entity target, Registry& registry, Entity context_entity, const MetaEvaluationContext& context) const;

			// Attempts to call `setter` as a static member-function using `value` as input.
			MetaAny set(MetaAny& value);

			// Attempts to call `setter` as a static member-function using `value` as input,
			// optionally passing `context` for additional context.
			MetaAny set(MetaAny& value, const MetaEvaluationContext& context);

			/*
				Attempts to retrieve a component of the type specified by `type_id` from
				`entity` using `registry`, then tries to call `setter` on the component with `value`.

				If this fails, this will fallback to calling `setter` as a static member-function instead.

				All attempts allow for passing of `registry` and `context_entity` as parameters for additional context.
			*/
			MetaAny set(MetaAny& value, Registry& registry, Entity entity);

			/*
				Attempts to retrieve a component of the type specified by `type_id` from
				`entity` using `registry`, then tries to call `setter` on the component with `value`.

				If this fails, this will fallback to calling `setter` as a static member-function instead.

				All attempts allow for passing of `registry`, `context_entity` and `context` as parameters for additional context.
			*/
			MetaAny set(MetaAny& value, Registry& registry, Entity entity, const MetaEvaluationContext& context);

			/*
				Attempts to call `setter` with `destination`, using `source` for assignment.
				
				If the type of `destination` differs from the type identified by `type_id`, this will fail.
			*/
			MetaAny set(MetaAny& source, MetaAny& destination);

			/*
				Attempts to call `setter` with `destination`, using `source` for assignment.

				This overload allows for passing of `context` as a parameter for additional context.

				If the type of `destination` differs from the type identified by `type_id`, this will fail.
			*/
			MetaAny set(MetaAny& source, MetaAny& destination, const MetaEvaluationContext& context);

			/*
				Attempts to call `setter` with `destination`, using `source` for assignment.
				
				This overload allows for passing of `registry` and `entity` as parameters for additional context.
				
				If the type of `destination` differs from the type identified by `type_id`, this will fail.
			*/
			MetaAny set(MetaAny& source, MetaAny& destination, Registry& registry, Entity entity);

			/*
				Attempts to call `setter` with `destination`, using `source` for assignment.
				
				This overload allows for passing of `registry`, `entity` and `context` as parameters for additional context.
				
				If the type of `destination` differs from the type identified by `type_id`, this will fail.
			*/
			MetaAny set(MetaAny& source, MetaAny& destination, Registry& registry, Entity entity, const MetaEvaluationContext& context);

			// Indicates whether `type_id` is specified.
			bool has_type() const;

			// Attempts to resolve the type identified by `type_id`.
			MetaType get_type() const;

			// Returns true if both `type_id` and `getter_id` are specified.
			bool has_member_type() const;

			/*
				If `getter_id` is specified, this attempts to retrieve the
				type of the value returned by a call to `getter`.

				If `getter_id` is not specified, but `data_member_id` is, this attempts
				to retrieve the type of the data-member identified by `data_member_id`.
				
				NOTE: This method returns `getter`'s first overload's return-type,
				which may differ from the type of the value provided during resolution.
			*/
			MetaType get_member_type() const;

			// Indicates whether `data_member_id` is specified.
			bool has_data_member() const;

			// Indicates whether `getter_id`, `setter_id` or `data_member_id` is specified.
			bool has_member_access() const;

			// Returns true if `getter_id` is specified.
			bool has_getter() const;

			// Returns true if `setter_id` is specified.
			bool has_setter() const;

			// Retrieves the getter-function referenced by `getter_id`, using `type`.
			MetaFunction getter(const MetaType& type) const;

			// Retrieves the getter-function referenced by `getter_id`.
			// This overload uses `get_type` to retrieve the requested function.
			MetaFunction getter() const;

			// Retrieves the getter-function referenced by `setter_id`, using `type`.
			MetaFunction setter(const MetaType& type) const;

			// Retrieves the setter-function referenced by `setter_id`.
			// This overload uses `get_type` to retrieve the requested function.
			MetaFunction setter() const;
			
			// Retrieves the data-member referenced by `data_member_id`, using `type`.
			entt::meta_data data_member(const MetaType& type) const;

			// Retrieves the data-member referenced by `data_member_id`.
			entt::meta_data data_member() const;
		private:
			template <bool check_type, typename ...Args>
			MetaAny get_from_impl(const MetaAny& instance, Args&&... args) const;

			template <typename ...Args>
			MetaAny get_from_impl_ex(const MetaAny& instance, const MetaType& instance_type, Args&&... args) const;

			template <bool fallback_to_entity_type, typename ...Args>
			MetaAny get_from_entity_impl(Entity target, Registry& registry, Args&&... args) const;

			template <bool check_value_type, typename ...Args>
			MetaAny static_set_impl(MetaAny& value, Args&&... args);

			template <bool check_value_type, typename ...Args>
			MetaAny static_set_impl_ex(const MetaType& type, MetaAny& value, Args&&... args);

			template <bool check_source_type, bool check_destination_type, typename ...Args>
			MetaAny set_with_impl(MetaAny& source, MetaAny& destination, Args&&... args);

			template <bool check_source_type, bool check_destination_type, typename ...Args>
			MetaAny set_with_impl_ex(MetaAny& source, MetaAny& destination, const MetaType& destination_type, Args&&... args);

			template <bool fallback_to_entity_type, bool check_value_type, typename ...Args>
			MetaAny set_impl(MetaAny& value, Registry& registry, Entity entity, Entity context_entity, Args&&... args);

			template <typename ...Args>
			MetaAny get_fallback_impl(Args&&... args) const;

			template <typename ...Args>
			MetaAny set_fallback_impl(Args&&... args);
	};
}