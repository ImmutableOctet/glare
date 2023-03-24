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

			MetaTypeID type_id;
			MetaFunctionID function_id;

			Arguments arguments;

			MetaAny self = {};

			MetaAny get_self(bool resolve_as_container=false, bool resolve_underlying=true) const;

			MetaAny get(MetaAny self={}, bool resolve_indirection=true) const;
			MetaAny get(Registry& registry, Entity entity=null, bool resolve_indirection=true, MetaAny self={}, const MetaEvaluationContext* opt_evaluation_context=nullptr) const;

			bool has_indirection() const;

			MetaFunction get_function() const;
			MetaFunction get_function(const MetaAny& self) const;
			MetaFunction get_function(const MetaType& type) const;

			MetaType get_type() const;

			bool has_type() const;

			// Wrapper for `get`; added for reflection purposes.
			inline MetaAny get_indirect_value() const
			{
				return get();
			}

			// Wrapper for `get`; added for reflection purposes.
			inline MetaAny get_indirect_value(const MetaAny& self) const
			{
				// NOTE: Const-cast needed here to allow for non-const member-function calls.
				// Usage of `as_ref` allows for reference-policy forwarding without
				// imposing transitive const from reference-type of `self`.
				return get(const_cast<MetaAny&>(self).as_ref(), true);
			}

			// Wrapper for `get`; added for reflection purposes.
			inline MetaAny get_indirect_value(const MetaAny& self, Registry& registry, Entity entity) const
			{
				// NOTE: See basic indirect chain-getter implementation for details on const-cast usage.
				return get(registry, entity, true, const_cast<MetaAny&>(self).as_ref());
			}

			// Wrapper for `get`; added for reflection purposes.
			inline MetaAny get_indirect_value(const MetaAny& self, Registry& registry, Entity entity, const MetaEvaluationContext& context) const
			{
				// NOTE: See basic indirect chain-getter implementation for details on const-cast usage.
				return get(registry, entity, true, const_cast<MetaAny&>(self).as_ref(), &context);
			}

			// Wrapper for `get`; added for reflection purposes.
			inline MetaAny get_indirect_value(Registry& registry, Entity entity) const
			{
				return get(registry, entity, true);
			}

			// Wrapper for `get`; added for reflection purposes.
			inline MetaAny get_indirect_value(Registry& registry, Entity entity, const MetaEvaluationContext& context) const
			{
				return get(registry, entity, true, {}, &context);
			}

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
	};
}