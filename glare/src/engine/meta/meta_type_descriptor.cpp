#include "meta_type_descriptor.hpp"

#include "meta_type_resolution_context.hpp"
#include "meta_value_operation.hpp"
#include "meta_evaluation_context.hpp"

#include "serial.hpp"
#include "hash.hpp"
#include "string.hpp"
#include "function.hpp"
#include "indirection.hpp"
#include "data_member.hpp"
#include "container.hpp"

#include <engine/entity/entity_target.hpp>

#include <string_view>
#include <string>

#include <util/string.hpp>
#include <util/parse.hpp>

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	MetaType MetaTypeDescriptor::get_self_type()
	{
		return resolve<MetaTypeDescriptor>();
	}

	// TODO: Look into whether we should use `try_get_underlying_type` here.
	MetaType MetaTypeDescriptor::resolve_type(const MetaType& type, const Flags& flags)
	{
		using namespace engine::literals;

		if (type)
		{
			if (flags.allow_type_aliasing)
			{
				if (auto real_type_fn = type.func("type_from_optional"_hs)) // "get_type"_hs
				{
					if (auto real_type_opaque = real_type_fn.invoke({}))
					{
						if (auto real_type = real_type_opaque.try_cast<MetaType>())
						{
							// NOTE: Recursion.
							if (auto real_type_out = resolve_type(*real_type, flags))
							{
								return real_type_out;
							}
						}
					}
				}
			}
		}

		return type;
	}

	MetaTypeID MetaTypeDescriptor::resolve_type_id(MetaTypeID type_id, const Flags& flags)
	{
		if (!type_id)
		{
			return type_id; // 0;
		}

		if (flags.allow_type_aliasing)
		{
			if (auto initial_type = engine::resolve(type_id))
			{
				if (auto resolved_type = resolve_type(initial_type, flags))
				{
					return resolved_type.id();
				}
			}
		}

		return type_id;
	}

	std::tuple<std::string_view, std::string_view, bool>
	MetaTypeDescriptor::parse_variable_declaration(std::string_view var_decl, std::string_view type_specifier_symbol)
	{
		const auto result = util::parse_key_expr_and_value_type(var_decl, type_specifier_symbol, false, true);

		return { std::get<0>(result), std::get<1>(result), std::get<3>(result) };
	}

	std::optional<std::pair<entt::id_type, entt::meta_data>>
	MetaTypeDescriptor::get_data_member_by_index(const entt::meta_type& type, std::size_t variable_index, bool recursive)
	{
		return engine::get_data_member_by_index(type, variable_index, recursive);
	}

	MetaTypeDescriptor::MetaTypeDescriptor(const MetaType& type, std::optional<SmallSize> constructor_argument_count, const MetaTypeDescriptorFlags& flags)
		: type_id(0), constructor_argument_count(constructor_argument_count), flags(flags) // type_id(resolve_type_id(type_id, flags))
	{
		set_type(type);
	}

	MetaTypeDescriptor::MetaTypeDescriptor(MetaTypeID type_id, std::optional<SmallSize> constructor_argument_count, const MetaTypeDescriptorFlags& flags)
		: type_id(0), constructor_argument_count(constructor_argument_count), flags(flags) // type_id(resolve_type_id(type_id, flags))
	{
		set_type_id(type_id);
	}

	MetaTypeDescriptor::MetaTypeDescriptor
	(
		const MetaType& type,

		std::string_view content,
		const MetaParsingInstructions& instructions,

		std::size_t argument_offset,

		std::optional<SmallSize> constructor_argument_count,
		const MetaTypeDescriptorFlags& flags,

		bool allow_nameless_fields
	) :
		type_id(0),
		constructor_argument_count(constructor_argument_count),
		flags(flags)
	{
		//assert(type);

		set_variables_impl
		(
			adopt_type(type),
			content, instructions,
			argument_offset,
			allow_nameless_fields
		);
	}

	MetaTypeDescriptor::MetaTypeDescriptor
	(
		const MetaType& type,

		const util::json& content,
		const MetaParsingInstructions& instructions,

		std::optional<SmallSize> constructor_argument_count,
		const MetaTypeDescriptorFlags& flags,
		bool allow_nameless_fields
	) :
		type_id(0),
		constructor_argument_count(constructor_argument_count),
		flags(flags)
	{
		//assert(type);

		set_variables_impl
		(
			adopt_type(type),
			content,
			instructions,
			0,
			allow_nameless_fields
		);
	}

	MetaType MetaTypeDescriptor::adopt_type(const MetaType& type)
	{
		using namespace engine::literals;

		if (auto resolved_type = resolve_type(type, this->flags))
		{
			this->type_id = resolved_type.id();

			const bool is_sequential_container  = resolved_type.is_sequence_container();
			const bool is_associative_container = resolved_type.is_associative_container();
			const bool is_wrapper_container     = meta_type_is_container_wrapper(type);

			this->flags.is_container             = (is_sequential_container || is_associative_container || is_wrapper_container);
			this->flags.is_sequential_container  = is_sequential_container;
			this->flags.is_associative_container = is_associative_container;
			this->flags.is_wrapper_container     = is_wrapper_container;

			return resolved_type;
		}

		return type;
	}

	bool MetaTypeDescriptor::set_type(const MetaType& type)
	{
		return static_cast<bool>(adopt_type(type));
	}

	bool MetaTypeDescriptor::set_type_id(MetaTypeID type_id)
	{
		if (type_id)
		{
			if (auto type = resolve(type_id))
			{
				return set_type(type);
			}
		}

		this->type_id = resolve_type_id(type_id, this->flags);

		return true; // static_cast<bool>(this->type_id);
	}

	MetaTypeID MetaTypeDescriptor::get_type_id() const
	{
		return type_id;
	}

	MetaType MetaTypeDescriptor::get_type() const
	{
		return resolve(type_id);
	}

	std::optional<std::size_t> MetaTypeDescriptor::get_variable_index(MetaSymbolID name) const
	{
		if (!name)
		{
			return std::nullopt;
		}

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

	MetaAny* MetaTypeDescriptor::set_variable(MetaVariable&& variable, bool safe, bool allow_nameless_fields)
	{
		if (!allow_nameless_fields)
		{
			if (!variable.name)
			{
				return nullptr;
			}
		}

		if (!this->flags.is_sequential_container)
		{
			if (auto existing = get_variable(variable.name))
			{
				if (!safe || variable.value)
				{
					*existing = std::move(variable.value);

					return existing;
				}

				return nullptr;
			}
		}

		if (!safe || variable.value)
		{
			if (!this->flags.is_sequential_container)
			{
				field_names.emplace_back(variable.name);
			}

			auto& value_out = field_values.emplace_back(std::move(variable.value));

			return &value_out;
		}

		return nullptr;
	}

	void MetaTypeDescriptor::set_variables(MetaTypeDescriptor&& variables, bool override_constructor_input_size)
	{
		for (std::size_t i = 0; i < variables.size(); i++)
		{
			const auto field_name = (variables.flags.is_sequential_container)
				? MetaSymbolID {}
				: variables.field_names[i]
			;

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
		const MetaParsingInstructions& instructions,
		std::size_t argument_offset,
		bool allow_nameless_fields
	)
	{
		return set_variables_impl
		(
			get_type(),
			content,
			instructions,
			argument_offset,
			allow_nameless_fields
		);
	}

	std::size_t MetaTypeDescriptor::set_variables_impl
	(
		const MetaType& type,
		const util::json& content,
		const MetaParsingInstructions& instructions,
		std::size_t argument_offset,
		bool allow_nameless_fields
	)
	{
		if (!type)
		{
			return 0;
		}

		if (this->flags.is_sequential_container)
		{
			allow_nameless_fields = true;
		}

		std::size_t count = 0;
		std::size_t variable_index = argument_offset;

		const auto opt_type_context = instructions.context.get_type_context();

		const auto var_name_expr_implied_type = MetaType {}; // resolve<std::string>();

		for (const auto& var_proxy : content.items())
		{
			const auto& var_decl = var_proxy.key();
			const auto& var_value = var_proxy.value();

			auto [var_name, var_type_spec, var_name_is_expression] = parse_variable_declaration(var_decl);

			auto var_name_hash = MetaSymbolID {};

			if ((!this->flags.is_sequential_container) && ((!this->flags.is_wrapper_container) || (!content.is_array())))
			{
				if (var_name_is_expression)
				{
					// NOTE: Parsing instructions purposefully not forwarded here.
					if (auto var_name_expr = meta_any_from_string(std::string_view { var_name }, {}, var_name_expr_implied_type, false))
					{
						if (auto var_name_resolved = try_get_underlying_value(var_name_expr))
						{
							var_name_expr = std::move(var_name_resolved);
						}

						if (auto var_name_expr_as_hash = meta_any_to_string_hash(var_name_expr, true, false, true))
						{
							var_name_hash = *var_name_expr_as_hash;
						}
					}
				}

				if (!var_name_hash)
				{
					var_name_hash = hash(var_name);
				}
			}
			
			auto resolve_data_entry = [this, &type, &content, &variable_index, &var_name_hash]() mutable -> entt::meta_data
			{
				if (this->flags.is_container)
				{
					return {};
				}

				/*
				// TODO: Determine if there's any benefit to keeping this check:
				if (var_value.is_primitive())
				{
					return {};
				}
				*/

				// NOTE: May make sense to forego this check in the case of array inputs.
				// (Leaving this as-is, for now)
				if (auto data_entry = resolve_data_member_by_id(type, true, var_name_hash))
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

			auto resolve_variable_type = [this, &type, &opt_type_context, &var_type_spec, &resolve_data_entry]() -> MetaType
			{
				if (this->flags.is_container)
				{
					// NOTE: Works for both associative and wrapper containers.
					// (Assuming a valid type resolution control-path is found; e.g. explicit member-function)
					if (auto pair_type = try_get_container_pair_type(type))
					{
						return pair_type;
					}

					// NOTE: Explicit check for sequence container due to possible
					// conflicts with associative and wrapper containers.
					if (type.is_sequence_container())
					{
						if (auto value_type = try_get_container_value_type(type))
						{
							return value_type;
						}
					}
				}
				else
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
						if (opt_type_context)
						{
							if (auto type = opt_type_context->get_type(var_type_spec))
							{
								return type;
							}
						}

						return resolve(hash(var_type_spec));
					}
				}

				return {};
			};

			// NOTE: In the case of `resolve_meta_any` and related functions,
			// this is where recursion may take place; caused by use of nested objects. (see `MetaVariable`)
			auto resolve_meta_variable = [allow_nameless_fields, &content, &instructions, &resolve_variable_type, &var_name, &var_name_hash, &var_value, &var_type_spec]() -> std::optional<MetaVariable>
			{
				if (auto variable_type = resolve_variable_type())
				{
					return MetaVariable(var_name_hash, var_value, variable_type, instructions);
				}
				else if (var_name.empty() || content.is_array() || content.is_string()) // || this->flags.is_container
				{
					if (allow_nameless_fields)
					{
						var_name_hash = {};
					}
					else
					{
						return std::nullopt;
					}
				}

				return MetaVariable(var_name_hash, var_value, instructions);
			};

			auto meta_var = resolve_meta_variable();

			if (!meta_var)
			{
				continue;
			}

			if (meta_var->value)
			{
				if (set_variable(std::move(*meta_var), true, allow_nameless_fields))
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
		const MetaParsingInstructions& instructions,

		std::size_t argument_offset,

		bool allow_nameless_fields
	)
	{
		return set_variables_impl
		(
			get_type(),
			content, instructions,
			argument_offset,
			allow_nameless_fields
		);
	}

	std::size_t MetaTypeDescriptor::set_variables_impl
	(
		const MetaType& type,
		std::string_view content,
		const MetaParsingInstructions& instructions,

		std::size_t argument_offset,

		bool allow_nameless_fields
	)
	{
		std::size_t count = 0;
		std::size_t variable_index = argument_offset;

		std::size_t content_position = 0;

		while (content_position < content.length())
		{
			auto data_entry = get_data_member_by_index(type, variable_index, true);

			const auto remaining_content = content.substr(content_position);

			if (data_entry)
			{
				const auto& var_name_id = data_entry->first;
				const auto& var_data = data_entry->second;

				// NOTE: We would normally pass `var_data.type()` here as the `type` argument, but doing so in this context would
				// imply that we're acting on an existing instance of that type. For this reason, we forgo providing type information,
				// and instead attempt to coerce the value during the instantiation phase. (see `instance` method, etc.)
				auto [result, length_processed] = meta_any_from_string_compound_expr_impl(remaining_content, instructions);

				if (result)
				{
					if (set_variable(MetaVariable(var_name_id, std::move(result))))
					{
						count++;

						content_position += length_processed;
					}
				}
			}
			else
			{
				if (!allow_nameless_fields)
				{
					break;
				}

				auto [result, length_processed] = meta_any_from_string_compound_expr_impl(remaining_content, instructions);

				if ((!result) || (length_processed == 0))
				{
					break;
				}

				const auto var_name_id = MetaSymbolID {}; // hash(std::to_string(variable_index)).value();

				if (set_variable(MetaVariable(var_name_id, std::move(result))))
				{
					count++;
				}
				
				content_position += length_processed;
			}

			variable_index++;
		}

		return count;
	}

	bool MetaTypeDescriptor::has_nested_descriptor(std::size_t n_values) const
	{
		using namespace engine::literals;

		auto self_type = get_self_type();

		return has_nested_type(self_type.id(), n_values);
	}

	bool MetaTypeDescriptor::has_nested_descriptor() const
	{
		using namespace engine::literals;

		auto self_type = get_self_type();

		return has_nested_type(self_type.id());
	}

	bool MetaTypeDescriptor::has_nested_member_reference(std::size_t n_values) const
	{
		using namespace engine::literals;

		return (has_nested_type("MetaDataMember"_hs, n_values) || has_nested_type("IndirectMetaDataMember"_hs, n_values));
	}

	bool MetaTypeDescriptor::has_nested_member_reference() const
	{
		using namespace engine::literals;

		return (has_nested_type("MetaDataMember"_hs) || has_nested_type("IndirectMetaDataMember"_hs));
	}

	bool MetaTypeDescriptor::has_nested_entity_target(std::size_t n_values) const
	{
		using namespace engine::literals;

		return has_nested_type("EntityTarget"_hs, n_values);
	}

	bool MetaTypeDescriptor::has_nested_entity_target() const
	{
		using namespace engine::literals;

		return has_nested_type("EntityTarget"_hs);
	}

	bool MetaTypeDescriptor::has_nested_value_operation(std::size_t n_values) const
	{
		using namespace engine::literals;

		return has_nested_type("MetaValueOperation"_hs, n_values);
	}

	bool MetaTypeDescriptor::has_nested_value_operation() const
	{
		using namespace engine::literals;

		return has_nested_type("MetaValueOperation"_hs);
	}

	bool MetaTypeDescriptor::has_nested_type(MetaTypeID type_id, std::size_t n_values) const
	{
		std::size_t idx = 0;

		for (const auto& entry : field_values)
		{
			if (idx >= n_values)
			{
				break;
			}

			const auto entry_type_id = entry.type().id();

			if (entry_type_id == type_id)
			{
				return true;
			}

			idx++;
		}

		return false;
	}

	bool MetaTypeDescriptor::has_nested_type(MetaTypeID type_id) const
	{
		for (const auto& entry : field_values)
		{
			const auto entry_type_id = entry.type().id();

			if (entry_type_id == type_id)
			{
				return true;
			}
		}

		return false;
	}

	bool MetaTypeDescriptor::has_indirection(std::size_t n_values, bool bypass_indirect_meta_any) const
	{
		std::size_t idx = 0;

		for (const auto& entry : field_values)
		{
			if (idx >= n_values)
			{
				break;
			}

			if (value_has_indirection(entry, bypass_indirect_meta_any))
			{
				return true;
			}

			idx++;
		}

		return false;
	}

	bool MetaTypeDescriptor::has_anonymous_field_name() const
	{
		for (const auto& entry_name : field_names)
		{
			if (!entry_name)
			{
				return true;
			}
		}

		return false;
	}

	bool MetaTypeDescriptor::has_indirection(bool bypass_indirect_meta_any) const
	{
		for (const auto& entry : field_values)
		{
			const auto entry_type_id = entry.type().id();

			if (value_has_indirection(entry, bypass_indirect_meta_any))
			{
				return true;
			}
		}

		return false;
	}

	MetaAny MetaTypeDescriptor::instance_default(const MetaType& type) const
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

	std::size_t MetaTypeDescriptor::apply_fields(MetaAny& instance, const MetaAny& source_value, std::size_t field_count, std::size_t offset) const
	{
		return apply_fields_impl(instance, field_count, offset, source_value);
	}

	std::size_t MetaTypeDescriptor::apply_fields(MetaAny& instance) const
	{
		return apply_fields_impl(instance, size(), 0);
	}

	std::size_t MetaTypeDescriptor::apply_fields(MetaAny& instance, const MetaAny& source_value) const
	{
		return apply_fields_impl(instance, size(), 0, source_value);
	}

	std::size_t MetaTypeDescriptor::apply_fields(MetaAny& instance, Registry& registry, Entity entity, std::size_t field_count, std::size_t offset) const
	{
		return apply_fields_impl(instance, field_count, offset, registry, entity);
	}

	std::size_t MetaTypeDescriptor::apply_fields(MetaAny& instance, const MetaAny& source_value, Registry& registry, Entity entity, std::size_t field_count, std::size_t offset) const
	{
		return apply_fields_impl(instance, field_count, offset, source_value, registry, entity);
	}

	std::size_t MetaTypeDescriptor::apply_fields(MetaAny& instance, Registry& registry, Entity entity, const MetaEvaluationContext& context, std::size_t field_count, std::size_t offset) const
	{
		return apply_fields_impl(instance, field_count, offset, registry, entity, context);
	}

	std::size_t MetaTypeDescriptor::apply_fields(MetaAny& instance, const MetaAny& source_value, Registry& registry, Entity entity, const MetaEvaluationContext& context, std::size_t field_count, std::size_t offset) const
	{
		return apply_fields_impl(instance, field_count, offset, source_value, registry, entity, context);
	}

	std::size_t MetaTypeDescriptor::apply_fields(MetaAny& instance, const MetaEvaluationContext& context, std::size_t field_count, std::size_t offset) const
	{
		return apply_fields_impl(instance, field_count, offset, context);
	}

	std::size_t MetaTypeDescriptor::apply_fields(MetaAny& instance, const MetaAny& source_value, const MetaEvaluationContext& context, std::size_t field_count, std::size_t offset) const
	{
		return apply_fields_impl(instance, field_count, offset, source_value, context);
	}

	std::size_t MetaTypeDescriptor::apply_fields(MetaAny& instance, const MetaEvaluationContext& context) const
	{
		return apply_fields_impl(instance, size(), 0, context);
	}

	std::size_t MetaTypeDescriptor::apply_fields(MetaAny& instance, const MetaAny& source_value, const MetaEvaluationContext& context) const
	{
		return apply_fields_impl(instance, size(), 0, source_value, context);
	}

	std::size_t MetaTypeDescriptor::apply_fields(MetaAny& instance, Registry& registry, Entity entity) const
	{
		return apply_fields_impl(instance, size(), 0, registry, entity);
	}

	std::size_t MetaTypeDescriptor::apply_fields(MetaAny& instance, const MetaAny& source_value, Registry& registry, Entity entity) const
	{
		return apply_fields_impl(instance, size(), 0, source_value, registry, entity);
	}

	std::size_t MetaTypeDescriptor::apply_fields(MetaAny& instance, Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		return apply_fields_impl(instance, size(), 0, registry, entity, context);
	}

	std::size_t MetaTypeDescriptor::apply_fields(MetaAny& instance, const MetaAny& source_value, Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		return apply_fields_impl(instance, size(), 0, source_value, registry, entity, context);
	}

	template <typename ...Args>
	std::size_t MetaTypeDescriptor::apply_fields_impl(MetaAny& instance, std::size_t field_count, std::size_t offset, Args&&... args) const
	{
		using namespace engine::literals;

		if (flags.is_container)
		{
			if (flags.is_sequential_container)
			{
				auto container_result = apply_fields_sequential_container_impl(instance, field_count, offset, std::forward<Args>(args)...);

				if (container_result || (!flags.is_wrapper_container))
				{
					return container_result;
				}
			}
			else if (flags.is_associative_container)
			{
				auto container_result = apply_fields_associative_container_impl(instance, field_count, offset, std::forward<Args>(args)...);

				if (container_result || (!flags.is_wrapper_container))
				{
					return container_result;
				}
			}
		}

		if (!instance)
		{
			return 0;
		}

		std::size_t fields_applied = 0;

		const auto check_indirection = has_indirection(); // has_indirection(field_count, offset);

		const auto type = instance.type(); // get_type();

		for (std::size_t i = offset; i < (offset + field_count); i++)
		{
			const auto& field_name  = field_names[i];
			const auto& field_value = field_values[i];

			if (!field_value)
			{
				print_warn("Unable to resolve field \"#{}\", at index #{}, in component: \"{}\"", field_name, i, type.id());

				continue;
			}

			auto set_field = [&]() -> bool // [this, &instance, check_indirection, &field_name, &field_value]
			{
				if (!field_name)
				{
					return false;
				}

				auto meta_field = resolve_data_member_by_id(type, true, field_name);

				if (!meta_field)
				{
					return false;
				}

				const auto meta_field_type = meta_field.type();

				auto set = [&type, &instance, &meta_field, &meta_field_type](const MetaAny& value) -> bool
				{
					if (auto result = meta_field.set(instance, value))
					{
						return result;
					}

					if (meta_field_type)
					{
						if (auto construct_from = meta_field_type.construct(value))
						{
							return meta_field.set(instance, std::move(construct_from));
						}

						if (MetaAny casted = value.allow_cast(meta_field_type))
						{
							return meta_field.set(instance, casted);
						}
					}

					if ((!meta_field_type) || (meta_field_type.id() != "Entity"_hs)) // entt::type_hash<Entity>::value()
					{
						// Fallbacks for string-to-hash conversion scenarios:
						if (const auto as_str_view = value.try_cast<std::string_view>())
						{
							return meta_field.set(instance, hash(*as_str_view).value());
						}

						if (const auto as_str = value.try_cast<std::string>())
						{
							return meta_field.set(instance, hash(*as_str).value());
						}
					}

					return false;
				};

				if (check_indirection)
				{
					const auto stored_field_type = field_value.type();

					if (stored_field_type != meta_field_type)
					{
						if (auto resolved_value = try_get_underlying_value(field_value, args...))
						{
							if (auto result = set(resolved_value))
							{
								return result;
							}
							else
							{
								// Debugging related.
								//print_warn("Failed to assign resolved value, attempting original value as fallback.");
								//get_indirect_value_or_ref(field_value, args...);
							}
						}
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
				//print_warn("Failed to assign field: \"#{}\", in component: \"#{}\"", field_name, type.id());
				
				// Debugging related:
				auto _dbg_field_name = get_known_string_from_hash(field_name);
				auto _dbg_type_name = get_known_string_from_hash(type.id());
				
				print_warn("Failed to assign field: \"{}\" (#{}), for type: \"{}\" (#{})", _dbg_field_name, field_name, _dbg_type_name, type.id());

				//auto test = try_get_underlying_value(field_value, args...);
			}
		}

		return fields_applied;
	}

	template <typename ...Args>
	std::size_t MetaTypeDescriptor::apply_fields_sequential_container_impl(MetaAny& instance, std::size_t field_count, std::size_t offset, Args&&... args) const
	{
		if (!instance)
		{
			return 0;
		}

		if (!flags.is_sequential_container)
		{
			return 0;
		}

		auto sequence_container = instance.as_sequence_container();

		if (!sequence_container)
		{
			return 0;
		}

		const auto container_value_type = sequence_container.value_type();

		const auto check_indirection = has_indirection(); // has_indirection(field_count, offset);

		// TODO: Add checks for successful use of `insert`.
		for (std::size_t i = offset; i < (offset + field_count); i++)
		{
			const auto& field_value = field_values[i];

			auto field_value_type = MetaType {};

			if (check_indirection) // (construction_flags.allow_indirection)
			{
				if (auto resolved_value = try_get_underlying_value(field_value, args...))
				{
					field_value_type = resolved_value.type();

					if (field_value_type != container_value_type)
					{
						// Attempt in-place conversion:
						if (!resolved_value.allow_cast(container_value_type))
						{
							continue;
						}
					}

					sequence_container.insert(sequence_container.end(), std::move(resolved_value));

					continue;
				}
			}

			field_value_type = field_value.type();

			if (field_value_type != container_value_type)
			{
				// Attempt conversion into new instance:
				if (auto converted_field_value = field_value.allow_cast(container_value_type))
				{
					sequence_container.insert(sequence_container.end(), std::move(converted_field_value));

					continue;
				}
			}

			sequence_container.insert(sequence_container.end(), field_value);
		}

		return field_count;
	}

	template <typename ...Args>
	std::size_t MetaTypeDescriptor::apply_fields_associative_container_impl(MetaAny& instance, std::size_t field_count, std::size_t offset, Args&&... args) const
	{
		if (!instance)
		{
			return 0;
		}

		if (!flags.is_associative_container)
		{
			return 0;
		}

		auto associative_container = instance.as_associative_container();

		if (!associative_container)
		{
			return 0;
		}

		const auto container_key_type = associative_container.key_type();

		const auto container_key_type_is_string = meta_type_is_string(container_key_type);

		if ((container_key_type.id() != entt::type_hash<MetaSymbolID>::value()) && (!container_key_type_is_string))
		{
			return 0;
		}

		const auto container_value_type = associative_container.mapped_type(); // value_type();

		const auto check_indirection = has_indirection(); // has_indirection(field_count, offset);

		// TODO: Add checks for successful use of `insert`.
		for (std::size_t i = offset; i < (offset + field_count); i++)
		{
			const auto& field_name_id  = field_names[i];
			const auto& field_value    = field_values[i];

			auto key = MetaAny {};

			if (container_key_type_is_string)
			{
				if (auto field_name = get_known_string_from_hash(field_name_id); !field_name.empty())
				{
					key = container_key_type.construct(field_name);
				}
			}
			else
			{
				key = field_name_id;
			}

			if (!key)
			{
				continue;
			}

			auto field_value_type = MetaType {};

			if (check_indirection) // (construction_flags.allow_indirection)
			{
				if (auto resolved_value = try_get_underlying_value(field_value, args...))
				{
					field_value_type = resolved_value.type();

					if (field_value_type != container_value_type)
					{
						// Attempt in-place conversion:
						if (!resolved_value.allow_cast(container_value_type))
						{
							continue;
						}
					}

					associative_container.insert(key, std::move(resolved_value));

					continue;
				}
			}

			field_value_type = field_value.type();

			if (field_value_type != container_value_type)
			{
				// Attempt conversion into new instance:
				if (auto converted_field_value = field_value.allow_cast(container_value_type))
				{
					associative_container.insert(key, std::move(converted_field_value));

					continue;
				}
			}

			associative_container.insert(key, field_value);
		}

		return field_count;
	}

	std::size_t MetaTypeDescriptor::resolve_values(Values& values_out, std::size_t constructor_args_count, std::size_t offset) const
	{
		return resolve_values_impl(values_out, constructor_args_count, offset);
	}

	std::size_t MetaTypeDescriptor::resolve_values(Values& values_out, std::size_t constructor_args_count, const MetaAny& source_value, std::size_t offset) const
	{
		return resolve_values_impl(values_out, constructor_args_count, offset, source_value);
	}

	std::size_t MetaTypeDescriptor::resolve_values(Values& values_out, std::size_t constructor_args_count, const MetaEvaluationContext& context, std::size_t offset) const
	{
		return resolve_values_impl(values_out, constructor_args_count, offset, context);
	}

	std::size_t MetaTypeDescriptor::resolve_values(Values& values_out, std::size_t constructor_args_count, const MetaAny& source_value, const MetaEvaluationContext& context, std::size_t offset) const
	{
		return resolve_values_impl(values_out, constructor_args_count, offset, source_value, context);
	}

	std::size_t MetaTypeDescriptor::resolve_values(Values& values_out, std::size_t constructor_args_count, Registry& registry, Entity entity, std::size_t offset) const
	{
		return resolve_values_impl(values_out, constructor_args_count, offset, registry, entity);
	}

	std::size_t MetaTypeDescriptor::resolve_values(Values& values_out, std::size_t constructor_args_count, const MetaAny& source_value, Registry& registry, Entity entity, std::size_t offset) const
	{
		return resolve_values_impl(values_out, constructor_args_count, offset, source_value, registry, entity);
	}

	std::size_t MetaTypeDescriptor::resolve_values(Values& values_out, std::size_t constructor_args_count, Registry& registry, Entity entity, const MetaEvaluationContext& context, std::size_t offset) const
	{
		return resolve_values_impl(values_out, constructor_args_count, offset, registry, entity, context);
	}

	std::size_t MetaTypeDescriptor::resolve_values(Values& values_out, std::size_t constructor_args_count, const MetaAny& source_value, Registry& registry, Entity entity, const MetaEvaluationContext& context, std::size_t offset) const
	{
		return resolve_values_impl(values_out, constructor_args_count, offset, source_value, registry, entity, context);
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
				if (auto indirect_value = get_indirect_value_or_ref(entry, std::forward<Args>(args)...))
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

	std::size_t MetaTypeDescriptor::populate_sequential_container_raw(MetaAny& instance, const MetaAny* field_values, std::size_t field_count)
	{
		if (!instance)
		{
			return 0;
		}

		auto sequence_container = instance.as_sequence_container();

		if (!sequence_container)
		{
			return 0;
		}

		return populate_sequential_container_raw(sequence_container, field_values, field_count);
	}

	std::size_t MetaTypeDescriptor::populate_sequential_container_raw(entt::meta_sequence_container& sequence_container, const MetaAny* field_values, std::size_t field_count)
	{
		if (!field_values)
		{
			return 0;
		}

		const auto container_value_type = sequence_container.value_type();

		// TODO: Add checks for successful use of `insert`.
		for (std::size_t i = 0; i < field_count; i++)
		{
			const auto& field_value = field_values[i];

			const auto field_value_type = field_value.type();

			if (field_value_type != container_value_type)
			{
				// Attempt conversion into new instance:
				if (auto converted_field_value = field_value.allow_cast(container_value_type))
				{
					sequence_container.insert(sequence_container.end(), std::move(converted_field_value));

					continue;
				}
			}

			sequence_container.insert(sequence_container.end(), field_value);
		}

		return field_count;
	}

	std::size_t MetaTypeDescriptor::populate_associative_container_raw(MetaAny& instance, const MetaSymbolID* field_names, const MetaAny* field_values, std::size_t field_count)
	{
		if (!instance)
		{
			return 0;
		}

		auto associative_container = instance.as_associative_container();

		if (!associative_container)
		{
			return 0;
		}

		return populate_associative_container_raw(associative_container, field_names, field_values, field_count);
	}

	std::size_t MetaTypeDescriptor::populate_associative_container_raw(entt::meta_associative_container& associative_container, const MetaSymbolID* field_names, const MetaAny* field_values, std::size_t field_count)
	{
		if (!field_names)
		{
			return 0;
		}

		if (!field_values)
		{
			return 0;
		}

		const auto container_key_type = associative_container.key_type();

		const auto container_key_type_is_string = meta_type_is_string(container_key_type);

		if ((container_key_type.id() != entt::type_hash<MetaSymbolID>::value()) && (!container_key_type_is_string))
		{
			return 0;
		}

		const auto container_value_type = associative_container.mapped_type(); // value_type();

		// TODO: Add checks for successful use of `insert`.
		for (std::size_t i = 0; i < field_count; i++)
		{
			const auto& field_name_id  = field_names[i];
			const auto& field_value    = field_values[i];

			auto key = MetaAny {};

			if (container_key_type_is_string)
			{
				if (auto field_name = get_known_string_from_hash(field_name_id); !field_name.empty())
				{
					key = container_key_type.construct(field_name);
				}
			}
			else
			{
				key = field_name_id;
			}

			if (!key)
			{
				continue;
			}

			const auto field_value_type = field_value.type();

			if (field_value_type != container_value_type)
			{
				// Attempt conversion into new instance:
				if (auto converted_field_value = field_value.allow_cast(container_value_type))
				{
					associative_container.insert(key, std::move(converted_field_value));

					continue;
				}
			}

			associative_container.insert(key, field_value);
		}

		return field_count;
	}

	MetaTypeDescriptor load_descriptor
	(
		const MetaType& type,

		const std::filesystem::path& path,
		
		const MetaParsingInstructions& parse_instructions,
		const MetaTypeDescriptorFlags& descriptor_flags
	)
	{
		return load_descriptor
		(
			type,

			util::load_json(path),

			parse_instructions,
			descriptor_flags
		);
	}

	MetaTypeDescriptor load_descriptor
	(
		const MetaType& type,

		const util::json& content,

		const MetaParsingInstructions& parse_instructions,
		const MetaTypeDescriptorFlags& descriptor_flags
	)
	{
		return MetaTypeDescriptor
		(
			type,
			
			content,
			
			parse_instructions,
			std::nullopt,
			descriptor_flags
		);
	}
}