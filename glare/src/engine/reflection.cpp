// This file aggregates all `engine`-related reflection headers into one translation unit. 
// The purpose of which is to provide a single implementation-file for meta-type instantiation. (see `reflect_all`)
//
// TODO: Rework this source file into some form of automated 'reflection generation' procedure in the build process.

#include "reflection.hpp"
#include "meta.hpp"

#include "types.hpp"

// TODO: Determine if these make sense as CPP files (or PCH) instead:
#include "components/reflection.hpp"

#include "meta/reflection.hpp"
#include "input/reflection.hpp"
#include "world/reflection.hpp"
#include "state/reflection.hpp"

#include <math/reflection.hpp>

#include <util/format.hpp>

//#include <entt/meta/meta.hpp>
//#include <entt/entt.hpp>

namespace engine
{
    static void reflect_systems()
    {
        reflect<StateSystem>();
        reflect<InputSystem>();
        reflect<World>();

        // ...
    }

    // Reflects `math::Vector2D` with the generalized name of `Vector2D`.
    // 
    // TODO: Look into migrating this to another file/header.
    template <>
    void reflect<math::Vector2D>()
    {
        math::reflect<math::Vector2D>("Vector2D"_hs);
    }

    // Reflects `math::Vector3D` with the generalized name of `Vector3D`.
    // 
    // TODO: Look into migrating this to another file/header.
    template <>
    void reflect<math::Vector3D>()
    {
        math::reflect<math::Vector3D>("Vector3D"_hs);
    }

    // TODO: Implement reflection for matrix types.
    static void reflect_math()
    {
        reflect<math::Vector2D>();
        reflect<math::Vector3D>();

        // ...
    }

    static void reflect_dependencies()
    {
        reflect_math();
    }

    static void reflect_primitives()
    {
        reflect<EntityType>();
        reflect<LightType>();

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

        if (primitives)
        {
            reflect_primitives();
        }

        if (dependencies)
        {
            reflect_dependencies();
        }

        reflect_core_components();
        reflect_systems();

        // ...

        reflection_generated = true;
    }
}