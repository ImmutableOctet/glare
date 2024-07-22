#pragma once

// This is an internal header for the `engine` module and should only be used by reflection implementation files.
// 
// This header defines general-purpose reflection facilities for systems, components, etc. using EnTT's meta-type system.
// For consuming or instantiating reflection data, it is recommended that you directly use the `meta` header instead.

#include "meta_type_reflection_config.hpp"

#include "core.hpp"
#include "reflect.hpp"
#include "context.hpp"
#include "function.hpp"
#include "operators.hpp"
#include "indirection.hpp"
#include "json_bindings.hpp"
#include "binary_bindings.hpp"
#include "aggregate.hpp"
#include "optional.hpp"
#include "history.hpp"
#include "enum.hpp"
#include "empty.hpp"
#include "static.hpp"
#include "command.hpp"
#include "service.hpp"
#include "system.hpp"

#include "common_extensions.hpp"
#include "component_extensions.hpp"
#include "optional_extensions.hpp"
#include "json_extensions.hpp"
#include "service_extensions.hpp"

#include <engine/types.hpp>
#include <engine/registry.hpp>
#include <engine/entity.hpp>

//#include <engine/meta/meta.hpp>
#include <engine/meta/types.hpp>
#include <engine/meta/enum.hpp>
#include <engine/meta/hash.hpp>
#include <engine/meta/indirection.hpp>
#include <engine/meta/serial.hpp>
#include <engine/meta/traits.hpp>
#include <engine/meta/short_name.hpp>
#include <engine/meta/cast.hpp>

#include <engine/meta/reflect_all.hpp>

#include <engine/meta/meta_type_descriptor.hpp>
#include <engine/meta/meta_event_listener.hpp>
#include <engine/meta/meta_evaluation_context.hpp>

#include <engine/service.hpp>
#include <engine/system_manager_interface.hpp>

#include <util/json.hpp>
#include <util/reflection.hpp>

#include <entt/entt.hpp>

#include <string_view>
#include <type_traits>
#include <utility>
#include <optional>
#include <tuple>
#include <stdexcept>
#include <cstddef>

// Debugging related:
#include <util/log.hpp>

#define GENERATE_SINGLE_FIELD_TYPE_REFLECTION(type_name, field_name) \
    template <>                                                      \
    void reflect<type_name>()                                        \
    {                                                                \
        REFLECT_SINGLE_FIELD_TYPE(type_name, field_name);            \
    }

#define GENERATE_SINGLE_FIELD_DERIVED_TYPE_REFLECTION(type_name, base_type_name, field_name) \
    template <>                                                                              \
    void reflect<type_name>()                                                                \
    {                                                                                        \
        REFLECT_SINGLE_FIELD_DERIVED_TYPE(type_name, base_type_name, field_name);            \
    }

#define GENERATE_SINGLE_FIELD_COMPONENT_REFLECTION GENERATE_SINGLE_FIELD_TYPE_REFLECTION

// Generates an empty `reflect` function for the specified `engine` type.
#define GENERATE_EMPTY_TYPE_REFLECTION(type_name) \
    template <>                                   \
    void reflect<type_name>()                     \
    {                                             \
        engine::engine_meta_type<type_name>();    \
    }

// Generates an empty `reflect` function for the specified `engine` type,
// where `type_name` is derived from `base_type_name`.
#define GENERATE_EMPTY_DERIVED_TYPE_REFLECTION(type_name, base_type_name) \
    template <>                                                           \
    void reflect<type_name>()                                             \
    {                                                                     \
        engine::engine_meta_type<type_name>()                             \
            .base<base_type_name>()                                       \
        ;                                                                 \
    }