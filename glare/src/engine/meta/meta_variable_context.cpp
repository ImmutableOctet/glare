#include "meta_variable_context.hpp"
#include "meta_parsing_context.hpp"
#include "hash.hpp"

#include <util/format.hpp>

#include <string>

namespace engine
{
	MetaVariableContext::MetaVariableContext(MetaVariableContext* parent_context, MetaSymbolID name)
		: parent_context(parent_context), name(name)
	{}

	MetaVariableContext::MetaVariableContext(const MetaParsingContext& parent_parsing_context)
		: MetaVariableContext(parent_parsing_context.get_variable_context())
	{}

	std::tuple<const MetaVariableDescription*, bool>
	MetaVariableContext::define_or_retrieve_variable(MetaVariableScope scope, std::string_view scope_local_name, MetaTypeID type_id, bool forward_to_parent, bool fail_on_different_type)
	{
		if (forward_to_parent && parent_context)
		{
			if (auto parent_result = parent_context->define_or_retrieve_variable(scope, scope_local_name, type_id, forward_to_parent); std::get<1>(parent_result))
			{
				return parent_result;
			}
		}

		const auto resolved_id = get_resolved_id(scope, scope_local_name);

		if (auto existing = retrieve_variable_by_resolved_id(resolved_id, (scope != MetaVariableScope::Local)))
		{
			if (fail_on_different_type && type_id && existing->type_id)
			{
				if (existing->type_id != type_id)
				{
					return { {}, false };
				}
			}

			return { existing, false };
		}

		const auto scope_local_id = (scope == MetaVariableScope::Local)
			? hash(scope_local_name).value()
			: resolved_id // <-- Minor optimization to avoid hashing twice.
		;

		const auto& new_entry = variables.emplace_back
		(
			scope_local_id,
			resolved_id,
			type_id,
			scope
		);

		return { &new_entry, true };
	}

	MetaSymbolID MetaVariableContext::resolve_path(MetaSymbolID prefix, std::string_view scope_local_name, MetaVariableScope scope)
	{
		if (scope_local_name.empty())
		{
			return {};
		}

		const auto scope_local_id = hash(scope_local_name).value();

		return resolve_path(prefix, scope_local_id, scope);
	}

	MetaSymbolID MetaVariableContext::resolve_path(MetaSymbolID prefix, MetaSymbolID scope_local_id, MetaVariableScope scope)
	{
		if (prefix)
		{
			if (!scope_local_id)
			{
				return prefix;
			}

			if (scope == MetaVariableScope::Local)
			{
				std::string _temp_name_resolved; // static thread_local

				_temp_name_resolved = util::format("{}->{}", prefix, scope_local_id);

				return hash(_temp_name_resolved).value();
			}
		}

		return scope_local_id;
	}

	MetaSymbolID MetaVariableContext::get_resolved_id(MetaVariableScope scope, std::string_view scope_local_name) const
	{
		if (scope_local_name.empty())
		{
			return {};
		}

		const auto scope_local_id = hash(scope_local_name).value();

		return get_resolved_id(scope, scope_local_id);
	}

	MetaSymbolID MetaVariableContext::get_resolved_id(MetaVariableScope scope, MetaSymbolID scope_local_id) const
	{
		if (!scope_local_id)
		{
			return {};
		}

		if (scope == MetaVariableScope::Local)
		{
			return resolve_path(get_path(), scope_local_id, scope);
		}

		return scope_local_id;
	}

	const MetaVariableDescription* MetaVariableContext::define_variable(MetaVariableScope scope, std::string_view scope_local_name, MetaTypeID type_id, bool forward_to_parent)
	{
		if (forward_to_parent && parent_context)
		{
			if (auto parent_result = parent_context->define_variable(scope, scope_local_name, type_id, forward_to_parent))
			{
				return parent_result;
			}
		}

		const auto scope_local_id = hash(scope_local_name).value();
		const auto resolved_id = get_resolved_id(scope, scope_local_id);

		if (auto existing = retrieve_variable_by_resolved_id(resolved_id, (scope != MetaVariableScope::Local)))
		{
			return {};
		}

		const auto& new_entry = variables.emplace_back
		(
			scope_local_id,
			resolved_id,
			type_id,
			scope
		);

		return &new_entry;
	}

	const MetaVariableDescription* MetaVariableContext::retrieve_variable(std::string_view scope_local_name, bool fallback_to_parent) const
	{
		return retrieve_variable(hash(scope_local_name).value(), fallback_to_parent);
	}

	const MetaVariableDescription* MetaVariableContext::retrieve_variable(MetaSymbolID scope_local_id, bool fallback_to_parent) const
	{
		for (const auto& entry : variables)
		{
			if (entry.scope_local_name == scope_local_id)
			{
				return &entry;
			}
		}

		if (fallback_to_parent && parent_context)
		{
			return parent_context->retrieve_variable(scope_local_id, fallback_to_parent);
		}

		return {};
	}

	const MetaVariableDescription* MetaVariableContext::retrieve_variable_by_resolved_id(MetaSymbolID resolved_id, bool fallback_to_parent) const
	{
		for (const auto& entry : variables)
		{
			if (entry.resolved_name == resolved_id)
			{
				return &entry;
			}
		}

		if (fallback_to_parent && parent_context)
		{
			return parent_context->retrieve_variable_by_resolved_id(resolved_id, fallback_to_parent);
		}

		return {};
	}

	MetaVariableContext* MetaVariableContext::get_parent()
	{
		return parent_context;
	}

	const MetaVariableContext* MetaVariableContext::get_parent() const
	{
		return parent_context;
	}

	MetaVariableContext* MetaVariableContext::get_root()
	{
		if (parent_context)
		{
			// NOTE: Recursion.
			return parent_context->get_root();
		}
		else
		{
			return this;
		}
	}

	const MetaVariableContext* MetaVariableContext::get_root() const
	{
		if (parent_context)
		{
			// NOTE: Recursion.
			return parent_context->get_root();
		}
		else
		{
			return this;
		}
	}

	bool MetaVariableContext::contains(MetaSymbolID scope_local_id) const
	{
		return retrieve_variable(scope_local_id);
	}

	bool MetaVariableContext::contains(std::string_view scope_local_name) const
	{
		return retrieve_variable(scope_local_name);
	}

	bool MetaVariableContext::contains(MetaVariableScope scope, MetaSymbolID scope_local_id) const
	{
		auto existing = retrieve_variable(scope_local_id);

		if (!existing)
		{
			return false;
		}

		return (existing->scope == scope);
	}

	bool MetaVariableContext::contains_resolved(MetaSymbolID resolved_id) const
	{
		return retrieve_variable_by_resolved_id(resolved_id);
	}

	bool MetaVariableContext::contains_resolved(MetaVariableScope scope, MetaSymbolID resolved_id) const
	{
		auto existing = retrieve_variable_by_resolved_id(resolved_id);

		if (!existing)
		{
			return false;
		}

		return (existing->scope == scope);
	}

	bool MetaVariableContext::has_parent() const
	{
		return static_cast<bool>(parent_context);
	}

	bool MetaVariableContext::parent_is_root() const
	{
		if (!parent_context)
		{
			return false;
		}

		return (get_root() == parent_context);
	}

	bool MetaVariableContext::has_name() const
	{
		return (name != MetaSymbolID {});
	}

	bool MetaVariableContext::set_name(MetaSymbolID name, bool force)
	{
		if (force || !has_name())
		{
			this->name = name;

			return true;
		}

		return false;
	}

	MetaSymbolID MetaVariableContext::get_name() const
	{
		return name;
	}

	MetaSymbolID MetaVariableContext::get_parent_name(bool recursive) const
	{
		if (parent_context)
		{
			const auto parent_name = parent_context->get_name();

			if (recursive)
			{
				return resolve_path(parent_context->get_parent_name(recursive), parent_name);
			}

			return parent_name;
		}

		return {};
	}

	MetaSymbolID MetaVariableContext::get_path(bool recursive) const
	{
		return resolve_path(get_parent_name(recursive), get_name());
	}

	std::size_t MetaVariableContext::apply_prefix(std::string_view prefix, std::optional<MetaVariableScope> scope)
	{
		return apply_prefix(hash(prefix).value(), scope);
	}

	std::size_t MetaVariableContext::apply_prefix(MetaSymbolID prefix, std::optional<MetaVariableScope> scope)
	{
		for (auto& variable_entry : variables)
		{
			if (!scope || (*scope == variable_entry.scope))
			{
				variable_entry.resolved_name = resolve_path(prefix, variable_entry.resolved_name);
			}
		}

		return variables.size();
	}
}