// This file aggregates all `engine`-related reflection headers into one translation unit. 
// The purpose of which is to provide a single implementation-file for meta-type instantiation. (see `reflect_all`)
//
// TODO: Rework this source file into some form of automated 'reflection generation' procedure in the build process.

#include "reflection.hpp"

#include "container_extensions.hpp"
#include "entity_extensions.hpp"
#include "primitive_extensions.hpp"
#include "string_extensions.hpp"
#include "chrono_extensions.hpp"

#include "string_conversion.hpp"

#include "math.hpp"
#include "util.hpp"
#include "script.hpp"

#include <engine/components/reflection.hpp>
#include <engine/commands/reflection.hpp>

#include <engine/meta/reflection.hpp>
#include <engine/resource_manager/reflection.hpp>
#include <engine/entity/reflection.hpp>
#include <engine/debug/reflection.hpp>
#include <engine/input/reflection.hpp>
#include <engine/world/reflection.hpp>
#include <engine/editor/reflection.hpp>

#include <engine/meta/hash.hpp>

#include <engine/config.hpp>
#include <engine/timer.hpp>

#include <engine/platform/reflection.hpp>

//#include <math/format.hpp>
//#include <math/types.hpp>

#include <util/format.hpp>
#include <util/string.hpp>

#include <utility>
#include <type_traits>
#include <string>
#include <string_view>
#include <optional>
#include <algorithm>
#include <cstdint>

//#include <entt/meta/meta.hpp>
//#include <entt/entt.hpp>

namespace engine
{
    void reflect_systems()
    {
        reflect<DebugListener>();
        reflect<EntitySystem>();
        reflect<InputSystem>();
        reflect<Editor>();

        // ...
    }

    template
    <
        typename PrimitiveType,
        
        bool generate_optional_type=true,
        bool generate_operators=false,
        bool generate_json_bindings=true
    >
    static auto extend_language_primitive_type(bool sync_context=true)
    {
        if (sync_context)
        {
            // Ensure that we're using the correct context.
            sync_reflection_context();
        }

        auto type = entt::meta<PrimitiveType>();

        auto cast_construct_from = [&type]<typename T>()
        {
            if constexpr (!std::is_same_v<std::decay_t<T>, std::decay_t<PrimitiveType>>)
            {
                type = type.ctor<&impl::static_cast_impl<PrimitiveType, T>>();
            }
        };

        auto cast_convert_to = [&type]<typename T>()
        {
            if constexpr (!std::is_same_v<std::decay_t<T>, std::decay_t<PrimitiveType>>)
            {
                type = type.conv<&impl::static_cast_impl<T, PrimitiveType>>();
            }
        };

        cast_construct_from.operator()<bool>();
        cast_construct_from.operator()<float>();
        cast_construct_from.operator()<double>();
        cast_construct_from.operator()<long double>();
        cast_construct_from.operator()<std::int64_t>();
        cast_construct_from.operator()<std::uint64_t>();
        cast_construct_from.operator()<std::int32_t>();
        cast_construct_from.operator()<std::uint32_t>();
        cast_construct_from.operator()<std::int16_t>();
        cast_construct_from.operator()<std::uint16_t>();

        //cast_construct_from.operator()<std::int8_t>>();
        //cast_construct_from.operator()<std::uint8_t>>();

        cast_convert_to.operator()<std::int32_t>();
        cast_convert_to.operator()<std::int64_t>();
        cast_convert_to.operator()<float>();
        cast_convert_to.operator()<double>();

        type = type
            .ctor<&impl::from_string_view_impl<PrimitiveType>>()
            .ctor<&impl::from_string_impl<PrimitiveType>>()

            .conv<&impl::arithmetic_to_string_impl<PrimitiveType>>()
            .conv<&impl::operator_bool_impl<PrimitiveType>>()
        ;

        if constexpr (generate_operators)
        {
            type = type
                .func<&impl::operator_unary_plus_impl<PrimitiveType>>("+operator"_hs)
                .func<&impl::operator_unary_minus_impl<PrimitiveType>>("-operator"_hs)
            ;
        }

        if constexpr (generate_json_bindings)
        {
            type = define_from_json_bindings<PrimitiveType>(type);
            type = define_to_json_bindings<PrimitiveType>(type);
        }

        if constexpr (generate_optional_type)
        {
            constexpr auto type_name = entt::type_name<PrimitiveType>::value();

            auto opt_type = custom_optional_type<PrimitiveType>(type_name);
        }

        return type;
    }

    template <>
    void reflect<Entity>()
    {
        constexpr bool sync_context = true;
        constexpr bool generate_optional_type = true;

        if constexpr (sync_context)
        {
            // Ensure that we're using the correct context.
            sync_reflection_context();
        }

        constexpr auto type_name = std::string_view { "Entity" }; // entt::type_name<Entity>::value();

        auto entity_type = entt::meta<Entity>()
            .type("Entity"_hs) // hash(type_name)
            
            .ctor<&impl::entity_static_create>()

            .conv<&impl::entity_to_string_impl>()
            //.conv<std::underlying_type_t<Entity>>()

            .conv<&impl::entity_to_bool_impl>()
            
            .ctor<&impl::entity_from_integer>()

            // Functions:
            .func<&impl::entity_static_create>("create"_hs)
            .func<&impl::entity_static_destroy>("destroy"_hs)

            // Methods:
            .func<&impl::entity_destroy>("destroy"_hs)

            .func<&impl::entity_try_get_state_component>("try_get_state_component"_hs)
            .func<&impl::entity_try_get_active_state>("try_get_active_state"_hs)
            .func<&impl::entity_try_get_prev_state>("try_get_prev_state"_hs)

            // Properties:
            .func<&impl::entity_get_position>("get_position"_hs)
            .func<&impl::entity_set_position>("set_position"_hs)

            .func<&impl::entity_get_local_position>("get_local_position"_hs)
            .func<&impl::entity_set_local_position>("set_local_position"_hs)

            .func<&impl::entity_get_rotation>("get_rotation"_hs)
            .func<&impl::entity_set_rotation>("set_rotation"_hs)

            .func<&impl::entity_get_local_rotation>("get_local_rotation"_hs)
            .func<&impl::entity_set_local_rotation>("set_local_rotation"_hs)

            .func<&impl::entity_get_basis>("get_basis"_hs)
            .func<static_cast<math::Quaternion(*)(Entity, Registry&, const math::Quaternion&, float)>(&impl::entity_set_basis_q)>("set_basis"_hs)
            .func<static_cast<math::Quaternion(*)(Entity, Registry&, const math::Quaternion&)>(&impl::entity_set_basis_q)>("set_basis"_hs)
            .func<static_cast<math::RotationMatrix(*)(Entity, Registry&, const math::RotationMatrix&, float)>(&impl::entity_set_basis)>("set_basis"_hs)
            .func<static_cast<math::RotationMatrix(*)(Entity, Registry&, const math::RotationMatrix&)>(&impl::entity_set_basis)>("set_basis"_hs)

            .func<&impl::entity_get_local_basis>("get_local_basis"_hs)
            .func<static_cast<math::Quaternion(*)(Entity, Registry&, const math::Quaternion&, float)>(&impl::entity_set_local_basis_q)>("set_local_basis"_hs)
            .func<static_cast<math::Quaternion(*)(Entity, Registry&, const math::Quaternion&)>(&impl::entity_set_local_basis_q)>("set_local_basis"_hs)
            .func<static_cast<math::RotationMatrix(*)(Entity, Registry&, const math::RotationMatrix&, float)>(&impl::entity_set_local_basis)>("set_local_basis"_hs)
            .func<static_cast<math::RotationMatrix(*)(Entity, Registry&, const math::RotationMatrix&)>(&impl::entity_set_local_basis)>("set_local_basis"_hs)

            .func<&impl::entity_get_basis_q>("get_basis_q"_hs)
            .func<static_cast<math::Quaternion(*)(Entity, Registry&, const math::Quaternion&, float)>(&impl::entity_set_basis_q)>("set_basis_q"_hs)
            .func<static_cast<math::Quaternion(*)(Entity, Registry&, const math::Quaternion&)>(&impl::entity_set_basis_q)>("set_basis_q"_hs)

            .func<&impl::entity_get_local_basis_q>("get_local_basis_q"_hs)
            .func<static_cast<math::Quaternion(*)(Entity, Registry&, const math::Quaternion&, float)>(&impl::entity_set_local_basis_q)>("set_local_basis_q"_hs)
            .func<static_cast<math::Quaternion(*)(Entity, Registry&, const math::Quaternion&)>(&impl::entity_set_local_basis_q)>("set_local_basis_q"_hs)

            // See below for `set_direction_vector` bindings.
            .func<&impl::entity_get_direction_vector>("get_direction_vector"_hs)

            .func<&impl::entity_get_flat_direction_vector>("get_flat_direction_vector"_hs)
            .func<static_cast<math::Vector(*)(Entity, Registry&, const math::Vector&, float)>(&impl::entity_set_flat_direction_vector)>("set_flat_direction_vector"_hs)
            .func<static_cast<math::Vector(*)(Entity, Registry&, const math::Vector&)>(&impl::entity_set_flat_direction_vector)>("set_flat_direction_vector"_hs)

            // See below for direction-vector assignment bindings.
            .func<&impl::entity_get_direction_vector>("get_direction"_hs)
            .func<static_cast<math::RotationMatrix(*)(Entity, Registry&, const math::RotationMatrix&, float)>(&impl::entity_set_basis)>("set_direction"_hs)
            .func<static_cast<math::RotationMatrix(*)(Entity, Registry&, const math::RotationMatrix&)>(&impl::entity_set_basis)>("set_direction"_hs)
            .func<static_cast<math::Quaternion(*)(Entity, Registry&, const math::Quaternion&, float)>(&impl::entity_set_basis_q)>("set_direction"_hs)
            .func<static_cast<math::Quaternion(*)(Entity, Registry&, const math::Quaternion&)>(&impl::entity_set_basis_q)>("set_direction"_hs)

            .func<&impl::entity_get_scale>("get_scale"_hs)
            .func<&impl::entity_set_scale>("set_scale"_hs)

            .func<&impl::entity_get_local_scale>("get_local_scale"_hs)
            .func<&impl::entity_set_local_scale>("set_local_scale"_hs)

            .func<&impl::entity_get_position_x>("get_x"_hs)
            .func<&impl::entity_set_position_x>("set_x"_hs)

            .func<&impl::entity_get_position_y>("get_y"_hs)
            .func<&impl::entity_set_position_y>("set_y"_hs)

            .func<&impl::entity_get_position_z>("get_z"_hs)
            .func<&impl::entity_set_position_z>("set_z"_hs)

            .func<&impl::entity_get_rotation_pitch>("get_pitch"_hs)
            .func<static_cast<float(*)(Entity, Registry&, float, float)>(&impl::entity_set_rotation_pitch)>("set_pitch"_hs)
            .func<static_cast<float(*)(Entity, Registry&, float)>(&impl::entity_set_rotation_pitch)>("set_pitch"_hs)

            .func<&impl::entity_get_rotation_yaw>("get_yaw"_hs)
            .func<static_cast<float(*)(Entity, Registry&, float, float)>(&impl::entity_set_rotation_yaw)>("set_yaw"_hs)
            .func<static_cast<float(*)(Entity, Registry&, float)>(&impl::entity_set_rotation_yaw)>("set_yaw"_hs)

            .func<&impl::entity_get_rotation_roll>("get_roll"_hs)
            .func<static_cast<float(*)(Entity, Registry&, float, float)>(&impl::entity_set_rotation_roll)>("set_roll"_hs)
            .func<static_cast<float(*)(Entity, Registry&, float)>(&impl::entity_set_rotation_roll)>("set_roll"_hs)

            .func<&impl::entity_get_local_rotation_pitch>("get_local_pitch"_hs)
            .func<static_cast<float(*)(Entity, Registry&, float, float)>(&impl::entity_set_local_rotation_pitch)>("set_local_pitch"_hs)
            .func<static_cast<float(*)(Entity, Registry&, float)>(&impl::entity_set_local_rotation_pitch)>("set_local_pitch"_hs)

            .func<&impl::entity_get_local_rotation_yaw>("get_local_yaw"_hs)
            .func<static_cast<float(*)(Entity, Registry&, float, float)>(&impl::entity_set_local_rotation_yaw)>("set_local_yaw"_hs)
            .func<static_cast<float(*)(Entity, Registry&, float)>(&impl::entity_set_local_rotation_yaw)>("set_local_yaw"_hs)

            .func<&impl::entity_get_local_rotation_roll>("get_local_roll"_hs)
            .func<static_cast<float(*)(Entity, Registry&, float, float)>(&impl::entity_set_local_rotation_roll)>("set_local_roll"_hs)
            .func<static_cast<float(*)(Entity, Registry&, float)>(&impl::entity_set_local_rotation_roll)>("set_local_roll"_hs)

            .func<&impl::entity_get_parent>("get_parent"_hs)
            .func<&impl::entity_set_parent>("set_parent"_hs)

            .func<&impl::entity_get_state_id>("get_state_id"_hs)
            .func<&impl::entity_set_state_name>("set_state_id"_hs)
            .func<&impl::entity_set_state_id>("set_state_id"_hs)

            .func<&impl::entity_get_state_name>("get_state_name"_hs)
            .func<&impl::entity_set_state_name>("set_state_name"_hs)

            .func<&impl::entity_get_state_index>("get_state_index"_hs)
            .func<&impl::entity_set_state_index>("set_state_index"_hs)

            .func<&impl::entity_get_prev_state_id>("get_prev_state_id"_hs)
            .func<&impl::entity_get_prev_state_name>("get_prev_state_name"_hs)
            .func<&impl::entity_get_prev_state_index>("get_prev_state_index"_hs)

            .func<&impl::entity_get_prev_state_name>("get_prev_state"_hs)

            .func<&impl::entity_get_state_name>("get_state"_hs)
            .func<&impl::entity_set_state_id>("set_state"_hs)
            .func<&impl::entity_set_state_name>("set_state"_hs)
        ;

        auto bind_apply_basis = [&](MetaFunctionID function_id)
        {
            // Matrix overloads:
            entity_type = make_overloads
		    <
                static_cast<math::RotationMatrix(*)(Entity, Registry&, const math::RotationMatrix&, bool)>(&impl::entity_apply_basis),
			    [](auto&&... args) { return impl::entity_apply_basis(std::forward<decltype(args)>(args)...); },
			    3
		    >(entity_type, function_id);

            entity_type = make_overloads
		    <
                static_cast<math::RotationMatrix(*)(Entity, Registry&, const math::RotationMatrix&, float, bool)>(&impl::entity_apply_basis),
			    [](auto&&... args) { return impl::entity_apply_basis(std::forward<decltype(args)>(args)...); },
			    4
		    >(entity_type, function_id);

            // Quaternion overloads:
            entity_type = make_overloads
		    <
                static_cast<math::Quaternion(*)(Entity, Registry&, const math::Quaternion&, bool)>(&impl::entity_apply_basis_q),
			    [](auto&&... args) { return impl::entity_apply_basis_q(std::forward<decltype(args)>(args)...); },
			    3
		    >(entity_type, function_id);

            entity_type = make_overloads
		    <
                static_cast<math::Quaternion(*)(Entity, Registry&, const math::Quaternion&, float, bool)>(&impl::entity_apply_basis_q),
			    [](auto&&... args) { return impl::entity_apply_basis_q(std::forward<decltype(args)>(args)...); },
			    4
		    >(entity_type, function_id);
        };

        auto bind_set_direction_vector = [&](MetaFunctionID function_id)
        {
            entity_type = make_overloads
		    <
                static_cast<math::Vector(*)(Entity, Registry&, const math::Vector&, float, bool, bool, bool)>(&impl::entity_set_direction_vector),
			    [](auto&&... args) { return impl::entity_set_direction_vector(std::forward<decltype(args)>(args)...); },
			    5
		    >(entity_type, function_id);

            entity_type = entity_type
                .func<static_cast<math::Vector(*)(Entity, Registry&, const math::Vector&, float)>(&impl::entity_set_direction_vector)>(function_id)
                .func<static_cast<math::Vector(*)(Entity, Registry&, const math::Vector&)>(&impl::entity_set_direction_vector)>(function_id)
            ;
        };

        bind_apply_basis("apply_basis"_hs);
        bind_apply_basis("rotate"_hs);

        bind_set_direction_vector("set_direction_vector"_hs);
        bind_set_direction_vector("set_direction"_hs);

        entity_type = make_overloads
		<
            &impl::entity_move,
			[](auto&&... args) { return impl::entity_move(std::forward<decltype(args)>(args)...); },
			3
		>(entity_type, "move"_hs);

        entity_type = make_overloads
		<
            static_cast<math::Vector(*)(Entity, Registry&, const math::Vector&, bool)>(&impl::entity_rotate),
			[](auto&&... args) { return impl::entity_rotate(std::forward<decltype(args)>(args)...); },
			3
		>(entity_type, "rotate"_hs);

        entity_type = make_overloads
		<
            static_cast<math::Vector(*)(Entity, Registry&, const math::Vector&, float, bool)>(&impl::entity_rotate),
			[](auto&&... args) { return impl::entity_rotate(std::forward<decltype(args)>(args)...); },
			4
		>(entity_type, "rotate"_hs);

        entity_type = make_overloads
		<
            &impl::entity_rotate_x,
			[](auto&&... args) { return impl::entity_rotate_x(std::forward<decltype(args)>(args)...); },
			3
		>(entity_type, "rotateX"_hs);

        entity_type = make_overloads
		<
            &impl::entity_rotate_y,
			[](auto&&... args) { return impl::entity_rotate_y(std::forward<decltype(args)>(args)...); },
			3
		>(entity_type, "rotateY"_hs);

        entity_type = make_overloads
		<
            &impl::entity_rotate_z,
			[](auto&&... args) { return impl::entity_rotate_z(std::forward<decltype(args)>(args)...); },
			3
		>(entity_type, "rotateZ"_hs);

        entity_type = make_overloads
		<
            static_cast<math::Quaternion(*)(Entity, Registry&, const math::Quaternion&, bool)>(&impl::entity_apply_basis_q),
			[](auto&&... args) { return impl::entity_apply_basis_q(std::forward<decltype(args)>(args)...); },
			3
		>(entity_type, "apply_basis_q"_hs);

        entity_type = make_overloads
		<
            static_cast<math::Quaternion(*)(Entity, Registry&, const math::Quaternion&, float, bool)>(&impl::entity_apply_basis_q),
			[](auto&&... args) { return impl::entity_apply_basis_q(std::forward<decltype(args)>(args)...); },
			4
		>(entity_type, "apply_basis_q"_hs);

        if constexpr (generate_optional_type)
        {
            auto opt_type = custom_optional_type<Entity>(type_name);
        }
    }

    template <>
    void reflect<std::string>()
    {
        custom_meta_type
        <
            std::string,

            MetaTypeReflectionConfig
            {
                .capture_standard_data_members = false, // Standard members not needed.
                .generate_optional_reflection  = true,  // Optional reflection enabled.
                .generate_operator_wrappers    = true,  // TODO: Revisit operator bindings for strings.
                .generate_indirect_getters     = false, // Indirection unsupported.
                .generate_indirect_setters     = false, // Indirection unsupported.
                .generate_json_bindings        = true   // May change this later.
            }
        >
        ("string"_hs) // "std::string"_hs // "String"_hs

        //entt::meta<std::string>().type("string"_hs)

            .func<&impl::add_strings<std::string, std::string>>("operator+"_hs)
            .func<&impl::add_strings<std::string, std::string_view>>("operator+"_hs)
            
            .func<&impl::string_equality<std::string, std::string>>("operator=="_hs)
            .func<&impl::string_equality<std::string, std::string_view>>("operator=="_hs)
            
            .func<&impl::string_inequality<std::string, std::string>>("operator!="_hs)
            .func<&impl::string_inequality<std::string, std::string_view>>("operator!="_hs)

            .func<&impl::string_starts_with_impl<std::string, std::string>>("starts_with"_hs)
            .func<&impl::string_starts_with_impl<std::string, std::string_view>>("starts_with"_hs)

            .func<&impl::string_ends_with_impl<std::string, std::string>>("ends_with"_hs)
            .func<&impl::string_ends_with_impl<std::string, std::string_view>>("ends_with"_hs)

            .func<&impl::string_contains_impl<std::string, std::string>>("contains"_hs)
            .func<&impl::string_contains_impl<std::string, std::string_view>>("contains"_hs)

            .data<nullptr, &std::string::length>("length"_hs)
            .data<nullptr, &std::string::size>("size"_hs)
            .data<nullptr, &std::string::empty>("empty"_hs)

            .ctor<std::string_view>()

            // Constructors for floating-point-to-string conversions:
            .ctor<&impl::arithmetic_to_string_impl<float>>()
            .ctor<&impl::arithmetic_to_string_impl<double>>()
            .ctor<&impl::arithmetic_to_string_impl<long double>>()

            // Constructors for integral-to-string conversions:
            .ctor<&impl::arithmetic_to_string_impl<std::int64_t>>()
            .ctor<&impl::arithmetic_to_string_impl<std::uint64_t>>()
            .ctor<&impl::arithmetic_to_string_impl<std::int32_t>>()
            .ctor<&impl::arithmetic_to_string_impl<std::uint32_t>>()
            .ctor<&impl::arithmetic_to_string_impl<std::int16_t>>()
            .ctor<&impl::arithmetic_to_string_impl<std::uint16_t>>()
            //.ctor<&impl::arithmetic_to_string_impl<std::int8_t>>()
            //.ctor<&impl::arithmetic_to_string_impl<std::uint8_t>>()
            .ctor<&impl::arithmetic_to_string_impl<bool>>()

            .ctor<&impl::entity_to_string_impl>()

            .conv<&impl::from_string_impl<std::int32_t>>()
            .conv<&impl::from_string_impl<std::uint32_t>>()
            .conv<&impl::from_string_impl<std::int64_t>>()
            .conv<&impl::from_string_impl<std::uint64_t>>()
            .conv<&impl::from_string_impl<float>>()
            .conv<&impl::from_string_impl<double>>()
            .conv<&impl::from_string_impl<bool>>()
            
            .conv<&impl::string_bool_impl<std::string>>()

            .conv<std::string_view>()
        ;
    }

    template <>
    void reflect<std::string_view>()
    {
        custom_meta_type
        <
            std::string_view,

            MetaTypeReflectionConfig
            {
                .capture_standard_data_members = false, // Standard members not needed.
                .generate_optional_reflection  = true,  // Optional reflection enabled.
                .generate_operator_wrappers    = true,  // TODO: Revisit operator bindings for strings.
                .generate_indirect_getters     = false, // Indirection unsupported.
                .generate_indirect_setters     = false, // Indirection unsupported.
                .generate_json_bindings        = true   // May change this later.
            }
        >
        ("string_view"_hs) // "std::string_view"_hs // "StringView"_hs
            .func<&impl::add_strings<std::string_view, std::string_view, std::string>>("operator+"_hs)
            .func<&impl::add_strings<std::string_view, std::string, std::string>>("operator+"_hs)

            .func<&impl::string_equality<std::string_view, std::string_view>>("operator=="_hs)
            .func<&impl::string_equality<std::string_view, std::string>>("operator=="_hs)
            
            .func<&impl::string_inequality<std::string_view, std::string_view>>("operator!="_hs)
            .func<&impl::string_inequality<std::string_view, std::string>>("operator!="_hs)

            .func<&impl::string_starts_with_impl<std::string_view, std::string_view>>("starts_with"_hs)
            .func<&impl::string_starts_with_impl<std::string_view, std::string>>("starts_with"_hs)

            .func<&impl::string_ends_with_impl<std::string_view, std::string_view>>("ends_with"_hs)
            .func<&impl::string_ends_with_impl<std::string_view, std::string>>("ends_with"_hs)

            .func<&impl::string_contains_impl<std::string_view, std::string_view>>("contains"_hs)
            .func<&impl::string_contains_impl<std::string_view, std::string>>("contains"_hs)

            .data<nullptr, &std::string_view::length>("length"_hs)
            .data<nullptr, &std::string_view::size>("size"_hs)
            .data<nullptr, &std::string_view::empty>("empty"_hs)

            .ctor<std::string_view>()

            // NOTE: May remove this constructor overload due to possible
            // memory-safety concerns around string lifetimes.
            .ctor<std::string>()

            .conv<&impl::string_bool_impl<std::string_view>>()
        ;
    }

    template <>
    void reflect<std::chrono::system_clock::duration>()
    {
        using T = std::chrono::system_clock::duration;

        constexpr bool generate_optional_type = true;
        constexpr bool sync_context = true;

        if constexpr (sync_context)
        {
            // Ensure that we're using the correct context.
            sync_reflection_context();
        }

        entt::meta<T>()
            //.type("Duration"_hs)
            
            // Constructor to convert from floating-point values (seconds) to STL system-clock duration values.
            // (Commonly aliased via `Timer::Duration`)
            .ctor<static_cast<T(*)(float)>(&Timer::to_duration)>()
            
            .ctor<&impl::duration_from_integer>()

            .ctor<&impl::duration_from_string_view>()
            .ctor<&impl::duration_from_string>()
        ;

        if constexpr (generate_optional_type)
        {
            constexpr auto type_name = entt::type_name<T>::value(); // std::string_view { "Duration" };

            auto opt_type = custom_optional_type<T>(type_name);
        }
    }

    // TODO: Split STL types into their own reflection submodule.
    static void reflect_stl()
    {
        reflect<std::string>();
        reflect<std::string_view>();
        reflect<std::chrono::system_clock::duration>();
    }

    template <>
    void reflect<GraphicsConfig>()
    {
        engine_meta_type<GraphicsConfig>()
            .data<&GraphicsConfig::shadows>("shadows"_hs)
            .data<&GraphicsConfig::parallax>("parallax"_hs)
        ;

        engine_meta_type<GraphicsConfig::Shadows>()
            .data<&GraphicsConfig::Shadows::resolution>("resolution"_hs)
            .data<&GraphicsConfig::Shadows::cubemap_resolution>("cubemap_resolution"_hs)
            .data<&GraphicsConfig::Shadows::enabled>("enabled"_hs)
        ;

        engine_meta_type<GraphicsConfig::Parallax>()
            .data<&GraphicsConfig::Parallax::min_layers>("min_layers"_hs)
            .data<&GraphicsConfig::Parallax::max_layers>("max_layers"_hs)
        ;
    }

    template <>
    void reflect<ObjectConfig>()
    {
        engine_meta_type<ObjectConfig>()
            .data<&ObjectConfig::object_path>("object_path"_hs)
        ;
    }

    template <>
    void reflect<PlayerConfig>()
    {
        engine_meta_type<PlayerConfig>()
            .data<&PlayerConfig::default_player>("default_player"_hs)
            .data<&PlayerConfig::character_path>("character_path"_hs)
        ;

        engine_meta_type<PlayerConfig::Player>()
            .data<&PlayerConfig::Player::name>("name"_hs)
            .data<&PlayerConfig::Player::character>("character"_hs)
        ;
    }

    template <>
    void reflect<EntityConfig>()
    {
        engine_meta_type<EntityConfig>()
            .data<&EntityConfig::archetype_path>("archetype_path"_hs)
        ;
    }

    template <>
    void reflect<Config>()
    {
        engine_meta_type<Config>()
            .data<&Config::graphics>("graphics"_hs)
            .data<&Config::objects>("objects"_hs)
            .data<&Config::players>("players"_hs)
            .data<&Config::entities>("entities"_hs)
        ;
    }

    template <>
    void reflect<entt::meta_sequence_container>()
    {
        constexpr bool sync_context = true;

        if constexpr (sync_context)
        {
            // Ensure that we're using the correct context.
            sync_reflection_context();
        }

        entt::meta<entt::meta_sequence_container>()
            .data<nullptr, &entt::meta_sequence_container::value_type>("value_type"_hs)
            .data<&entt::meta_sequence_container::resize, &entt::meta_sequence_container::size>("size"_hs)

            //.data<nullptr, &entt::meta_sequence_container::begin>("begin"_hs)
            //.data<nullptr, &entt::meta_sequence_container::end>("end"_hs)

            .func<&entt::meta_sequence_container::clear>("clear"_hs)

            .func<&entt::meta_sequence_container::begin>("begin"_hs)
            .func<&entt::meta_sequence_container::end>("end"_hs)
            
            .func<&impl::meta_sequence_container_at_impl>("at"_hs)
            .func<&impl::meta_sequence_container_at_impl>("index"_hs)

            // NOTE: May need to make a wrapper function for this.
            .func<&entt::meta_sequence_container::insert>("insert"_hs)

            .func<&entt::meta_sequence_container::erase>("erase"_hs)
            .func<&entt::meta_sequence_container::operator[]>("operator[]"_hs)
            .func<&entt::meta_sequence_container::operator bool>("operator bool"_hs)

            .func<&impl::meta_sequence_container_push_back_impl>("push_back"_hs)
            .func<&impl::meta_sequence_container_push_back_impl>("emplace_back"_hs)

            .func<&impl::meta_sequence_container_find_impl>("find"_hs)
        ;
    }

    template <>
    void reflect<entt::meta_sequence_container::iterator>()
    {
        constexpr bool sync_context = true;

        if constexpr (sync_context)
        {
            // Ensure that we're using the correct context.
            sync_reflection_context();
        }

        entt::meta<entt::meta_sequence_container::iterator>()
            .func<static_cast<entt::meta_sequence_container::iterator&(entt::meta_sequence_container::iterator::*)()>(&entt::meta_sequence_container::iterator::operator++)>("operator++"_hs)
            .func<static_cast<entt::meta_sequence_container::iterator(entt::meta_sequence_container::iterator::*)(int)>(&entt::meta_sequence_container::iterator::operator++)>("operator++"_hs)

            .func<static_cast<entt::meta_sequence_container::iterator& (entt::meta_sequence_container::iterator::*)()>(&entt::meta_sequence_container::iterator::operator--)>("operator--"_hs)
            .func<static_cast<entt::meta_sequence_container::iterator(entt::meta_sequence_container::iterator::*)(int)>(&entt::meta_sequence_container::iterator::operator--)>("operator--"_hs)

            /*
            .func<&entt::meta_sequence_container::iterator::operator bool>("operator bool"_hs)
            .func<&entt::meta_sequence_container::iterator::operator==>("operator=="_hs)
            .func<&entt::meta_sequence_container::iterator::operator!=>("operator!="_hs)
            */

            .func<&entt::meta_sequence_container::iterator::operator*>("*operator"_hs)
        ;
    }

    template <>
    void reflect<entt::meta_associative_container>()
    {
        constexpr bool sync_context = true;

        if constexpr (sync_context)
        {
            // Ensure that we're using the correct context.
            sync_reflection_context();
        }

        auto type = entt::meta<entt::meta_associative_container>()
            .data<nullptr, &entt::meta_associative_container::key_only>("key_only"_hs)
            .data<nullptr, &entt::meta_associative_container::key_type>("key_type"_hs)
            .data<nullptr, &entt::meta_associative_container::mapped_type>("mapped_type"_hs)
            .data<nullptr, &entt::meta_associative_container::value_type>("value_type"_hs)
            .data<nullptr, &entt::meta_associative_container::size>("size"_hs)
            
            //.data<nullptr, &entt::meta_associative_container::begin>("begin"_hs)
            //.data<nullptr, &entt::meta_associative_container::end>("end"_hs)

            .func<&entt::meta_associative_container::clear>("clear"_hs)
            .func<&entt::meta_associative_container::begin>("begin"_hs)
            .func<&entt::meta_associative_container::end>("end"_hs)

            .func
            <
                static_cast<bool(entt::meta_associative_container::*)(MetaAny, MetaAny)>
                (&entt::meta_associative_container::insert)
            >("insert"_hs)

            .func<&entt::meta_associative_container::erase>("erase"_hs)
            .func<&entt::meta_associative_container::find>("find"_hs)

            .func<&entt::meta_associative_container::operator bool>("operator bool"_hs)
        ;

        type = make_overloads
        <
            &entt::meta_associative_container::insert,
            [](auto& self, auto&&... args) { return self.insert(std::forward<decltype(args)>(args)...); },
            2
        >(type, "insert"_hs);
    }

    template <>
    void reflect<entt::meta_associative_container::iterator>()
    {
        constexpr bool sync_context = true;

        if constexpr (sync_context)
        {
            // Ensure that we're using the correct context.
            sync_reflection_context();
        }

        entt::meta<entt::meta_associative_container::iterator>()
            .func<static_cast<entt::meta_associative_container::iterator&(entt::meta_associative_container::iterator::*)()>(&entt::meta_associative_container::iterator::operator++)>("operator++"_hs)
            .func<static_cast<entt::meta_associative_container::iterator(entt::meta_associative_container::iterator::*)(int)>(&entt::meta_associative_container::iterator::operator++)>("operator++"_hs)

            /*

            .func<&entt::meta_associative_container::iterator::operator bool>("operator bool"_hs)
            .func<&entt::meta_associative_container::iterator::operator==>("operator=="_hs)
            .func<&entt::meta_associative_container::iterator::operator!=>("operator!="_hs)
            */

            .func<&entt::meta_associative_container::iterator::operator*>("*operator"_hs)

            // Currently unsupported:
            //.func<static_cast<entt::meta_associative_container::iterator& (entt::meta_associative_container::iterator::*)()>(&entt::meta_associative_container::iterator::operator--)>("operator--"_hs)
            //.func<static_cast<entt::meta_associative_container::iterator(entt::meta_associative_container::iterator::*)(int)>(&entt::meta_associative_container::iterator::operator--)>("operator--"_hs)
        ;
    }

    void reflect_entt()
    {
        reflect<entt::meta_sequence_container>();
        reflect<entt::meta_sequence_container::iterator>();
        reflect<entt::meta_associative_container>();
        reflect<entt::meta_associative_container::iterator>();
    }

    void reflect_dependencies()
    {
        reflect_entt();
        reflect_stl();

        reflect<engine::Math>();
        reflect<engine::Util>();
        reflect<engine::Script>();
    }

    void reflect_primitives()
    {
        reflect<Entity>();

        reflect<EntityType>();

        // See: `std::chrono::system_clock::duration` in `reflect_stl`.
        //reflect<Timer::Duration>();

        //reflect<LightProperties>();
        //reflect<Axis>(); // RotationAxis

        // Primitive type extensions:
        extend_language_primitive_type<std::int64_t>();
        extend_language_primitive_type<std::uint64_t>();
        extend_language_primitive_type<std::int32_t>();
        extend_language_primitive_type<std::uint32_t>();
        extend_language_primitive_type<std::int16_t>();
        extend_language_primitive_type<std::uint16_t>();
        //extend_language_primitive_type<std::int8_t>();
        //extend_language_primitive_type<std::uint8_t>();

        extend_language_primitive_type<float>();
        extend_language_primitive_type<double>();
        extend_language_primitive_type<long double>();

        extend_language_primitive_type<bool>();
    }
    
    void reflect_all(bool primitives, bool dependencies)
    {
        // NOTE: Not thread-safe. (Shouldn't matter for this use-case, though)
        static bool reflection_generated = false;

        if (reflection_generated)
        {
            return;
        }

        reflect_meta();

        if (dependencies)
        {
            reflect_dependencies();
        }

        if (primitives)
        {
            reflect_primitives();
        }

        reflect_exported_functions();

        reflect_core_components();
        reflect_core_commands();

        reflect<ResourceManager>();

        reflect_systems();

        reflect<World>();

        reflect<GraphicsConfig>();
        reflect<ObjectConfig>();
        reflect<PlayerConfig>();
        reflect<EntityConfig>();
        reflect<Config>();

        // ...

        reflection_generated = true;
    }

    template <>
    void reflect<void>()
    {
        reflect_all();
    }
}