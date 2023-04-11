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
#include <string_view>
#include <cstdint>
#include <type_traits>
#include <algorithm>

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

        // ...
    }

    static entt::meta_sequence_container::iterator meta_sequence_container_push_back_impl(entt::meta_sequence_container& container, MetaAny value) // const MetaAny&
    {
        /*
        // Alternative implementation:
        auto it = (container.begin());

        for (std::size_t i = 0; i < (container.size() - 1); i++)
        {
            it = it++;
        }
        */

        /*
        // Alternative implementation:
        auto it = (container.begin());

        const auto current_container_size = container.size();

        if (current_container_size > 0)
        {
            it.operator++(static_cast<int>(current_container_size - 1));
        }
        */

        auto it = container.end();

        return container.insert(it, std::move(value));
    }

    static entt::meta_sequence_container::iterator meta_sequence_container_at_impl(entt::meta_sequence_container& container, std::int32_t index) // std::size_t // const
    {
        auto it = container.begin();

        if (index > 0)
        {
            it.operator++((index - 1));
        }

        return it;
    }

    static entt::meta_sequence_container::iterator meta_sequence_container_find_impl(entt::meta_sequence_container& container, const MetaAny& value) // const
    {
        return std::find(container.begin(), container.end(), value);
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

    static Entity entity_from_integer(std::underlying_type_t<Entity> value)
    {
        return static_cast<Entity>(value);
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

    template <typename PrimitiveType, bool generate_optional_type=true>
    static auto extend_language_primitive_type(bool sync_context=true)
    {
        if (sync_context)
        {
            // Ensure that we're using the correct context.
            sync_reflection_context();
        }

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
            .conv<&entity_to_string_impl>()
            //.conv<std::underlying_type_t<Entity>>()
            .ctor<&entity_from_integer>()
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

            .func<&add_strings>("operator+"_hs)
            .func<&string_equality>("operator=="_hs)
            .func<&string_inequality>("operator!="_hs)

            .ctor<std::string_view>()

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

    template <typename T, bool generate_optional_type=true>
    auto reflect_math_type(auto type_name, bool sync_context=true)
    {
        if (sync_context)
        {
            // Ensure that we're using the correct context.
            sync_reflection_context();
        }

        auto type = math::reflect<T>(hash(type_name));

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
            
            .func<&meta_sequence_container_at_impl>("at"_hs)
            .func<&meta_sequence_container_at_impl>("index"_hs)

            // NOTE: May need to make a wrapper function for this.
            .func<&entt::meta_sequence_container::insert>("insert"_hs)

            .func<&entt::meta_sequence_container::erase>("erase"_hs)
            .func<&entt::meta_sequence_container::operator[]>("operator[]"_hs)
            .func<&entt::meta_sequence_container::operator bool>("operator bool"_hs)

            .func<&meta_sequence_container_push_back_impl>("push_back"_hs)
            .func<&meta_sequence_container_push_back_impl>("emplace_back"_hs)

            .func<&meta_sequence_container_find_impl>("find"_hs)
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