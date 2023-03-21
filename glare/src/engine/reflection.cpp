// This file aggregates all `engine`-related reflection headers into one translation unit. 
// The purpose of which is to provide a single implementation-file for meta-type instantiation. (see `reflect_all`)
//
// TODO: Rework this source file into some form of automated 'reflection generation' procedure in the build process.

#include "reflection.hpp"

#include "components/reflection.hpp"
#include "commands/reflection.hpp"

#include "meta/reflection.hpp"
#include "resource_manager/reflection.hpp"
#include "entity/reflection.hpp"
#include "debug/reflection.hpp"
#include "input/reflection.hpp"
#include "world/reflection.hpp"

#include "config.hpp"
#include "timer.hpp"

#include <math/reflection.hpp>

#include <util/format.hpp>
#include <util/string.hpp>

#include <utility>
#include <optional>
#include <string>
#include <cstdint>
#include <type_traits>

//#include <entt/meta/meta.hpp>
//#include <entt/entt.hpp>

namespace engine
{
    entt::locator<entt::meta_ctx>::node_type get_shared_reflection_handle()
    {
        return entt::locator<entt::meta_ctx>::handle();
    }

    void reflect_systems()
    {
        reflect<DebugListener>();
        reflect<EntitySystem>();
        reflect<InputSystem>();
        reflect<World>();

        // ...
    }

    static std::string add_strings(const std::string& a, const std::string& b)
    {
        return (a + b);
    }

    static bool string_equality(const std::string a, const std::string& b)
    {
        return (a == b);
    }

    static bool string_inequality(const std::string a, const std::string& b)
    {
        return (a != b);
    }

    static std::string entity_to_string_impl(Entity entity)
    {
        return util::format("Entity #{}", entity);
    }

    template <typename ArithmeticType, typename=std::enable_if_t<std::is_arithmetic_v<ArithmeticType>>>
    static std::string arithmetic_to_string_impl(ArithmeticType value) // const ArithmeticType&
    {
        if constexpr (std::is_same_v<std::decay_t<ArithmeticType>, bool>)
        {
            if (value)
            {
                return "true";
            }
            else
            {
                return "false";
            }
        }
        else
        {
            return std::to_string(value);
        }
    }

    template <typename T>
    static T from_string_view_impl(std::string_view value) // const std::string_view&
    {
        // TODO: Determine if `bool` should have different behavior here.
        // (e.g. checking against `value.empty()` instead of looking for `true`-like values)

        if (auto result = util::from_string<T>(value, true))
        {
            return *result;
        }

        return {};
    }

    template <typename ToType, typename FromType>
    static ToType static_cast_impl(const FromType& from_value)
    {
        return static_cast<ToType>(from_value);
    }

    template <typename T>
    static T from_string_impl(const std::string& value)
    {
        return from_string_view_impl<T>(value);
    }

    template <typename PrimitiveType>
    static auto extend_language_primitive_type()
    {
        auto type = entt::meta<PrimitiveType>()
            .ctor<&from_string_view_impl<PrimitiveType>>()
            .ctor<&from_string_impl<PrimitiveType>>()
            .ctor<&static_cast_impl<PrimitiveType, float>>()
            .ctor<&static_cast_impl<PrimitiveType, double>>()
            .ctor<&static_cast_impl<PrimitiveType, long double>>()
            .ctor<&static_cast_impl<PrimitiveType, std::int64_t>>()
            .ctor<&static_cast_impl<PrimitiveType, std::uint64_t>>()
            .ctor<&static_cast_impl<PrimitiveType, std::int32_t>>()
            .ctor<&static_cast_impl<PrimitiveType, std::uint32_t>>()
            .ctor<&static_cast_impl<PrimitiveType, std::int16_t>>()
            .ctor<&static_cast_impl<PrimitiveType, std::uint16_t>>()
            //.ctor<&static_cast_impl<PrimitiveType, std::int8_t>>()
            //.ctor<&static_cast_impl<PrimitiveType, std::uint8_t>>()

            .conv<&arithmetic_to_string_impl<PrimitiveType>>()
        ;

        return type;
    }

    template <>
    void reflect<std::string>()
    {
        custom_meta_type<std::string>
        (
            "string"_hs, // "std::string"_hs // "String"_hs
            false, // Standard members not needed.
            true,  // Optional reflection enabled.
            true,  // TODO: Revisit operator bindings for strings.
            false, // Indirection unsupported.
            false  // Indirection unsupported.
        )

        //entt::meta<std::string>().type("string"_hs)

            .func<&add_strings>("operator+"_hs)
            .func<&string_equality>("operator=="_hs)
            .func<&string_inequality>("operator!="_hs)

            // Constructors for floating-point-to-string conversions:
            .ctor<&arithmetic_to_string_impl<float>>()
            .ctor<&arithmetic_to_string_impl<double>>()
            .ctor<&arithmetic_to_string_impl<long double>>()

            // Constructors for integral-to-string conversions:
            .ctor<&arithmetic_to_string_impl<std::int64_t>>()
            .ctor<&arithmetic_to_string_impl<std::uint64_t>>()
            .ctor<&arithmetic_to_string_impl<std::int32_t>>()
            .ctor<&arithmetic_to_string_impl<std::uint32_t>>()
            .ctor<&arithmetic_to_string_impl<std::int16_t>>()
            .ctor<&arithmetic_to_string_impl<std::uint16_t>>()
            //.ctor<&arithmetic_to_string_impl<std::int8_t>>()
            //.ctor<&arithmetic_to_string_impl<std::uint8_t>>()

            .ctor<&entity_to_string_impl>()

            .conv<&from_string_impl<std::int32_t>>()
            .conv<&from_string_impl<std::uint32_t>>()
            .conv<&from_string_impl<std::int64_t>>()
            .conv<&from_string_impl<std::uint64_t>>()
            .conv<&from_string_impl<float>>()
            .conv<&from_string_impl<double>>()
            .conv<&from_string_impl<bool>>()
        ;
    }

    template <>
    void reflect<std::chrono::system_clock::duration>()
    {
        entt::meta<std::chrono::system_clock::duration>()
            // Constructor to convert from floating-point values (seconds) to STL system-clock duration values.
            // (Commonly aliased via `Timer::Duration`)
            .ctor<static_cast<std::chrono::system_clock::duration(*)(float)>(&Timer::to_duration)>()
        ;
    }

    static void reflect_stl()
    {
        reflect<std::string>();
        reflect<std::chrono::system_clock::duration>();
    }

    template <typename T>
    auto reflect_math_type(auto type_name)
    {
        auto type = math::reflect<T>(hash(type_name));

        auto opt_type = entt::meta<std::optional<T>>()
            .type(hash(optional_name(type_name)))
            //.template func<&from_optional<T>>("from_optional"_hs)
            .template func<&type_id_from_optional<T>>("type_id_from_optional"_hs)
            .template func<&type_from_optional<T>>("type_from_optional"_hs)
        ;

        if constexpr (std::is_copy_constructible_v<T>)
        {
            opt_type = opt_type.ctor<T>(); // const T&
        }

        /*
        if constexpr (std::is_move_constructible_v<T>)
        {
            opt_type = opt_type.ctor<T&&>();
        }
        */

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
        reflect_math_type<math::Vector3D>("Vector"); // "Vector3D"
    }

    // Reflects `math::Vector4D` with the generalized name of `Vector4D`.
    // 
    // TODO: Look into migrating this to another file/header.
    template <>
    void reflect<math::Vector4D>()
    {
        reflect_math_type<math::Vector4D>("Vector4D");
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
        auto type = entt::meta<entt::meta_sequence_container>()
            .data<nullptr, &entt::meta_sequence_container::value_type>("value_type"_hs)
            .data<&entt::meta_sequence_container::resize, &entt::meta_sequence_container::size>("size"_hs)

            //.data<nullptr, &entt::meta_sequence_container::begin>("begin"_hs)
            //.data<nullptr, &entt::meta_sequence_container::end>("end"_hs)

            .func<&entt::meta_sequence_container::clear>("clear"_hs)
            .func<&entt::meta_sequence_container::begin>("begin"_hs)
            .func<&entt::meta_sequence_container::end>("end"_hs)

            // NOTE: May need to make a wrapper function for this.
            .func<&entt::meta_sequence_container::insert>("insert"_hs)

            .func<&entt::meta_sequence_container::erase>("erase"_hs)
            .func<&entt::meta_sequence_container::operator[]>("operator[]"_hs)
            .func<&entt::meta_sequence_container::operator bool>("operator bool"_hs)

            // TODO: Add custom `find` member-function.
        ;
    }

    template <>
    void reflect<entt::meta_associative_container>()
    {
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
                static_cast<bool(entt::meta_associative_container::*)(entt::meta_any)>
                (&entt::meta_associative_container::insert)
            >("insert"_hs)

            .func
            <
                static_cast<bool(entt::meta_associative_container::*)(entt::meta_any, entt::meta_any)>
                (&entt::meta_associative_container::insert)
            >("insert"_hs)

            .func<&entt::meta_associative_container::erase>("erase"_hs)
            .func<&entt::meta_associative_container::find>("find"_hs)

            .func<&entt::meta_sequence_container::operator bool>("operator bool"_hs)
        ;
    }

    // TODO: Implement reflection for matrix types.
    void reflect_math()
    {
        reflect<math::Vector2D>();
        reflect<math::Vector3D>();
        reflect<math::Vector4D>();

        reflect<math::vec2i>();

        // ...
    }

    void reflect_entt()
    {
        reflect<entt::meta_sequence_container>();
        reflect<entt::meta_associative_container>();
    }

    void reflect_dependencies()
    {
        reflect_entt();
        reflect_stl();
        reflect_math();
    }

    void reflect_primitives()
    {
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

        reflect<GraphicsConfig>();
        reflect<ObjectConfig>();
        reflect<PlayerConfig>();
        reflect<EntityConfig>();
        reflect<Config>();

        // ...

        reflection_generated = true;
    }
}