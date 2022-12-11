#include "meta_type_descriptor.hpp"
#include "meta.hpp"
#include "serial.hpp"

#include <util/string.hpp>

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	MetaType MetaTypeDescriptor::get_self_type()
	{
		return resolve<MetaTypeDescriptor>();
	}

	std::tuple<std::string_view, std::string_view>
	MetaTypeDescriptor::parse_variable_declaration(std::string_view var_decl, std::string_view type_specifier_symbol)
	{
		auto symbol_position = var_decl.find(type_specifier_symbol);

		if (symbol_position == std::string_view::npos)
		{
			return { var_decl, {} };
		}

		auto var_name = var_decl.substr(0, symbol_position);
    
		auto var_type = var_decl.substr
		(
			(symbol_position + type_specifier_symbol.length()),
			(var_decl.length() - symbol_position)
		);

		return { var_name, var_type };
	}

	MetaTypeDescriptor::MetaTypeDescriptor(MetaType type, std::optional<SmallSize> constructor_argument_count, const MetaTypeDescriptorFlags& flags)
		: type(type), constructor_argument_count(constructor_argument_count), flags(flags) {}

	MetaTypeDescriptor::MetaTypeDescriptor(MetaTypeID type_id, std::optional<SmallSize> constructor_argument_count, const MetaTypeDescriptorFlags& flags)
		: type(resolve(type_id)), constructor_argument_count(constructor_argument_count), flags(flags) {}

	MetaTypeDescriptor::MetaTypeDescriptor
	(
		MetaType type,
		std::string_view content,
		std::string_view arg_separator,
		bool resolve_values,
		std::size_t argument_offset,

		std::optional<SmallSize> constructor_argument_count,
		const MetaTypeDescriptorFlags& flags
	) :
		type(type),
		constructor_argument_count(constructor_argument_count),
		flags(flags)
	{
		assert(type);

		set_variables(content, arg_separator, resolve_values, argument_offset);
	}

	MetaTypeDescriptor::MetaTypeDescriptor
	(
		MetaType type,
		const util::json& content,

		std::optional<SmallSize> constructor_argument_count,
		const MetaTypeDescriptorFlags& flags
	) :
		type(type),
		constructor_argument_count(constructor_argument_count),
		flags(flags)
	{
		assert(type);

		set_variables(content);
	}

	std::optional<std::size_t> MetaTypeDescriptor::get_variable_index(MetaSymbolID name) const
	{
		for (std::size_t i = 0; i < field_names.size(); i++)
		{
			if (field_names[i] == name)
			{
				assert(field_values.size() > i);

				return i;
			}
		}

		return std::nullopt;
	}

	const MetaAny* MetaTypeDescriptor::get_variable(MetaSymbolID name) const
	{
		if (auto index = get_variable_index(name))
		{
			return &(field_values[*index]);
		}

		return nullptr;
	}

	MetaAny* MetaTypeDescriptor::get_variable(MetaSymbolID name)
	{
		return const_cast<MetaAny*>
		(
			const_cast<const MetaTypeDescriptor*>(this)->get_variable(name)
		);
	}

	MetaAny* MetaTypeDescriptor::set_variable(MetaVariable&& variable, bool safe)
	{
		auto existing = get_variable(variable.name);

		if (existing)
		{
			if (!safe || variable.value)
			{
				*existing = std::move(variable.value);

				return existing;
			}

			return nullptr;
		}

		if (!safe || variable.value)
		{
			field_names.emplace_back(variable.name);

			auto& value_out = field_values.emplace_back(std::move(variable.value));

			return &value_out;
		}

		return nullptr;
	}

	void MetaTypeDescriptor::set_variables(MetaTypeDescriptor&& variables, bool override_constructor_input_size)
	{
		for (std::size_t i = 0; i < variables.size(); i++)
		{
			const auto& field_name = variables.field_names[i];
			auto& field_value = variables.field_values[i];

			// TODO: Implement overload of `set_variable` that directly takes in field values.
			set_variable(MetaVariable(field_name, std::move(field_value)));
		}

		if (override_constructor_input_size)
		{
			if (variables.constructor_argument_count)
			{
				constructor_argument_count = variables.constructor_argument_count;
			}
		}
	}

	std::size_t MetaTypeDescriptor::set_variables(const util::json& content, std::size_t argument_offset)
	{
		std::size_t count = 0;
		std::size_t variable_index = argument_offset;

		for (const auto& var_proxy : content.items())
		{
			const auto& var_decl = var_proxy.key();
			const auto& var_value = var_proxy.value();

			auto [var_name, var_type_spec] = parse_variable_declaration(var_decl);

			// TODO: Change to `std::optional<MetaSymbolID>`, where `std::nullopt` in case of `content.is_array()`.
			MetaSymbolID var_name_hash = hash(var_name);
			//process_value(var_name, var_value);

			auto resolve_data_entry = [this, &content, &variable_index, &var_value, &var_name_hash]() mutable -> entt::meta_data
			{
				/*
				// TODO: Determine if there's any benefit to keeping this check:
				if (var_value.is_primitive())
				{
					return {};
				}
				*/

				// NOTE: May make sense to forego this check in the case of array inputs.
				// (Leaving this as-is, for now)
				if (auto data_entry = type.data(var_name_hash))
				{
					return data_entry;
				}

				if (content.is_array() || content.is_string())
				{
					if (auto data_entry = get_data_member_by_index(type, variable_index, true))
					{
						// Re-assign the variable name/hash to reflect the newly resolved entry.
						var_name_hash = data_entry->first;

						// Return the newly resolved data entry.
						return data_entry->second;
					}
				}

				return {};
			};

			auto resolve_variable_type = [&var_type_spec, &resolve_data_entry]() -> MetaType
			{
				if (var_type_spec.empty())
				{
					if (auto data_entry = resolve_data_entry())
					{
						return data_entry.type();
					}
				}
				else
				{
					return resolve(hash(var_type_spec));
				}

				return {};
			};

			// NOTE: In the case of `resolve_meta_any` and related functions,
			// this is where recursion may take place; caused by use of nested objects. (see `MetaVariable`)
			auto resolve_meta_variable = [&resolve_variable_type, &var_name_hash, &var_value]()
			{
				if (auto variable_type = resolve_variable_type())
				{
					return MetaVariable(var_name_hash, var_value, variable_type);
				}

				return MetaVariable(var_name_hash, var_value);
			};

			if (auto meta_var = resolve_meta_variable(); meta_var.value)
			{
				if (set_variable(std::move(meta_var)))
				{
					count++;
				}
			}

			variable_index++;
		}

		return count;
	}

	std::size_t MetaTypeDescriptor::set_variables
	(
		std::string_view content,

		std::string_view arg_separator,
		bool resolve_values,
		std::size_t argument_offset,

		bool safe
	)
	{
		std::size_t count = 0;
		std::size_t variable_index = argument_offset;

		util::split
		(
			content,
			arg_separator,

			[this, resolve_values, safe, &variable_index, &count](std::string_view argument, bool is_last_argument=false)
			{
				if (auto data_entry = get_data_member_by_index(type, variable_index, true))
				{
					const auto var_name_hash = data_entry->first;

					if (set_variable(MetaVariable(var_name_hash, meta_any_from_string(argument, resolve_values)), safe))
					{
						count++;
					}
				}

				variable_index++;
			}
		);

		return count;
	}


	bool MetaTypeDescriptor::has_nested_descriptor(std::size_t n_values) const
	{
		auto self_type = get_self_type();

		std::size_t idx = 0;

		for (const auto& entry : field_values)
		{
			if (idx >= n_values)
			{
				break;
			}

			if (entry.type() == self_type)
			{
				return true;
			}

			idx++;
		}

		return false;
	}

	bool MetaTypeDescriptor::has_nested_descriptor() const
	{
		auto self_type = get_self_type();

		for (const auto& entry : field_values)
		{
			if (entry.type() == self_type)
			{
				return true;
			}
		}

		return false;
	}

	MetaAny MetaTypeDescriptor::instance(bool allow_recursion) const
	{
		MetaAny instance;

		if (flags.allow_forwarding_fields_to_constructor)
		{
			instance = instance_exact(allow_recursion);
		}

		bool is_default_constructed = false;

		if (!instance)
		{
			if (flags.allow_default_construction)
			{
				instance = instance_default();

				is_default_constructed = true;
			}

			if (!instance)
			{
				print_warn("Unable to resolve constructor for component: \"#{}\"", type.id());

				return {};
			}
		}

		if (is_default_constructed || flags.force_field_assignment)
		{
			apply_fields(instance);
		}

		return instance;
	}

	MetaAny MetaTypeDescriptor::instance_exact(bool allow_recursion) const
	{
		auto constructor_args_count = field_values.size();

		if (constructor_argument_count)
		{
			constructor_args_count = std::min
			(
				constructor_args_count,
				static_cast<std::size_t>(*constructor_argument_count)
			);
		}

		if (constructor_args_count == 0) // <=
		{
			return {};
		}

		if (allow_recursion)
		{
			// NOTE: In the event of a nested descriptor without `allow_recursion`, we assume it is
			// correct to pass the nested `MetaTypeDescriptor` object to the underlying type's constructor.
			if (has_nested_descriptor(constructor_args_count))
			{
				// TODO: Look into whether there's any room to optimize via
				// use of 'handles' rather than full `entt::meta_any` instances.
				MetaTypeDescriptor::Values rebuilt_constructor_values;

				rebuilt_constructor_values.reserve(constructor_args_count);

				//rebuilt_constructor_values = field_values;

				auto self_type = get_self_type();

				//for (auto& entry : field_values) // const auto&
				for (std::size_t i = 0; i < constructor_args_count; i++)
				{
					auto& entry = field_values[i]; // const auto&

					if (entry.type() == self_type)
					{
						auto instance_fn = entry.type().func(hash("instance"));

						assert(instance_fn);

						// NOTE: Recursion.
						auto nested_instance = instance_fn.invoke(entry);

						assert(nested_instance);

						rebuilt_constructor_values.emplace_back(std::move(nested_instance));
					}
					else
					{
						// NOTE: Cannot perform a move operation here due to possible side effects.
						// TODO: Look into real-world performance implications of this limitation.
						rebuilt_constructor_values.emplace_back(entry);
					}
				}

				auto constructor_args = rebuilt_constructor_values.data();
				//auto constructor_args_count = rebuilt_constructor_values.size();

				return type.construct(constructor_args, constructor_args_count);
			}
		}

		// NOTE: We const-cast here due to possible side effects being part of entt's interface.
		// 
		// TODO: Look into whether side effects (e.g. moves, lvalue-refs, etc.)
		// are something we should be concerned with in practice.
		auto constructor_args = const_cast<MetaAny*>(field_values.data());

		return type.construct(constructor_args, constructor_args_count);
	}

	MetaAny MetaTypeDescriptor::instance_default() const
	{
		return type.construct();
	}

	std::size_t MetaTypeDescriptor::apply_fields(MetaAny& instance, std::size_t field_count, std::size_t offset) const
	{
		std::size_t fields_applied = 0;

		for (std::size_t i = offset; i < (offset+field_count); i++)
		{
			const auto& field_name = field_names[i];
			const auto& field_value = field_values[i];

			if (!field_value)
			{
				print_warn("Unable to resolve field \"#{}\", at index #{}, in component: \"{}\"", field_name, i, type.id());

				continue;
			}

			auto set_field = [&]() -> bool
			{
				auto meta_field = type.data(field_name);

				if (!meta_field)
				{
					return false;
				}

				auto result = meta_field.set(instance, field_value);

				return static_cast<bool>(result);
			};

			if (set_field())
			{
				fields_applied++;
			}
			else
			{
				print_warn("Failed to assign field: \"#{}\", in component: \"#{}\"", field_name, type.id());
			}
		}

		return fields_applied;
	}

	std::size_t MetaTypeDescriptor::apply_fields(MetaAny& instance) const
	{
		return apply_fields(instance, size(), 0);
	}
}