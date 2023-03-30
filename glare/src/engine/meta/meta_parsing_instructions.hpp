#pragma once

#include "meta_parsing_context.hpp"

namespace engine
{
	class SharedStorageInterface;

	using MetaParsingStorage = SharedStorageInterface;

	struct MetaParsingInstructions
	{
		// An optional 'parsing context', used to handle
		// resolution of type-name and variable references.
		MetaParsingContext context;

		// An optional pointer to a 'shared storage container',
		// used for indirect references to objects that are
		// too large to fit inside of a `MetaAny`.
		MetaParsingStorage* storage = nullptr;

		// Controls whether symbol resolution steps shall be performed.
		// (Most parsing and evaluation falls under this category)
		bool resolve_symbol                   : 1 = true;
		
		// Controls whether quote symbols may be
		// included in the contents of a string.
		bool strip_quotes                     : 1 = true;

		// Controls whether some part of the expression can be
		// treated as a string if no other resolution path is available.
		bool fallback_to_string               : 1 = true;
		
		// Controls whether a freestanding symbol can be
		// treated as a direct reference to a component type.
		bool fallback_to_component_reference  : 1 = true;

		// Controls whether a freestanding symbol can be
		// treated as a direct reference to an entity by name.
		bool fallback_to_entity_reference     : 1 = false;

		// Controls whether references to type data-members
		// can be encoded into the expression.
		bool allow_member_references          : 1 = false;

		// Controls whether entity indirection statements
		// (targets) can be encoded into the expression.
		bool allow_entity_indirection         : 1 = false;

		// Controls whether a residual symbol can be considered
		// as input to construct an instance of a specified type.
		bool allow_implicit_type_construction : 1 = false;

		// Controls whether the user is allowed to manually construct type
		// instances during the evaluation and initial processing steps.
		bool allow_explicit_type_construction : 1 = true;

		// Controls whether parsing of arithmetic literals is allowed.
		bool allow_numeric_literals           : 1 = true;

		// Controls whether parsing of boolean literals is allowed. (i.e. `true` and `false`)
		bool allow_boolean_literals           : 1 = true;

		// Controls whether an expression can encode a deferred function or method call operation.
		bool allow_function_call_semantics    : 1 = true;

		// Controls whether an expression can encode a subscript operation.
		bool allow_subscript_semantics        : 1 = true;

		// Controls whether top-level value commands can be used. (e.g. `hash`)
		bool allow_value_resolution_commands  : 1 = true;

		// Controls whether variable assignment can be encoded. (Use of assignment operators; `=`, `+=`, etc.)
		bool allow_variable_assignment        : 1 = true;

		// Controls whether indirect/remote variable references can be inferred.
		bool allow_remote_variable_references : 1 = false;

		// Controls whether opaque data-member references can be encoded.
		// (i.e. when the type is not known ahead-of-time, but the member is)
		bool allow_opaque_member_references   : 1 = true;

		// Controls whether opaque function references can be encoded.
		// (i.e. when the type is not known ahead-of-time, but the function is)
		bool allow_opaque_function_references : 1 = true;

		// Controls whether value operations (Use of operators; `+`, `-`, etc.)
		// can be performed by an expression during its evaluation step.
		bool resolve_value_operations         : 1 = true;

		// By default, components are handled regularly.
		// (i.e. `name` can be used in place of `NameComponent`)
		bool resolve_component_aliases        : 1 = true;

		/*
			NOTE: Command-type shorthand is currently disabled by default.
			
			This is mainly due to the first two fields being reserved for entity IDs.
			(Which are normally handled outside of `meta_any_from_string`, etc.)
								
			When disabled, command-types can still be instantiated
			directly using their reflected names.
			(i.e. use `PrintCommand` rather than `print`).
		*/
		bool resolve_command_aliases          : 1 = false;

		// Instruction aliases are disabled by default, since they're not usually expected.
		bool resolve_instruction_aliases      : 1 = false;

		// If enabled, container creation will be deferred to the evaluation phase.
		// 
		// NOTE: Deferral of associative containers has the drawback that
		// only string and string-hash based keys may be used.
		// 
		// Additionally, deferred containers may have an implied 'redundant' memory footprint.
		// Standard behavior for container creation from a `MetaTypeDescriptor` is to perform a deep copy.
		// 
		// On the other hand, element values that are themselves evaluable may have little overhead compared to
		// storing the final value, and are able to take advantage of shared-storage mechanics for better memory locality.
		// 
		// Deferral may be elided in the event that immediate evaluation of the container is known to take place.
		// (This is usually the case with `engine::load`, as opposed to `resolve_meta_any`)
		bool defer_container_creation : 1 = true; // false;
	};
}