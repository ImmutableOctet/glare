#include "meta_type_descriptor.hpp"
#include "meta.hpp"
#include "serial.hpp"
#include "parsing_context.hpp"

#include <string_view>
#include <string>

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

		if ((symbol_position == std::string_view::npos) || (symbol_position == (var_decl.length()-type_specifier_symbol.length())))
		{
			return { var_decl, {} };
		}

		auto var_name = var_decl.substr(0, symbol_position);
    
		auto var_type = var_decl.substr
		(
			(symbol_position + type_specifier_symbol.length()),
			(var_decl.length() - symbol_position)
		);

		var_name = util::trim(var_name);
		var_type = util::trim(var_type);

		return { var_name, var_type };
	}

	std::optional<std::pair<entt::id_type, entt::meta_data>>
	MetaTypeDescriptor::get_data_member_by_index(const entt::meta_type& type, std::size_t variable_index, bool recursive)
	{
		return engine::get_data_member_by_index(type, variable_index, recursive);
	}

	MetaTypeDescriptor::MetaTypeDescriptor(MetaType type, std::optional<SmallSize> constructor_argument_count, const MetaTypeDescriptorFlags& flags)
		: type_id(type.id()), constructor_argument_count(constructor_argument_count), flags(flags) {}

	MetaTypeDescriptor::MetaTypeDescriptor(MetaTypeID type_id, std::optional<SmallSize> constructor_argument_count, const MetaTypeDescriptorFlags& flags)
		: type_id(type_id), constructor_argument_count(constructor_argument_count), flags(flags) {}

	MetaTypeDescriptor::MetaTypeDescriptor
	(
		MetaType type,

		std::string_view content,
		const MetaAnyParseInstructions& instructions,

		std::string_view arg_separator,
		std::size_t argument_offset,

		std::optional<SmallSize> constructor_argument_count,
		const MetaTypeDescriptorFlags& flags
	) :
		type_id(type.id()),
		constructor_argument_count(constructor_argument_count),
		flags(flags)
	{
		assert(type);

		set_variables(content, instructions, arg_separator, argument_offset, true); // false
	}

	MetaTypeDescriptor::MetaTypeDescriptor
	(
		MetaType type,

		const util::json& content,
		const MetaAnyParseInstructions& instructions,

		std::optional<SmallSize> constructor_argument_count,
		const MetaTypeDescriptorFlags& flags,
		const ParsingContext* opt_parsing_context
	) :
		type_id(type.id()),
		constructor_argument_count(constructor_argument_count),
		flags(flags)
	{
		assert(type);

		set_variables(content, instructions, 0, opt_parsing_context);
	}

	MetaType MetaTypeDescriptor::get_type() const
	{
		return resolve(type_id);
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

	std::size_t MetaTypeDescriptor::set_variables
	(
		const util::json& content,
		const MetaAnyParseInstructions& instructions,
		std::size_t argument_offset,
		const ParsingContext* opt_parsing_context
	)
	{
		return set_variables_impl
		(
			get_type(),
			content,
			instructions,
			argument_offset,
			opt_parsing_context
		);
	}

	std::size_t MetaTypeDescriptor::set_variables_impl
	(
		const MetaType& type,
		const util::json& content,
		const MetaAnyParseInstructions& instructions,
		std::size_t argument_offset,
		const ParsingContext* opt_parsing_context
	)
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
			
			auto resolve_data_entry = [this, &type, &content, &variable_index, &var_name_hash]() mutable -> entt::meta_data
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

			auto resolve_variable_type = [&opt_parsing_context, &var_type_spec, &resolve_data_entry]() -> MetaType
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
					if (opt_parsing_context)
					{
						if (auto type = opt_parsing_context->get_type(var_type_spec))
						{
							return type;
						}
					}

					return resolve(hash(var_type_spec));
				}

				return {};
			};

			// NOTE: In the case of `resolve_meta_any` and related functions,
			// this is where recursion may take place; caused by use of nested objects. (see `MetaVariable`)
			auto resolve_meta_variable = [&instructions, &opt_parsing_context, &resolve_variable_type, &var_name_hash, &var_value]()
			{
				if (auto variable_type = resolve_variable_type())
				{
					return MetaVariable(var_name_hash, var_value, variable_type, instructions, opt_parsing_context);
				}

				return MetaVariable(var_name_hash, var_value, instructions, opt_parsing_context);
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
		const MetaAnyParseInstructions& instructions,

		std::string_view arg_separator,
		std::size_t argument_offset,

		bool safe
	)
	{
		return set_variables_impl
		(
			get_type(),
			content, instructions,
			arg_separator, argument_offset,
			safe
		);
	}

	std::size_t MetaTypeDescriptor::set_variables_impl
	(
		const MetaType& type,
		std::string_view content,
		const MetaAnyParseInstructions& instructions,

		std::string_view arg_separator,
		std::size_t argument_offset,

		bool safe
	)
	{
		std::size_t count = 0;
		std::size_t variable_index = argument_offset;

		auto on_argument = [this, &type, &instructions, safe, &variable_index, &count](std::string_view argument, bool is_last_argument=false)
		{
			if (auto data_entry = get_data_member_by_index(type, variable_index, true))
			{
				const auto var_name_hash = data_entry->first;

				if (set_variable(MetaVariable(var_name_hash, meta_any_from_string(argument, instructions)), safe))
				{
					count++;
				}
			}

			variable_index++;
		};

		if (arg_separator.empty())
		{
			on_argument(content, true);
		}
		else
		{
			util::split
			(
				content,
				arg_separator,

				on_argument
			);
		}

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

	bool MetaTypeDescriptor::has_nested_member_reference(std::size_t n_values) const
	{
		using namespace entt::literals;

		std::size_t idx = 0;

		for (const auto& entry : field_values)
		{
			if (idx >= n_values)
			{
				break;
			}

			const auto type_id = entry.type().id();

			if ((type_id == "MetaDataMember"_hs) || (type_id == "IndirectMetaDataMember"_hs))
			{
				return true;
			}

			idx++;
		}

		return false;
	}

	bool MetaTypeDescriptor::has_nested_member_reference() const
	{
		using namespace entt::literals;

		for (const auto& entry : field_values)
		{
			const auto type_id = entry.type().id();

			if ((type_id == "MetaDataMember"_hs) || (type_id == "IndirectMetaDataMember"_hs))
			{
				return true;
			}
		}

		return false;
	}

	MetaAny MetaTypeDescriptor::instance_default(MetaType type) const
	{
		if (!type)
		{
			return {};
		}

		return type.construct();
	}

	std::size_t MetaTypeDescriptor::apply_fields(MetaAny& instance, std::size_t field_count, std::size_t offset) const
	{
		return apply_fields_impl(instance, field_count, offset);
	}

	std::size_t MetaTypeDescriptor::apply_fields(MetaAny& instance) const
	{
		return apply_fields(instance, size());
	}

	// Updates each field of `instance`, defined by this descriptor.
	// 
	// This overload uses `registry` and `entity` to resolve source-indirection
	// from the `field_values` container. (e.g. References to component member)
	std::size_t MetaTypeDescriptor::apply_fields(MetaAny& instance, std::size_t field_count, Registry& registry, Entity entity, std::size_t offset) const
	{
		return apply_fields_impl(instance, field_count, offset, registry, entity);
	}

	std::size_t MetaTypeDescriptor::apply_fields(MetaAny& instance, Registry& registry, Entity entity) const
	{
		return apply_fields_impl(instance, size(), 0, registry, entity);
	}

	template <typename ...Args>
	std::size_t MetaTypeDescriptor::apply_fields_impl(MetaAny& instance, std::size_t field_count, std::size_t offset, Args&&... args) const
	{
		std::size_t fields_applied = 0;

		const auto check_indirection = has_indirection(); // has_indirection(field_count, offset);

		const auto type = instance.type(); // get_type();

		for (std::size_t i = offset; i < (offset+field_count); i++)
		{
			const auto& field_name = field_names[i];
			const auto& field_value = field_values[i];

			if (!field_value)
			{
				print_warn("Unable to resolve field \"#{}\", at index #{}, in component: \"{}\"", field_name, i, type.id());

				continue;
			}

			auto set_field = [&]() -> bool // [this, &instance, check_indirection, &field_name, &field_value]
			{
				auto meta_field = type.data(field_name);

				if (!meta_field)
				{
					return false;
				}

				auto set = [&type, &instance, &meta_field](const MetaAny& value) -> bool
				{
					if (auto result = meta_field.set(instance, value))
					{
						return result;
					}

					/*
					if (auto casted = value.allow_cast(meta_field.type()))
					{
						return meta_field.set(instance, casted);
					}
					*/

					if (auto construct_from = meta_field.type().construct(value))
					{
						return meta_field.set(instance, std::move(construct_from));
					}

					// Fallbacks for string-to-hash conversion scenarios:
					if (const auto as_str_view = value.try_cast<std::string_view>())
					{
						return meta_field.set(instance, hash(*as_str_view).value());
					}

					if (const auto as_str = value.try_cast<std::string>())
					{
						return meta_field.set(instance, hash(*as_str).value());
					}

					return false;
				};

				if (check_indirection)
				{
					if (auto result = resolve_indirect_value(field_value, std::forward<Args>(args)...))
					{
						return set(result);
					}
				}

				return set(field_value);
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

	std::size_t MetaTypeDescriptor::resolve_values(Values& values_out, std::size_t constructor_args_count, std::size_t offset) const
	{
		return resolve_values_impl(values_out, constructor_args_count, offset);
	}

	std::size_t MetaTypeDescriptor::resolve_values(Values& values_out, std::size_t constructor_args_count, Registry& registry, Entity entity, std::size_t offset) const
	{
		return resolve_values_impl(values_out, constructor_args_count, offset, registry, entity);
	}

	template <typename ...Args>
	std::size_t MetaTypeDescriptor::resolve_values_impl(Values& values_out, std::size_t constructor_args_count, std::size_t offset, Args&&... args) const
	{
		std::size_t count = 0;

		const auto check_indirection = has_indirection(); // has_indirection(constructor_args_count, offset);

		//for (auto& entry : field_values) // const auto&
		for (std::size_t i = offset; i < (offset + constructor_args_count); i++)
		{
			auto& entry = field_values[i]; // const auto&

			count++;

			if (check_indirection)
			{
				if (auto indirect_value = resolve_indirect_value(entry, std::forward<Args>(args)...))
				{
					values_out.emplace_back(std::move(indirect_value));

					continue;
				}
			}

			// NOTE: Cannot perform a move operation here due to possible side effects.
			// TODO: Look into real-world performance implications of this limitation.
			values_out.emplace_back(entry);
		}

		return count;
	}

	MetaAny MetaTypeDescriptor::resolve_indirect_value(const MetaAny& entry)
	{
		using namespace entt::literals;

		switch (entry.type().id())
		{
			case "MetaTypeDescriptor"_hs: // self_type.id():
			{
				auto instance_fn = entry.type().func("instance"_hs);

				assert(instance_fn);

				// NOTE: Recursion.
				auto nested_instance = instance_fn.invoke(entry);

				//assert(nested_instance);

				return nested_instance;
			}
		}

		return {};
	}

	MetaAny MetaTypeDescriptor::resolve_indirect_value(const MetaAny& entry, Registry& registry, Entity entity)
	{
		using namespace entt::literals;

		if (auto result = resolve_indirect_value(entry))
		{
			return result;
		}

		switch (entry.type().id())
		{
			case "MetaDataMember"_hs:
			{
				if (const auto* data_member = entry.try_cast<MetaDataMember>())
				{
					return data_member->get(registry, entity);
				}

				break;
			}
			case "IndirectMetaDataMember"_hs:
			{
				if (const auto* data_member = entry.try_cast<IndirectMetaDataMember>())
				{
					return data_member->get(registry, entity);
				}

				break;
			}
		}

		return {};
	}
}