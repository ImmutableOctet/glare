#pragma once

#include "types.hpp"

namespace engine
{
	// Returns true if `type` is considered a primitive type.
    // 
    // The `strings_as_primitives` parameter controls whether `std::string`
    // and `std::string_view` are considered primitive as well.
    bool type_is_primitive(const MetaType& type, bool strings_as_primitives=true);

    // Returns true if the type referenced by `type_id` is considered a primitive type.
    bool type_is_primitive(MetaTypeID type_id, bool strings_as_primitives=true);

    // Returns true if the `value` specified is a primitive type.
    // 
    // The `strings_as_primitives` parameter controls whether `std::string`
    // and `std::string_view` are considered primitive as well.
    bool value_is_primitive(const MetaAny& value, bool strings_as_primitives=true);

	// Returns true if the `value` specified has a reflected indirection function.
    bool value_has_indirection(const MetaAny& value, bool bypass_indirect_meta_any=false);

    // Returns true if the `type` specified has a reflected indirection function.
    bool type_has_indirection(const MetaType& type);

    // Returns true if the the type referenced by `type_id` has a reflected indirection function.
    bool type_has_indirection(MetaTypeID type_id);

    // Returns true if the `type` specified is a 'system' type.
    bool type_is_system(const MetaType& type);

    // Returns true if the type referenced by the `type_id` specified is a 'system' type.
    bool type_is_system(const MetaTypeID type_id);

    // Returns true if the `value` specified references a 'system'.
    bool value_is_system(const MetaAny& value);

    // Returns true  if the `type` specified has the `global namespace` property.
    bool type_has_global_namespace_flag(const MetaType& type);

    // Returns true  if the type identified by `type_id` has the `global namespace` property.
    bool type_has_global_namespace_flag(MetaTypeID type_id);
}