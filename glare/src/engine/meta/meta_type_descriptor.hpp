#pragma once

#include "types.hpp"
#include "meta_variable.hpp"

#include <util/small_vector.hpp>

// TODO: Forward declare JSON type.
#include <util/json.hpp>

#include <optional>
#include <string_view>
#include <tuple>

namespace engine
{
	struct MetaTypeDescriptorFlags
	{
		bool allow_default_construction : 1 = true;
		bool allow_forwarding_fields_to_constructor : 1 = true;
		bool force_field_assignment : 1 = false;
	};

	struct MetaTypeDescriptor
	{
		protected:
			MetaTypeDescriptor() = default;
		public:
			using Flags = MetaTypeDescriptorFlags;
			using SmallSize = std::uint8_t; // std::uint16_t; // std::size_t;

			using Names = MetaSymbolStorage; // util::small_vector<MetaSymbolID, 4>; // 8
			using Values = MetaStorage; // util::small_vector<MetaAny, 4>; // 8

			static MetaType get_self_type();

			static std::tuple
			<
				std::string_view, // Name
				std::string_view  // Type
			>
			parse_variable_declaration(std::string_view var_decl, std::string_view type_specifier_symbol=":");

			MetaTypeDescriptor(MetaType type, std::optional<SmallSize> constructor_argument_count=std::nullopt, const MetaTypeDescriptorFlags& flags={});
			MetaTypeDescriptor(MetaTypeID type_id, std::optional<SmallSize> constructor_argument_count=std::nullopt, const MetaTypeDescriptorFlags& flags={});
			
			// Constructs a meta-type descriptor from a symbol-separated list of values. (Defaults to comma-separated; CSV)
			MetaTypeDescriptor
			(
				MetaType type,
				std::string_view content,
				std::string_view arg_separator=",",
				bool resolve_values=true,
				std::size_t argument_offset=0,
				
				std::optional<SmallSize> constructor_argument_count=std::nullopt,
				const MetaTypeDescriptorFlags& flags={}
			);

			MetaTypeDescriptor
			(
				MetaType meta_type,
				const util::json& content,

				std::optional<SmallSize> constructor_argument_count=std::nullopt,
				const MetaTypeDescriptorFlags& flags={}
			);

			MetaTypeDescriptor(const MetaTypeDescriptor&) = default;
			MetaTypeDescriptor(MetaTypeDescriptor&&) noexcept = default;

			MetaTypeDescriptor& operator=(const MetaTypeDescriptor&) = default;
			MetaTypeDescriptor& operator=(MetaTypeDescriptor&&) noexcept = default;

			// The type this descriptor is wrapping.
			MetaType type;

			Names field_names;
			Values field_values;

			std::optional<SmallSize> constructor_argument_count = std::nullopt; // std::size_t

			Flags flags;

			std::optional<std::size_t> get_variable_index(MetaSymbolID name) const;

			const MetaAny* get_variable(MetaSymbolID name) const;
			MetaAny* get_variable(MetaSymbolID name);

			// This returns `nullptr` if variable assignment fails.
			// NOTE: In the event of failure, `variable` is guaranteed to be in a valid state.
			MetaAny* set_variable(MetaVariable&& variable, bool safe=true);

			// Executes `set_variable` for every field in `variables`.
			// 
			// NOTE:
			// The values held by `variables` will be left in a moved-from state.
			// Variable names/identifiers will be left intact.
			void set_variables(MetaTypeDescriptor&& variables, bool override_constructor_input_size=true);

			// Reads each element from `content` as a field of this meta-type descriptor.
			std::size_t set_variables(const util::json& content, std::size_t argument_offset=0);

			// Attempts to convert from a symbol-separated string into a series of indexed variables.
			std::size_t set_variables
			(
				std::string_view content,

				std::string_view arg_separator=",",
				bool resolve_values=true,
				std::size_t argument_offset=0,

				bool safe=true
			);

			// Performs a shallow search of `field_values` to determine if an additional `MetaTypeDescriptor` is nested.
			// This overload stops searching at the number of values specified.
			bool has_nested_descriptor(std::size_t n_values) const;
		
			// Performs a shallow search of `field_values` to determine if an additional `MetaTypeDescriptor` is nested.
			bool has_nested_descriptor() const;


			/*
				Generates an instance of the described type.
			
				NOTE:
			
				When `allow_recursion` is true, if any of the entries in `field_values` is a `MetaTypeDescriptor`,
				those objects' `instance` methods will be called as well. (recursion)
			
				This allows for nested complex type definitions.
			*/
			MetaAny instance(bool allow_recursion=true) const;

			// Attempts to create an instance of the underlying `type`,
			// using the defined fields as constructor arguments.
			// 
			// The `instance` member-function is recommended over this one for general purpose usage.
			MetaAny instance_exact(bool allow_recursion=true) const;

			// Attempts to default-construct an instance of `type`.
			MetaAny instance_default() const;

			/*
				Updates each field of `instance` defined by this descriptor.
			
				NOTE: A `MetaTypeDescriptor` object does not necessarily need to
				define 1:1 field names/values of its underlying type.
				(e.g. hashes of array indices in place of field names)
			
				If a field is not found within `type` matching the
				field in question, it will be ignored.
			*/
			std::size_t apply_fields(MetaAny& instance, std::size_t field_count, std::size_t offset=0) const;

			std::size_t apply_fields(MetaAny & instance) const;

			inline MetaAny operator()() const
			{
				return instance(true);
			}

			inline auto data() const { return field_values.data(); }
			inline auto size() const { return field_values.size(); }

			inline bool can_default_construct() const { return flags.allow_default_construction; }
			inline bool can_forward_fields_to_constructor() const { return flags.allow_forwarding_fields_to_constructor; }
			inline bool forces_field_assignment() const { return flags.force_field_assignment; }
	};
}