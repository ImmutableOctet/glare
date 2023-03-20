#pragma once

#include "types.hpp"
#include "meta_parsing_instructions.hpp"
#include "meta_data_member.hpp"
#include "indirect_meta_data_member.hpp"
#include "meta_type_descriptor_flags.hpp"
#include "meta_variable_scope.hpp"

#include <engine/entity/entity_target.hpp>

// TODO: Forward declare `util::json`, etc.
#include <util/json.hpp>

#include <util/format.hpp>

#include <entt/meta/meta.hpp>
//#include <entt/entt.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <filesystem>
#include <stdexcept>
#include <tuple>

namespace engine
{
	// JSON-shorthand overload for string-to-any resolution function.
	MetaAny meta_any_from_string(const util::json& value, const MetaParsingInstructions& instructions={}, MetaType type={}, bool allow_string_fallback=true, bool allow_numeric_literals=true, bool allow_boolean_literals=true);

	// Attempts to resolve a native value from a raw string value, using reflection.
	MetaAny meta_any_from_string
	(
		std::string_view value,
		
		const MetaParsingInstructions& instructions={},
		
		MetaType type={},

		bool allow_string_fallback=true,
		bool allow_numeric_literals=true,
		bool allow_boolean_literals=true,
		bool allow_entity_fallback=true,
		bool allow_component_fallback=true
	);

	// Internal subroutine of `meta_any_from_string`.
	std::tuple
	<
		MetaAny,    // result
		std::size_t // length_processed
	>
	meta_any_from_string_compound_expr_impl
	(
		std::string_view expr,
		
		const MetaParsingInstructions& instructions,
		MetaType type={},
		
		bool try_arithmetic=true,
		bool try_string_fallback=true,
		bool try_boolean=true,
		bool allow_entity_fallback=true,
		bool allow_component_fallback=true,

		bool assume_static_type=false
	);

	// Attempts to resolve the value indicated by `string_reference` as a string.
	// 
	// If the value is a string, `string_callback` will be executed.
	// If the value is not a string, `non_string_callback` will be called instead.
	// 
	// NOTE: Although the return value of `string_callback` is ignored, the return-value of
	// `non_string_callback` is used to determine if the `MetaAny` instance
	// retrieved should be returned back to the initial caller.
	template <typename StringCallback, typename NonStringCallback>
	inline MetaAny peek_string_value(std::string_view string_reference, StringCallback&& string_callback, NonStringCallback&& non_string_callback, const MetaParsingInstructions& instructions={}, MetaType type={}) // { .fallback_to_string=true }
	{
		const auto resolved_value = meta_any_from_string(string_reference, instructions, type);

		if (!resolved_value)
		{
			return {};
		}

		if (try_string_value(resolved_value, string_callback))
		{
			if constexpr (std::is_invocable_r_v<bool, NonStringCallback, decltype(resolved_value)>)
			{
				if (!non_string_callback(resolved_value))
				{
					return {};
				}
			}
			else
			{
				non_string_callback(resolved_value);
			}
		}

		return resolved_value;
	}

	// Convenience overload for `peek_string_value` without the need to specify a non-string callback.
	template <typename Callback>
	inline bool peek_string_value(std::string_view string_reference, Callback&& callback, const MetaParsingInstructions& instructions={}) // { .fallback_to_string=true }
	{
		auto result = peek_string_value
		(
			string_reference,
			std::forward<Callback>(callback),
			
			[](const MetaAny& non_string_value)
			{
				return false;
			},

			instructions
		);

		return static_cast<bool>(result);
	}

	MetaAny resolve_meta_any
	(
		const util::json& value,
		MetaTypeID type_id,
		const MetaParsingInstructions& instructions={}
	);

	MetaAny resolve_meta_any
	(
		const util::json& value,
		MetaType type,
		const MetaParsingInstructions& instructions={}
	);

	// NOTE: This overload cannot handle non-primitive values.
	// (see overload taking a native-type identifier)
	MetaAny resolve_meta_any
	(
		const util::json& value,
		const MetaParsingInstructions& instructions={}
	);

	// Attempts to resolve a `MetaDataMember` from `value`.
	// NOTE: Does not support indirection. (i.e. other entities from the initial source)
	std::optional<MetaDataMember> meta_data_member_from_string(std::string_view value);

	// Attempts to resolve `type_name` and `data_member_name`, then returns a
	// `MetaDataMember` instance using the corresponding reflection data.
	std::optional<MetaDataMember> process_meta_data_member(std::string_view type_name, std::string_view data_member_name);

	// Attempts to resolve an `IndirectMetaDataMember` from `value`.
	std::optional<IndirectMetaDataMember> indirect_meta_data_member_from_string(std::string_view value, EntityTarget target = { EntityTarget::SelfTarget {} });

	EntityTarget::TargetType parse_target_type(const util::json& target_data);

	std::optional<MetaVariableScope> parse_variable_scope(std::string_view scope_qualifier);

	void read_type_context(MetaTypeResolutionContext& context, const util::json& data);

	MetaAny load
	(
		MetaAny out,
		
		const util::json& data,
		bool use_assignment=true,

		const MetaParsingInstructions& parse_instructions={},
		const MetaTypeDescriptorFlags& descriptor_flags={}
	);

	MetaAny load
	(
		MetaType type,
		const util::json& data,

		const MetaParsingInstructions& parse_instructions={},
		const MetaTypeDescriptorFlags& descriptor_flags={}
	);

	template <typename T>
	void load
	(
		T& out,
		
		const util::json& data,
		bool use_assignment=true,

		const MetaParsingInstructions& parse_instructions={},
		const MetaTypeDescriptorFlags& descriptor_flags={},

		bool fallback_to_default_construction=std::is_default_constructible_v<T>
	)
	{
		auto instance = load
		(
			entt::forward_as_meta(out),
			
			data,

			use_assignment,

			parse_instructions,
			descriptor_flags
		);

		if (use_assignment)
		{
			return;
		}

		if (!instance)
		{
			if (fallback_to_default_construction)
			{
				if constexpr (std::is_default_constructible_v<T> && (std::is_move_assignable_v<T> || std::is_copy_assignable_v<T>))
				{
					out = T{};
				}
			}

			throw std::runtime_error(util::format("Unable to deserialize object of type: #{}", resolve<T>().id()));
		}

		auto raw_instance_ptr = instance.try_cast<T>();

		if ((!raw_instance_ptr) && fallback_to_default_construction)
		{
			if constexpr (std::is_default_constructible_v<T> && (std::is_move_assignable_v<T> || std::is_copy_assignable_v<T>))
			{
				out = T {};
			}

			return;
		}

		assert(raw_instance_ptr);

		if constexpr (std::is_move_assignable_v<T>)
		{
			out = std::move(*raw_instance_ptr);
		}
		else if constexpr (std::is_copy_assignable_v<T>)
		{
			out = *raw_instance_ptr;
		}
	}

	template <typename T>
	T load
	(
		const util::json& data,

		const MetaParsingInstructions& parse_instructions={},
		const MetaTypeDescriptorFlags& descriptor_flags={},

		bool fallback_to_default_construction=std::is_default_constructible_v<T>
	)
	{
		auto type = resolve<T>();

		assert(type);

		auto instance = load
		(
			type,
			data,

			parse_instructions,
			descriptor_flags
		);

		if (!instance)
		{
			if (fallback_to_default_construction)
			{
				if constexpr (std::is_default_constructible_v<T>)
				{
					return T {};
				}
			}

			throw std::runtime_error(util::format("Unable to deserialize object of type: #{}", type.id()));
		}

		auto raw_instance_ptr = instance.try_cast<T>();

		if ((!raw_instance_ptr) && fallback_to_default_construction)
		{
			return T{};
		}

		assert(raw_instance_ptr);

		return T(std::move(*raw_instance_ptr));
	}

	template <typename T>
	void load
	(
		T& out,
		
		const std::filesystem::path& path,
		bool use_assignment=true,

		const MetaParsingInstructions& parse_instructions={},
		const MetaTypeDescriptorFlags& descriptor_flags={},

		bool fallback_to_default_construction=std::is_default_constructible_v<T>
	)
	{
		load<T>
		(
			out,

			util::load_json(path),
			
			use_assignment,

			parse_instructions,
			descriptor_flags,

			fallback_to_default_construction
		);
	}

	template <typename T>
	T load
	(
		const std::filesystem::path& path,

		const MetaParsingInstructions& parse_instructions={},
		const MetaTypeDescriptorFlags& descriptor_flags={},

		bool fallback_to_default_construction=std::is_default_constructible_v<T>
	)
	{
		return load<T>
		(
			util::load_json(path),

			parse_instructions,
			descriptor_flags,

			fallback_to_default_construction
		);
	}

	template <typename T>
	T load
	(
		std::string_view path,

		const MetaParsingInstructions& parse_instructions={},
		const MetaTypeDescriptorFlags& descriptor_flags={},

		bool fallback_to_default_construction=std::is_default_constructible_v<T>
	)
	{
		return load<T>
		(
			std::filesystem::path(path),

			parse_instructions,
			descriptor_flags,

			fallback_to_default_construction
		);
	}
}