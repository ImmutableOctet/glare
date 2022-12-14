#pragma once

#include <engine/reflection.hpp>

#include "entity_target.hpp"
#include "event_trigger_condition.hpp"

namespace engine
{
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

	void reflect_entity_systems()
	{
		reflect<EntityTarget>();
        reflect<EventTriggerCondition>();
	}
}