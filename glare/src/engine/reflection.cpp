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

#include <math/reflection.hpp>

#include <util/format.hpp>

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

    /*
    template <>
    void reflect<std::string>()
    {
        engine_meta_type<std::string>();
    }

    static void reflect_stl()
    {
        reflect<std::string>();
    }
    */

    // Reflects `math::Vector2D` with the generalized name of `Vector2D`.
    // 
    // TODO: Look into migrating this to another file/header.
    template <>
    void reflect<math::Vector2D>()
    {
        math::reflect<math::Vector2D>("Vector2D"_hs);
    }

    // Reflects `math::Vector3D` with the generalized name of `Vector`.
    // 
    // TODO: Look into migrating this to another file/header.
    template <>
    void reflect<math::Vector3D>()
    {
        math::reflect<math::Vector3D>("Vector"_hs); // "Vector3D"_hs
    }

    // Reflects `math::Vector4D` with the generalized name of `Vector4D`.
    // 
    // TODO: Look into migrating this to another file/header.
    template <>
    void reflect<math::Vector4D>()
    {
        math::reflect<math::Vector4D>("Vector4D"_hs);
    }

    // Reflects `math::vec2i` with the generalized name of `vec2i`.
    // 
    // TODO: Look into migrating this to another file/header.
    template <>
    void reflect<math::vec2i>()
    {
        math::reflect<math::vec2i>("vec2i"_hs);
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
    void reflect<Config>()
    {
        engine_meta_type<Config>()
            .data<&Config::graphics>("graphics"_hs)
            .data<&Config::players>("players"_hs)
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

    void reflect_dependencies()
    {
        //reflect_stl();
        reflect_math();
    }

    void reflect_primitives()
    {
        reflect<EntityType>();
        reflect<LightType>();
        reflect<Command>();

        //reflect<LightProperties>();
        //reflect<Axis>(); // RotationAxis
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
        reflect<PlayerConfig>();
        reflect<Config>();

        // ...

        reflection_generated = true;
    }
}