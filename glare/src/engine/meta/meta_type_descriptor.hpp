#pragma once

#include "types.hpp"
#include "meta_variable.hpp"
#include "meta_type_descriptor_flags.hpp"

#include <util/small_vector.hpp>

// TODO: Forward declare JSON type.
#include <util/json.hpp>

#include <optional>
#include <utility>
#include <string_view>
#include <tuple>

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	struct ParsingContext;

	struct MetaTypeConstructionFlags
	{
		MetaType type = {};
		bool allow_indirection = true;
	};

	struct MetaTypeDescriptor
	{
		public:
			using Flags = MetaTypeDescriptorFlags;
			using SmallSize = std::uint8_t; // std::uint16_t; // std::size_t;

			using Names = MetaSymbolStorage; // util::small_vector<MetaSymbolID, 4>; // 8
			using Values = MetaStorage; // util::small_vector<MetaAny, 4>; // 8

			static MetaType get_self_type();

			static MetaType resolve_type(MetaType type, const Flags& flags);
			static MetaTypeID resolve_type_id(MetaTypeID type_id, const Flags& flags);

			static std::tuple
			<
				std::string_view, // Name
				std::string_view  // Type
			>
			parse_variable_declaration(std::string_view var_decl, std::string_view type_specifier_symbol=":");

			// Alias for the version implemented in `meta`. (Acts as a forward declaration)
			static std::optional<std::pair<entt::id_type, entt::meta_data>>
			get_data_member_by_index(const entt::meta_type& type, std::size_t variable_index, bool recursive=true);

			MetaTypeDescriptor() = default;

			MetaTypeDescriptor(MetaType type, std::optional<SmallSize> constructor_argument_count=std::nullopt, const MetaTypeDescriptorFlags& flags={});
			MetaTypeDescriptor(MetaTypeID type_id, std::optional<SmallSize> constructor_argument_count=std::nullopt, const MetaTypeDescriptorFlags& flags={});
			
			// Constructs a meta-type descriptor from a symbol-separated list of values. (Defaults to comma-separated; CSV)
			MetaTypeDescriptor
			(
				MetaType type,

				std::string_view content,
				const MetaAnyParseInstructions& instructions={},
				
				std::string_view arg_separator=",",
				std::size_t argument_offset=0,
				
				std::optional<SmallSize> constructor_argument_count=std::nullopt,
				const MetaTypeDescriptorFlags& flags={}
			);

			MetaTypeDescriptor
			(
				MetaType meta_type,

				const util::json& content,
				const MetaAnyParseInstructions& instructions={},
				//std::size_t argument_offset = 0,

				std::optional<SmallSize> constructor_argument_count=std::nullopt,
				const MetaTypeDescriptorFlags& flags={},
				const ParsingContext* opt_parsing_context=nullptr
			);

			MetaTypeDescriptor(const MetaTypeDescriptor&) = default;
			MetaTypeDescriptor(MetaTypeDescriptor&&) noexcept = default;

			MetaTypeDescriptor& operator=(const MetaTypeDescriptor&) = default;
			MetaTypeDescriptor& operator=(MetaTypeDescriptor&&) noexcept = default;

		protected:
			// The type this descriptor is wrapping.
			MetaTypeID type_id = {};
		public:
			Names field_names;
			Values field_values;

			std::optional<SmallSize> constructor_argument_count = std::nullopt; // std::size_t

			Flags flags;

			inline auto data() const { return field_values.data(); }
			inline auto size() const { return field_values.size(); }

			inline bool can_default_construct() const { return flags.allow_default_construction; }
			inline bool can_forward_fields_to_constructor() const { return flags.allow_forwarding_fields_to_constructor; }
			inline bool forces_field_assignment() const { return flags.force_field_assignment; }

			bool set_type(MetaType type);
			MetaType get_type() const;

			bool set_type_id(MetaTypeID type_id);
			MetaTypeID get_type_id() const;

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
			void set_variables
			(
				MetaTypeDescriptor&& variables,
				bool override_constructor_input_size=true
			);

			// Reads each element from `content` as a field of this meta-type descriptor.
			std::size_t set_variables
			(
				const util::json& content,
				const MetaAnyParseInstructions& instructions={},
				std::size_t argument_offset=0,
				const ParsingContext* opt_parsing_context=nullptr
			);

			// Attempts to convert from a symbol-separated string into a series of indexed variables.
			std::size_t set_variables
			(
				std::string_view content,
				const MetaAnyParseInstructions& instructions={},

				std::string_view arg_separator=",",
				std::size_t argument_offset=0,

				bool safe=true
			);

			/*
			template <typename ...Content>
			std::size_t set_variables(Content&&... content)
			{
				return set_variables_direct(std::forward<Content>(content)...);
			}
			*/

			template <typename ...Content>
			std::size_t set_variables_direct(Content&&... content)
			{
				return set_variables_direct_impl(get_type(), std::forward<Content>(content)...);
			}

			// Performs a shallow search of `field_values` to determine if an additional `MetaTypeDescriptor` is nested.
			// This overload stops searching at the number of values specified.
			// 
			// TODO: Implement optional `offset` parameter.
			bool has_nested_descriptor(std::size_t n_values) const;
		
			// Performs a shallow search of `field_values` to determine if an additional `MetaTypeDescriptor` is nested.
			bool has_nested_descriptor() const;

			// Performs a shallow search of `field_values` to determine if a `MetaDataMember` or `IndirectDataMember` object is nested.
			// This overload stops searching at the number of values specified.
			// 
			// TODO: Implement optional `offset` parameter.
			bool has_nested_member_reference(std::size_t n_values) const;

			// Performs a shallow search of `field_values` to determine if a `MetaDataMember` or `IndirectMetaDataMember` object is nested.
			bool has_nested_member_reference() const;

			// TODO: Implement optional `offset` parameter.
			inline bool has_indirection(std::size_t n_values) const
			{
				return (has_nested_descriptor(n_values) || has_nested_member_reference(n_values));
			}

			inline bool has_indirection() const
			{
				return (has_nested_descriptor() || has_nested_member_reference());
			}
			
			/*
				Generates an instance of the described type.
			
				NOTE:
			
				When `allow_indirection` is true, if any of the entries in `field_values` is a `MetaTypeDescriptor`,
				those objects' `instance` methods will be called as well. (Recursion)
			
				This allows for nested complex type definitions.
			*/
			template <typename ...Args>
			inline MetaAny instance(const MetaTypeConstructionFlags& construction_flags, Args&&... args) const
			{
				MetaAny instance;

				if (flags.allow_forwarding_fields_to_constructor)
				{
					instance = instance_exact(construction_flags, std::forward<Args>(args)...);
				}

				bool is_default_constructed = false;

				if (!instance)
				{
					if (flags.allow_default_construction)
					{
						instance = instance_default(construction_flags.type); // std::forward<Args>(args)...

						is_default_constructed = true;
					}

					if (!instance)
					{
						if (construction_flags.type)
						{
							print_warn("Unable to resolve constructor for component: \"#{}\"", construction_flags.type.id());
						}

						return {};
					}
				}

				if (is_default_constructed || flags.force_field_assignment)
				{
					apply_fields(instance, std::forward<Args>(args)...);
				}

				return instance;
			}

			template <typename ...Args>
			inline MetaAny instance(MetaType type, Args&&... args) const
			{
				return instance
				(
					MetaTypeConstructionFlags
					{
						type, true
					},

					std::forward<Args>(args)...
				);
			}

			template <typename ...Args>
			inline MetaAny instance(bool allow_indirection, Args&&... args) const
			{
				return instance
				(
					MetaTypeConstructionFlags
					{
						get_type(), allow_indirection
					},

					std::forward<Args>(args)...
				);
			}

			inline MetaAny instance() const
			{
				return instance(true);
			}

			// Attempts to create an instance of the underlying `type`,
			// using the defined fields as constructor arguments.
			// 
			// The `instance` member-function is recommended over this one for general purpose usage.
			template <typename ...Args>
			inline MetaAny instance_exact(const MetaTypeConstructionFlags& construction_flags, Args&&... args) const
			{
				const auto& type = construction_flags.type;

				if (!type)
				{
					return {};
				}

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

				if (construction_flags.allow_indirection)
				{
					// NOTE: In the event of a nested descriptor without `allow_indirection`, we assume it is
					// correct to pass the nested `MetaTypeDescriptor` object to the underlying type's constructor.
					if (has_indirection(constructor_args_count))
					{
						// TODO: Look into whether there's any room to optimize via
						// use of 'handles' rather than full `entt::meta_any` instances.
						MetaTypeDescriptor::Values rebuilt_constructor_values;

						rebuilt_constructor_values.reserve(constructor_args_count);

						auto result = resolve_values(rebuilt_constructor_values, constructor_args_count, std::forward<Args>(args)...);

						assert(result == constructor_args_count);

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

			template <typename ...Args>
			inline MetaAny instance_exact(MetaType type, Args&&... args) const
			{
				return instance_exact
				(
					MetaTypeConstructionFlags
					{
						type, true
					},

					std::forward<Args>(args)...
				);
			}

			template <typename ...Args>
			inline MetaAny instance_exact(bool allow_indirection, Args&&... args) const
			{
				return instance_exact
				(
					MetaTypeConstructionFlags
					{
						get_type(), allow_indirection
					},

					std::forward<Args>(args)...
				);
			}

			inline MetaAny instance_exact() const
			{
				return instance_exact(true);
			}

			// Attempts to default-construct an instance of `type`.
			MetaAny instance_default(MetaType type) const;

			inline MetaAny instance_default(const MetaTypeConstructionFlags& construction_flags) const
			{
				return instance_default(construction_flags.type);
			}

			inline MetaAny instance_default() const
			{
				return instance_default(get_type());
			}

			/*
				Updates each field of `instance`, defined by this descriptor.
			
				NOTE: A `MetaTypeDescriptor` object does not necessarily need to
				define 1:1 field names/values of its underlying type.
				(e.g. You can have hashes of array indices in place of field names)
			
				If a field is not found within `type` matching
				the field in question, it will be ignored.
			*/
			std::size_t apply_fields(MetaAny& instance, std::size_t field_count, std::size_t offset=0) const;

			std::size_t apply_fields(MetaAny& instance) const;

			// Updates each field of `instance`, defined by this descriptor.
			// 
			// This overload uses `registry` and `entity` to resolve source-indirection
			// from the `field_values` container. (e.g. References to component member)
			std::size_t apply_fields(MetaAny& instance, std::size_t field_count, Registry& registry, Entity entity, std::size_t offset=0) const;

			std::size_t apply_fields(MetaAny& instance, Registry& registry, Entity entity) const;

			std::size_t resolve_values(Values& values_out, std::size_t constructor_args_count, std::size_t offset=0) const;
			std::size_t resolve_values(Values& values_out, std::size_t constructor_args_count, Registry& registry, Entity entity, std::size_t offset=0) const;

			template <typename ...Args>
			inline MetaAny operator()(Args&&... args) const
			{
				return instance(true, std::forward<Args>(args)...);
			}

			inline MetaAny operator()() const
			{
				return instance(true);
			}
		private:
			std::size_t set_variables_impl
			(
				const MetaType& type,
				const util::json& content,
				const MetaAnyParseInstructions& instructions={},
				std::size_t argument_offset=0,
				const ParsingContext* opt_parsing_context=nullptr
			);

			std::size_t set_variables_impl
			(
				const MetaType& type,
				std::string_view content,
				const MetaAnyParseInstructions& instructions={},

				std::string_view arg_separator=",",
				std::size_t argument_offset=0,

				bool safe=true
			);

			template <typename ...Content>
			std::size_t set_variables_direct_impl(MetaType type, Content&&... content)
			{
				std::size_t variable_index = 0;
				std::size_t count = 0;

				(
					[&]()
					{
						if (auto data_entry = get_data_member_by_index(type, variable_index, true))
						{
							const auto var_name_hash = data_entry->first;

							if (set_variable(MetaVariable(var_name_hash, MetaAny { std::forward<Content>(content) })))
							{
								count++;
							}
						}

						variable_index++;
					}(), ...
				);

				return count;
			}

			static MetaAny resolve_indirect_value(const MetaAny& entry);
			static MetaAny resolve_indirect_value(const MetaAny& entry, Registry& registry, Entity entity);

			// Defined and only used in source file.
			template <typename ...Args>
			std::size_t apply_fields_impl(MetaAny& instance, std::size_t field_count, std::size_t offset, Args&&... args) const;

			// Defined and only used in source file.
			template <typename ...Args>
			std::size_t resolve_values_impl(Values& values_out, std::size_t constructor_args_count, std::size_t offset, Args&&... args) const;
	};

	MetaTypeDescriptor load_descriptor
	(
		MetaType type,

		const std::filesystem::path& path,
		
		const MetaAnyParseInstructions& parse_instructions={},
		const MetaTypeDescriptorFlags& descriptor_flags={},
		const ParsingContext* opt_parsing_context=nullptr
	);

	MetaTypeDescriptor load_descriptor
	(
		MetaType type,

		const util::json& content,
		
		const MetaAnyParseInstructions& parse_instructions={},
		const MetaTypeDescriptorFlags& descriptor_flags={},
		const ParsingContext* opt_parsing_context=nullptr
	);
}