// This file aggregates all `engine`-related reflection headers into one translation unit. 
// The purpose of which is to provide a single implementation-file for meta-type instantiation. (see `reflect_all`)
//
// TODO: Rework this source file into some form of automated 'reflection generation' procedure in the build process.

#include "reflection.hpp"

#include "container_extensions.hpp"
#include "entity_extensions.hpp"
#include "math_extensions.hpp"
#include "primitive_extensions.hpp"
#include "string_extensions.hpp"

#include "string_conversion.hpp"

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

#include <math/reflection.hpp>
#include <math/format.hpp>
#include <math/types.hpp>

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

    template <typename PrimitiveType, bool generate_optional_type=true>
    static auto extend_language_primitive_type(bool sync_context=true)
    {
        if (sync_context)
        {
            // Ensure that we're using the correct context.
            sync_reflection_context();
        }

        auto type = entt::meta<PrimitiveType>()
            .ctor<&impl::from_string_view_impl<PrimitiveType>>()
            .ctor<&impl::from_string_impl<PrimitiveType>>()
            .ctor<&impl::static_cast_impl<PrimitiveType, float>>()
            .ctor<&impl::static_cast_impl<PrimitiveType, double>>()
            .ctor<&impl::static_cast_impl<PrimitiveType, long double>>()
            .ctor<&impl::static_cast_impl<PrimitiveType, std::int64_t>>()
            .ctor<&impl::static_cast_impl<PrimitiveType, std::uint64_t>>()
            .ctor<&impl::static_cast_impl<PrimitiveType, std::int32_t>>()
            .ctor<&impl::static_cast_impl<PrimitiveType, std::uint32_t>>()
            .ctor<&impl::static_cast_impl<PrimitiveType, std::int16_t>>()
            .ctor<&impl::static_cast_impl<PrimitiveType, std::uint16_t>>()
            //.ctor<&impl::static_cast_impl<PrimitiveType, std::int8_t>>()
            //.ctor<&impl::static_cast_impl<PrimitiveType, std::uint8_t>>()

            .conv<&impl::arithmetic_to_string_impl<PrimitiveType>>()
        ;

        if constexpr (generate_optional_type)
        {
            constexpr auto type_name = entt::type_name<PrimitiveType>::value();

            auto opt_type = optional_custom_meta_type<PrimitiveType>(type_name);
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

        entt::meta<Entity>()
            .type("Entity"_hs)
            
            .conv<&impl::entity_to_string_impl>()
            //.conv<std::underlying_type_t<Entity>>()
            
            .ctor<&impl::entity_from_integer>()

            .func<&impl::entity_set_position>("set_position"_hs)
            .func<&impl::entity_get_position>("get_position"_hs)

            .func<&impl::entity_set_local_position>("set_local_position"_hs)
            .func<&impl::entity_get_local_position>("get_local_position"_hs)

            .func<&impl::entity_set_rotation>("set_rotation"_hs)
            .func<&impl::entity_get_rotation>("get_rotation"_hs)

            .func<&impl::entity_set_local_rotation>("set_local_rotation"_hs)
            .func<&impl::entity_get_local_rotation>("get_local_rotation"_hs)

            .func<&impl::entity_get_direction_vector>("get_direction_vector"_hs)
            .func<&impl::entity_set_direction_vector>("set_direction_vector"_hs)

            .func<&impl::entity_set_scale>("set_scale"_hs)
            .func<&impl::entity_get_scale>("get_scale"_hs)

            .func<&impl::entity_set_local_scale>("set_local_scale"_hs)
            .func<&impl::entity_get_local_scale>("get_local_scale"_hs)
        ;

        if constexpr (generate_optional_type)
        {
            constexpr auto type_name = std::string_view { "Entity" }; // entt::type_name<Entity>::value();

            auto opt_type = optional_custom_meta_type<Entity>(type_name);
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
                .generate_json_constructor     = true   // May change this later.
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

            .ctor<&impl::entity_to_string_impl>()

            .conv<&impl::from_string_impl<std::int32_t>>()
            .conv<&impl::from_string_impl<std::uint32_t>>()
            .conv<&impl::from_string_impl<std::int64_t>>()
            .conv<&impl::from_string_impl<std::uint64_t>>()
            .conv<&impl::from_string_impl<float>>()
            .conv<&impl::from_string_impl<double>>()
            .conv<&impl::from_string_impl<bool>>()
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
                .generate_json_constructor     = true   // May change this later.
            }
        >
        ("string_view"_hs) // "std::string_view"_hs // "StringView"_hs
            .func<&impl::add_strings<std::string_view, std::string_view, std::string>>("operator+"_hs)
            .func<&impl::add_strings<std::string_view, std::string, std::string>>("operator+"_hs)

            .func<&impl::string_equality<std::string_view, std::string_view>>("operator=="_hs)
            .func<&impl::string_equality<std::string_view, std::string>>("operator=="_hs)
            
            .func<&impl::string_inequality<std::string_view, std::string_view>>("operator!="_hs)
            .func<&impl::string_inequality<std::string_view, std::string>>("operator!="_hs)

            .ctor<std::string_view>()

            // NOTE: May remove this constructor overload due to possible
            // memory-safety concerns around string lifetimes.
            .ctor<std::string>()
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
        ;

        if constexpr (generate_optional_type)
        {
            constexpr auto type_name = entt::type_name<T>::value(); // std::string_view { "Duration" };

            auto opt_type = optional_custom_meta_type<T>(type_name);
        }
    }

    static void reflect_stl()
    {
        reflect<std::string>();
        reflect<std::string_view>();
        reflect<std::chrono::system_clock::duration>();
    }

    template
    <
        typename T,
        
        bool generate_optional_type=true,
        bool generate_operators=true,
        bool generate_standard_methods=true
    >
    auto reflect_math_type(auto type_name, bool sync_context=true)
    {
        if (sync_context)
        {
            // Ensure that we're using the correct context.
            sync_reflection_context();
        }

        auto type = math::reflect<T>(hash(type_name));

        if constexpr (generate_operators)
        {
            type = type
                .func<&impl::add_impl<T>>("operator+"_hs)
                .func<&impl::subtract_impl<T>>("operator-"_hs)
                .func<&impl::multiply_impl<T>>("operator*"_hs)
            ;
        }

        // NOTE: This check doesn't work due to non-type template parameters for `glm::vec`.
        // TODO: Revisit the idea of checking for vector types.
        //if constexpr (util::is_specialization_v<T, glm::vec>)
        if constexpr (std::is_same_v<T, math::Vector2D> || std::is_same_v<T, math::Vector3D> || std::is_same_v<T, math::Vector4D>) // || std::is_same_v<T, math::vec2i>
        {
            if constexpr (generate_operators)
            {
                type = type.func<&impl::divide_impl<T>>("operator/"_hs);
            }

            type = type.conv<&impl::format_string_conv<T>>();

            if constexpr (generate_standard_methods)
            {
                using value_type = typename T::value_type;

                if constexpr (std::is_same_v<value_type, float>)
                {
                    type = type
                        .func<&impl::normalize_impl<T>>("normalize"_hs)
                    ;
                }
            }
        }

        if constexpr (generate_optional_type)
        {
            auto opt_type = optional_custom_meta_type<T>(type_name);
        }

        return type;
    }

    // Reflects `math::Vector2D` with the generalized name of `Vector2D`.
    // 
    // TODO: Look into migrating this to another file/header.
    template <>
    void reflect<math::Vector2D>()
    {
        reflect_math_type<math::Vector2D>("Vector2D");
    }

    // Reflects `math::Vector3D` with the generalized name of `Vector`.
    // 
    // TODO: Look into migrating this to another file/header.
    template <>
    void reflect<math::Vector3D>()
    {
        reflect_math_type<math::Vector3D>("Vector") // "Vector3D"
            .ctor<&impl::vec3_from_vec2>()
        ;
    }

    // Reflects `math::Vector4D` with the generalized name of `Vector4D`.
    // 
    // TODO: Look into migrating this to another file/header.
    template <>
    void reflect<math::Vector4D>()
    {
        reflect_math_type<math::Vector4D>("Vector4D")
            .ctor<&impl::vec4_from_vec2>()
            .ctor<&impl::vec4_from_vec3>()
        ;
    }

    // Reflects `math::vec2i` with the generalized name of `vec2i`.
    // 
    // TODO: Look into migrating this to another file/header.
    template <>
    void reflect<math::vec2i>()
    {
        reflect_math_type<math::vec2i>("vec2i");
    }

    template <>
    void reflect<math::Matrix4x4>()
    {
        reflect_math_type<math::Matrix4x4>("Matrix") // "Matrix4x4"
            .func<&impl::mat4_vec4_multiply_impl>("operator*"_hs)
            .func<&impl::mat4_vec3_multiply_impl>("operator*"_hs)
            .func<&impl::mat_quat_multiply_impl<math::Matrix4x4>>("operator*"_hs)
            .func<&math::identity<math::Matrix4x4>>("identity"_hs)
            .func<&impl::inverse_impl<math::Matrix4x4>>("inverse"_hs)
            .func<&impl::transpose_impl<math::Matrix4x4>>("transpose"_hs)
            .func<&impl::mat4_translate_impl>("translate"_hs)
        ;
    }

    template <>
    void reflect<math::Matrix3x3>()
    {
        reflect_math_type<math::Matrix3x3>("Matrix3x3")
            .func<&impl::multiply_impl<math::Matrix3x3, math::Vector3D>>("operator*"_hs)
            .func<&impl::mat_quat_multiply_impl<math::Matrix3x3>>("operator*"_hs)
            .func<&math::identity<math::Matrix3x3>>("identity"_hs)
            .func<&impl::inverse_impl<math::Matrix3x3>>("inverse"_hs)
            .func<&impl::transpose_impl<math::Matrix3x3>>("transpose"_hs)
        ;
    }

    template <>
    void reflect<math::Quaternion>()
    {
        reflect_math_type<math::Quaternion>("Quaternion")
            .func<&impl::quat_vec3_multiply_impl>("operator*"_hs)
            .func<&impl::quat_vec4_multiply_impl>("operator*"_hs)
            .func<&impl::inverse_impl<math::Quaternion>>("inverse"_hs)
            .conv<&math::quaternion_to_matrix<math::Matrix3x3>>()
            .conv<&math::quaternion_to_matrix<math::Matrix4x4>>()
            .data<&math::Quaternion::x>("x"_hs)
            .data<&math::Quaternion::y>("y"_hs)
            .data<&math::Quaternion::z>("z"_hs)
            .data<&math::Quaternion::w>("w"_hs)
            .ctor<math::Vector3D>()
        ;
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
                static_cast<bool(entt::meta_associative_container::*)(MetaAny)>
                (&entt::meta_associative_container::insert)
            >("insert"_hs)

            .func
            <
                static_cast<bool(entt::meta_associative_container::*)(MetaAny, MetaAny)>
                (&entt::meta_associative_container::insert)
            >("insert"_hs)

            .func<&entt::meta_associative_container::erase>("erase"_hs)
            .func<&entt::meta_associative_container::find>("find"_hs)

            .func<&entt::meta_associative_container::operator bool>("operator bool"_hs)
        ;
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

    // TODO: Implement reflection for matrix types.
    void reflect_math()
    {
        reflect<math::Vector2D>();
        reflect<math::Vector3D>();
        reflect<math::Vector4D>();

        reflect<math::vec2i>();

        reflect<math::Matrix4x4>();
        reflect<math::Matrix3x3>();

        // ...
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
        reflect_math();
    }

    void reflect_primitives()
    {
        reflect<Entity>();

        reflect<EntityType>();
        reflect<LightType>();

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
}