#include "meta_variable_evaluation_context.hpp"

#include "indirection.hpp"

#include "meta_variable_storage_interface.hpp"
#include "meta_variable_context.hpp"
#include "meta_variable_description.hpp"

//#include "hash.hpp"

#include <utility>

namespace engine
{
	MetaVariableEvaluationContext::MetaVariableEvaluationContext(VariableStoreLookupTable&& variables)
		: variables(std::move(variables)) {}

	MetaVariableEvaluationContext::MetaVariableEvaluationContext
	(
		MetaVariableStorageInterface* local_variables,
		MetaVariableStorageInterface* global_variables,
		MetaVariableStorageInterface* context_variables,
		MetaVariableStorageInterface* universal_variables
	)
		: variables
		(
			{
				local_variables,
				global_variables,
				context_variables,
				universal_variables
			}
		)
	{}

	MetaVariableEvaluationContext::VariableStoreLookupTable& MetaVariableEvaluationContext::set_lookup_table(VariableStoreLookupTable&& variables)
	{
		this->variables = std::move(variables);

		return this->variables;
	}

	const MetaVariableEvaluationContext::VariableStoreLookupTable& MetaVariableEvaluationContext::get_lookup_table() const
	{
		return variables;
	}

	MetaVariableStorageInterface* MetaVariableEvaluationContext::get_storage(MetaVariableScope scope)
	{
		const auto scope_index = static_cast<std::size_t>(scope);

		assert(scope_index < variables.size());

		return variables[scope_index];
	}

	const MetaVariableStorageInterface* MetaVariableEvaluationContext::get_storage(MetaVariableScope scope) const
	{
		const auto scope_index = static_cast<std::size_t>(scope);

		assert(scope_index < variables.size());

		return variables[scope_index];
	}

	MetaAny* MetaVariableEvaluationContext::get_ptr(MetaVariableScope scope, MetaSymbolID name)
	{
		auto* scope_storage = get_storage(scope);

		if (!scope_storage)
		{
			return {};
		}

		return scope_storage->get(name);
	}

	// NOTE: This overload does not preserve the constness of the requested variable.
	MetaAny* MetaVariableEvaluationContext::get_ptr(MetaVariableScope scope, MetaSymbolID name) const
	{
		return const_cast<MetaVariableEvaluationContext*>(this)->get_ptr(scope, name);
	}

	MetaAny MetaVariableEvaluationContext::get(MetaVariableScope scope, MetaSymbolID name)
	{
		if (auto entry = get_ptr(scope, name))
		{
			return entry->as_ref();
		}

		return {};
	}

	MetaAny MetaVariableEvaluationContext::get(MetaVariableScope scope, MetaSymbolID name) const
	{
		return const_cast<MetaVariableEvaluationContext*>(this)->get(scope, name);
	}

	bool MetaVariableEvaluationContext::set(MetaVariableScope scope, MetaSymbolID name, MetaAny&& value, bool override_existing, bool allow_variable_allocation)
	{
		return set_impl
		(
			scope, name, std::move(value),
			override_existing, allow_variable_allocation
		);
	}

	bool MetaVariableEvaluationContext::set
	(
		MetaVariableScope scope, MetaSymbolID name, MetaAny&& value,
		bool override_existing, bool allow_variable_allocation,
		Registry& registry, Entity entity
	)
	{
		return set_impl
		(
			scope, name, std::move(value),
			override_existing, allow_variable_allocation,
			registry, entity
		);
	}

	bool MetaVariableEvaluationContext::set
	(
		MetaVariableScope scope, MetaSymbolID name, MetaAny&& value,
		bool override_existing, bool allow_variable_allocation,
		Registry& registry, Entity entity, const MetaEvaluationContext& context
	)
	{
		return set_impl
		(
			scope, name, std::move(value),
			override_existing, allow_variable_allocation,
			registry, entity, context
		);
	}

	bool MetaVariableEvaluationContext::set
	(
		MetaVariableScope scope, MetaSymbolID name, MetaAny&& value,
		bool override_existing, bool allow_variable_allocation,
		const MetaEvaluationContext& context
	)
	{
		return set_impl
		(
			scope, name, std::move(value),
			override_existing, allow_variable_allocation,
			context
		);
	}

	bool MetaVariableEvaluationContext::exists(MetaVariableScope scope, MetaSymbolID name) const
	{
		return static_cast<bool>(get_ptr(scope, name));
	}

	bool MetaVariableEvaluationContext::declare(MetaVariableScope scope, MetaSymbolID name, const MetaType& type)
	{
		return set
		(
			scope, name,

			(type)
				? type.construct()
				: MetaAny {}
			,

			false, true
		);
	}

	bool MetaVariableEvaluationContext::declare(MetaVariableScope scope, MetaSymbolID name, MetaTypeID type_id)
	{
		if (type_id)
		{
			auto type = resolve(type_id);

			if (!type)
			{
				return false;
			}

			return declare(scope, name, type);
		}
		
		return declare(scope, name);
	}

	bool MetaVariableEvaluationContext::declare(MetaVariableScope scope, MetaSymbolID name, MetaAny&& value)
	{
		return set(scope, name, std::move(value), false, true);
	}

	bool MetaVariableEvaluationContext::declare(MetaVariableScope scope, MetaSymbolID name)
	{
		return declare(scope, name, MetaAny {});
	}

	bool MetaVariableEvaluationContext::declare(const MetaVariableDescription& variable)
	{
		return declare(variable.scope, variable.resolved_name, variable.type_id);
	}

	std::size_t MetaVariableEvaluationContext::declare(const MetaVariableContext& context)
	{
		std::size_t count = 0;

		for (const auto& entry : context.variables)
		{
			if (declare(entry))
			{
				count++;
			}
		}

		return count;
	}

	MetaVariableEvaluationContext::operator bool() const
	{
		for (const auto& entry : variables)
		{
			if (entry)
			{
				return true;
			}
		}

		return false;
	}

	template <typename ...EvaluationArgs>
	bool MetaVariableEvaluationContext::set_impl(MetaVariableScope scope, MetaSymbolID name, MetaAny&& value, bool override_existing, bool allow_variable_allocation, EvaluationArgs&&... args)
	{
		auto* scope_storage = get_storage(scope);

		if (!scope_storage)
		{
			return false;
		}

		auto* existing = scope_storage->get(name);

		if (existing)
		{
			if (override_existing)
			{
				if (auto underlying = try_get_underlying_value(value, std::forward<EvaluationArgs>(args)...))
				{
					*existing = std::move(underlying);
				}
				else
				{
					*existing = std::move(value);
				}

				return true;
			}
		}
		else
		{
			if (allow_variable_allocation)
			{
				if (auto underlying = try_get_underlying_value(value, std::forward<EvaluationArgs>(args)...))
				{
					return scope_storage->set(name, std::move(underlying));
				}
				else
				{
					return scope_storage->set(name, std::move(value));
				}
			}
		}

		return false;
	}
}