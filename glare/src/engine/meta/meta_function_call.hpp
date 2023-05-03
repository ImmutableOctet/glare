#pragma once

#include "types.hpp"

#include <util/small_vector.hpp>

namespace engine
{
	struct MetaEvaluationContext;

	struct MetaFunctionCall
	{
		public:
			using Arguments = util::small_vector<MetaAny, 4>; // 8
			using ForwardingArguments = util::small_vector<MetaAny, 5>; // 8

			MetaTypeID type_id;
			MetaFunctionID function_id;

			Arguments arguments;

			MetaAny self = {};

			// When enabled, data-member accesses may be performed using function-call syntax.
			bool allow_fallback_to_member : 1 = true;

			MetaAny get_self(bool resolve_as_container=false, bool resolve_underlying=true) const;

			MetaAny get(MetaAny self={}, bool resolve_indirection=true) const;
			MetaAny get(Registry& registry, Entity entity=null, bool resolve_indirection=true, MetaAny self={}, const MetaEvaluationContext* opt_evaluation_context=nullptr) const;

			bool has_indirection() const;

			MetaFunction get_function() const;
			MetaFunction get_function(const MetaAny& self) const;
			MetaFunction get_function(const MetaType& type) const;

			MetaType get_type() const;

			bool has_type() const;
			bool has_function() const;

			// Wrapper for `get`; added for reflection purposes.
			MetaAny get_indirect_value() const;

			// Wrapper for `get`; added for reflection purposes.
			MetaAny get_indirect_value(const MetaEvaluationContext& context) const;

			// Wrapper for `get`; added for reflection purposes.
			MetaAny get_indirect_value(const MetaAny& self) const;

			// Wrapper for `get`; added for reflection purposes.
			MetaAny get_indirect_value(const MetaAny& self, Registry& registry, Entity entity) const;

			// Wrapper for `get`; added for reflection purposes.
			MetaAny get_indirect_value(const MetaAny& self, Registry& registry, Entity entity, const MetaEvaluationContext& context) const;

			// Wrapper for `get`; added for reflection purposes.
			MetaAny get_indirect_value(Registry& registry, Entity entity) const;

			// Wrapper for `get`; added for reflection purposes.
			MetaAny get_indirect_value(Registry& registry, Entity entity, const MetaEvaluationContext& context) const;

			MetaAny set_indirect_value(MetaAny& value);
			MetaAny set_indirect_value(MetaAny& value, const MetaEvaluationContext& context);
			MetaAny set_indirect_value(MetaAny& value, Registry& registry, Entity entity);
			MetaAny set_indirect_value(MetaAny& value, Registry& registry, Entity entity, const MetaEvaluationContext& context);

			MetaAny set_indirect_value(MetaAny& source, MetaAny& destination);
			MetaAny set_indirect_value(MetaAny& source, MetaAny& destination, const MetaEvaluationContext& context);
			MetaAny set_indirect_value(MetaAny& source, MetaAny& destination, Registry& registry, Entity entity);
			MetaAny set_indirect_value(MetaAny& source, MetaAny& destination, Registry& registry, Entity entity, const MetaEvaluationContext& context);

			inline MetaAny operator()(MetaAny self={}, bool resolve_indirection=true) const
			{
				return get(std::move(self), resolve_indirection);
			}

			inline MetaAny operator()(Registry& registry, Entity entity, bool resolve_indirection=true, MetaAny self={}, const MetaEvaluationContext* opt_evaluation_context=nullptr) const
			{
				return get(registry, entity, resolve_indirection, std::move(self), opt_evaluation_context);
			}
		private:
			template <typename ...Args>
			MetaAny get_self_impl(bool resolve_as_container, bool resolve_underlying, Args&&... args) const;

			template <typename ...Args>
			MetaAny get_self(bool resolve_as_container, bool resolve_underlying, Args&&... args) const;

			template <typename ...Args>
			MetaAny get_fallback_impl(const MetaAny& self, bool resolve_indirection, Args&&... args) const;

			template <typename ...Args>
			MetaAny get_fallback(const MetaAny& self, bool resolve_indirection, Args&&... args) const;

			template <typename ...Args>
			MetaAny set_fallback_impl(MetaAny self, MetaAny& value, bool resolve_indirection, Args&&... args);

			template <typename ...Args>
			MetaAny set_fallback(MetaAny self, MetaAny& value, bool resolve_indirection, Args&&... args);

			template <typename ...Args>
			MetaAny set_with_fallback_impl(MetaAny& source, MetaAny& destination, bool resolve_indirection, Args&&... args);

			template <typename ...Args>
			MetaAny set_with_fallback(MetaAny& source, MetaAny& destination, bool resolve_indirection, Args&&... args);

			template <typename ArgumentContainer>
			MetaAny execute_impl(ArgumentContainer&& arguments, MetaAny self={}, bool resolve_indirection=true) const;

			template <typename ArgumentContainer>
			MetaAny execute_impl(ArgumentContainer&& arguments, Registry& registry, Entity entity=null, bool resolve_indirection=true, MetaAny self={}, const MetaEvaluationContext* opt_evaluation_context=nullptr) const;

			// TODO: Implement an overload of `execute_impl` that takes in only a `MetaEvaluationContext` reference.
	};
}