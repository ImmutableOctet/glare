#pragma once

#include "types.hpp"
#include "meta_variable_scope.hpp"

#include <array>

namespace engine
{
	class MetaVariableStorageInterface;
	class MetaVariableContext;
	
	struct MetaEvaluationContext;
	struct MetaVariableDescription;

	class MetaVariableEvaluationContext
	{
		public:
			using VariableStoreLookupTable = std::array
			<
				MetaVariableStorageInterface*,
				static_cast<std::size_t>(MetaVariableScope::Count)
			>;

			MetaVariableEvaluationContext() = default;

			MetaVariableEvaluationContext(VariableStoreLookupTable&& variables);

			MetaVariableEvaluationContext
			(
				MetaVariableStorageInterface* local_variables,
				MetaVariableStorageInterface* global_variables={},
				MetaVariableStorageInterface* context_variables={},
				MetaVariableStorageInterface* universal_variables={}
			);

			MetaVariableEvaluationContext(const MetaVariableEvaluationContext&) = default;
			MetaVariableEvaluationContext(MetaVariableEvaluationContext&&) noexcept = default;

			VariableStoreLookupTable& set_lookup_table(VariableStoreLookupTable&& variables);
			const VariableStoreLookupTable& get_lookup_table() const;

			MetaVariableStorageInterface* get_storage(MetaVariableScope scope);
			const MetaVariableStorageInterface* get_storage(MetaVariableScope scope) const;

			inline MetaVariableStorageInterface* operator[](MetaVariableScope scope)
			{
				return get_storage(scope);
			}

			inline const MetaVariableStorageInterface* operator[](MetaVariableScope scope) const
			{
				return get_storage(scope);
			}

			MetaAny* get_ptr(MetaVariableScope scope, MetaSymbolID name);

			// NOTE: This overload does not preserve the constness of the requested variable.
			MetaAny* get_ptr(MetaVariableScope scope, MetaSymbolID name) const;

			MetaAny get(MetaVariableScope scope, MetaSymbolID name);

			// NOTE: This overload does not preserve the constness of the requested variable.
			MetaAny get(MetaVariableScope scope, MetaSymbolID name) const;

			bool set
			(
				MetaVariableScope scope, MetaSymbolID name, MetaAny&& value,
				bool override_existing=true, bool allow_variable_allocation=true
			);

			bool set
			(
				MetaVariableScope scope, MetaSymbolID name, MetaAny&& value,
				bool override_existing, bool allow_variable_allocation,
				Registry& registry, Entity entity
			);

			bool set
			(
				MetaVariableScope scope, MetaSymbolID name, MetaAny&& value,
				bool override_existing, bool allow_variable_allocation,
				Registry& registry, Entity entity, const MetaEvaluationContext& context
			);

			bool set
			(
				MetaVariableScope scope, MetaSymbolID name, MetaAny&& value,
				bool override_existing, bool allow_variable_allocation,
				const MetaEvaluationContext& context
			);

			// Checks if a variable named `name` exists within `scope`.
			bool exists(MetaVariableScope scope, MetaSymbolID name) const;

			bool declare(MetaVariableScope scope, MetaSymbolID name, const MetaType& type);
			bool declare(MetaVariableScope scope, MetaSymbolID name, MetaTypeID type_id);
			bool declare(MetaVariableScope scope, MetaSymbolID name, MetaAny&& value);
			bool declare(MetaVariableScope scope, MetaSymbolID name);

			bool declare(const MetaVariableDescription& variable);

			std::size_t declare(const MetaVariableContext& context);

			MetaVariableEvaluationContext& operator=(const MetaVariableEvaluationContext&) = default;
			MetaVariableEvaluationContext& operator=(MetaVariableEvaluationContext&&) noexcept = default;

			explicit operator bool() const;

		protected:
			VariableStoreLookupTable variables = {};

		private:
			template <typename ...EvaluationArgs>
			bool set_impl(MetaVariableScope scope, MetaSymbolID name, MetaAny&& value, bool override_existing, bool allow_variable_allocation, EvaluationArgs&&... args);
	};
}