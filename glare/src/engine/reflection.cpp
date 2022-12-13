// This file aggregates all `engine`-related reflection headers into one translation unit. 
// The purpose of which is to provide a single implementation-file for meta-type instantiation. (see `reflect_all`)
//
// TODO: Rework this source file into some form of automated 'reflection generation' procedure in the build process.

#include "reflection.hpp"
#include "meta.hpp"

#include "types.hpp"

#include "event_trigger_condition.hpp"
#include "entity_target.hpp"

// TODO: Determine if these make sense as CPP files (or PCH) instead:
#include "components/reflection.hpp"
#include "commands/reflection.hpp"

#include "meta/reflection.hpp"
#include "debug/reflection.hpp"
#include "state/reflection.hpp"
#include "input/reflection.hpp"
#include "world/reflection.hpp"

#include <math/reflection.hpp>

#include <util/format.hpp>

//#include <entt/meta/meta.hpp>
//#include <entt/entt.hpp>

namespace engine
{
    static void reflect_systems()
    {
        reflect<DebugListener>();
        reflect<StateSystem>();
        reflect<InputSystem>();
        reflect<World>();

        // ...
    }

    // TODO: Migrate these reflection routines to a different module.
    /*
    template <>
    void reflect<EventTriggerConditionType>()
    {
        engine_meta_type<EventTriggerConditionType>()
            .data<nullptr, &EventTriggerConditionType::compound_method>("compound_method"_hs)
        ;
    }
    */

    template <>
    void reflect<EventTriggerSingleCondition>()
    {
        engine_meta_type<EventTriggerSingleCondition>()
            //.base<EventTriggerConditionType>()
            .data<nullptr, &EventTriggerSingleCondition::compound_method>("compound_method"_hs)
            .data<nullptr, &EventTriggerSingleCondition::get_event_type_member>("event_type_member"_hs)
            .data<nullptr, &EventTriggerSingleCondition::get_comparison_value>("comparison_value"_hs)
            .data<nullptr, &EventTriggerSingleCondition::get_comparison_method>("comparison_method"_hs)
            .ctor<MetaSymbolID, MetaAny&&, EventTriggerComparisonMethod>()
        ;
    }

    template <>
    void reflect<EventTriggerAndCondition>()
    {
        engine_meta_type<EventTriggerAndCondition>()
            //.base<EventTriggerConditionType>()
            .data<nullptr, &EventTriggerAndCondition::compound_method>("compound_method"_hs)
        ;
    }

    template <>
    void reflect<EventTriggerOrCondition>()
    {
        engine_meta_type<EventTriggerOrCondition>()
            //.base<EventTriggerConditionType>()
            .data<nullptr, &EventTriggerOrCondition::compound_method>("compound_method"_hs)
        ;
    }

    template <>
    void reflect<EventTriggerTrueCondition>()
    {
        engine_meta_type<EventTriggerTrueCondition>()
            //.base<EventTriggerConditionType>()
            .data<nullptr, &EventTriggerTrueCondition::compound_method>("compound_method"_hs)
        ;
    }

    template <>
    void reflect<EventTriggerFalseCondition>()
    {
        engine_meta_type<EventTriggerFalseCondition>()
            //.base<EventTriggerConditionType>()
            .data<nullptr, &EventTriggerFalseCondition::compound_method>("compound_method"_hs)
        ;
    }

    template <>
    void reflect<EventTriggerCondition>()
    {
        //reflect<EventTriggerConditionType>();
        reflect<EventTriggerSingleCondition>();
        reflect<EventTriggerAndCondition>();
        reflect<EventTriggerOrCondition>();
        reflect<EventTriggerTrueCondition>();
        reflect<EventTriggerFalseCondition>();

        //engine_meta_type<EventTriggerCondition>();
    }

    template <>
    void reflect<EntityTarget>()
    {
        engine_meta_type<EntityTarget>()
            .data<nullptr, &EntityTarget::target_index>("target_index"_hs)

            .data<nullptr, &EntityTarget::is_self_targeted>("is_self_targeted"_hs)
            .data<nullptr, &EntityTarget::is_parent_target>("is_parent_target"_hs)
            .data<nullptr, &EntityTarget::is_exact_entity_target>("is_exact_entity_target"_hs)
            .data<nullptr, &EntityTarget::is_entity_name_target>("is_entity_name_target"_hs)
            .data<nullptr, &EntityTarget::is_child_target>("is_child_target"_hs)
            .data<nullptr, &EntityTarget::is_player_target>("is_player_target"_hs)
            .data<nullptr, &EntityTarget::is_null_target>("is_null_target"_hs)
            
            .func<&EntityTarget::get>("resolve"_hs)

            // NOTE: Alias to `resolve`.
            .func<&EntityTarget::get>("get"_hs)

            //.func<&EntityTarget::parse_type>("parse_type"_hs)
        ;
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
        //reflect_stl();
        reflect_math();
    }

    static void reflect_primitives()
    {
        reflect<EntityType>();
        reflect<LightType>();
        reflect<EntityTarget>();
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
        reflect_systems();

        // TODO: Move to a different module.
        reflect<EventTriggerCondition>();

        // ...

        reflection_generated = true;
    }
}