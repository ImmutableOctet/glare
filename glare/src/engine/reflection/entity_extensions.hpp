#pragma once

#include <engine/types.hpp>
#include <engine/events.hpp>
#include <engine/service.hpp>
#include <engine/transform.hpp>

#include <engine/components/relationship_component.hpp>

#include <engine/entity/entity_descriptor.hpp>
#include <engine/entity/components/instance_component.hpp>
#include <engine/entity/components/state_component.hpp>
#include <engine/entity/commands/state_change_command.hpp>

#include <engine/meta/hash.hpp>
#include <engine/meta/meta_evaluation_context.hpp>

#include <math/types.hpp>

namespace engine
{
	namespace impl
	{
        inline std::string entity_to_string_impl(Entity entity)
        {
            if (entity == null)
            {
                return "null";
            }
            else
            {
                return util::format("Entity #{}", entity);
            }
        }

        inline bool entity_to_bool_impl(Entity entity)
        {
            return (entity != null);
        }

        inline Entity entity_from_integer(std::underlying_type_t<Entity> value)
        {
            return static_cast<Entity>(value);
        }

        inline Entity entity_static_create(Registry& registry)
        {
            return registry.create();
        }

        inline Entity entity_static_destroy(Registry& registry, Entity entity)
        {
            registry.destroy(entity);

            return null; // entity;
        }

        inline Entity entity_destroy(Entity self, Registry& registry)
        {
            return entity_static_destroy(registry, self);
        }

		inline math::Vector entity_get_position(Entity self, Registry& registry)
        {
            return Transform(registry, self).get_position();
        }

        inline math::Vector entity_set_position(Entity self, Registry& registry, const math::Vector3D& position)
        {
            Transform(registry, self).set_position(position);
            
            return position;
        }

        inline math::Vector entity_get_local_position(Entity self, Registry& registry)
        {
            return Transform(registry, self).get_local_position();
        }

        inline math::Vector entity_set_local_position(Entity self, Registry& registry, const math::Vector3D& position)
        {
            Transform(registry, self).set_local_position(position);
            
            return position;
        }

        inline math::Vector entity_get_rotation(Entity self, Registry& registry)
        {
            return Transform(registry, self).get_rotation();
        }

        inline math::Vector entity_set_rotation(Entity self, Registry& registry, const math::Vector3D& rotation_angles)
        {
            Transform(registry, self).set_rotation(rotation_angles);

            return rotation_angles;
        }

        inline math::Vector entity_get_local_rotation(Entity self, Registry& registry)
        {
            return Transform(registry, self).get_local_rotation();
        }

        inline math::Vector entity_set_local_rotation(Entity self, Registry& registry, const math::Vector3D& rotation_angles)
        {
            Transform(registry, self).set_local_rotation(rotation_angles);

            return rotation_angles;
        }

        inline math::Vector entity_get_direction_vector(Entity self, Registry& registry)
        {
            return Transform(registry, self).get_direction_vector();
        }

        inline math::Vector entity_set_direction_vector(Entity self, Registry& registry, const math::Vector3D& direction_vector)
        {
            Transform(registry, self).set_direction_vector(direction_vector);

            return direction_vector;
        }

        inline math::Vector entity_get_scale(Entity self, Registry& registry)
        {
            return Transform(registry, self).get_scale();
        }

        inline math::Vector entity_set_scale(Entity self, Registry& registry, const math::Vector3D& scale)
        {
            Transform(registry, self).set_scale(scale);

            return scale;
        }

        inline math::Vector entity_get_local_scale(Entity self, Registry& registry)
        {
            return Transform(registry, self).get_local_scale();
        }

        inline math::Vector entity_set_local_scale(Entity self, Registry& registry, const math::Vector3D& scale)
        {
            Transform(registry, self).set_local_scale(scale);

            return scale;
        }

        inline Entity entity_get_parent(Entity self, Registry& registry)
        {
            if (auto relationship_comp = registry.try_get<RelationshipComponent>(self))
            {
                return relationship_comp->get_parent();
            }

            return null;
        }

        inline Entity entity_set_parent(Entity self, Registry& registry, Entity context_entity, const MetaEvaluationContext& context, Entity parent)
        {
            if (context.service)
            {
                context.service->set_parent(self, parent);
            }

            return self;
        }

        inline const StateComponent* entity_try_get_state_component(Entity self, Registry& registry)
        {
            return registry.try_get<StateComponent>(self);
        }

        inline const EntityState* entity_try_get_active_state(Entity self, Registry& registry)
        {
            if (auto instance_comp = registry.try_get<InstanceComponent>(self))
            {
                auto& descriptor = instance_comp->get_descriptor();

                if (auto state_comp = registry.try_get<StateComponent>(self))
                {
                    return descriptor.get_state_by_index(state_comp->state_index);
                }
            }

            return {};
        }

        inline const EntityState* entity_try_get_prev_state(Entity self, Registry& registry)
        {
            if (auto instance_comp = registry.try_get<InstanceComponent>(self))
            {
                auto& descriptor = instance_comp->get_descriptor();

                if (auto state_comp = registry.try_get<StateComponent>(self))
                {
                    return descriptor.get_state_by_index(state_comp->prev_state_index);
                }
            }

            return {};
        }

        inline MetaSymbolID entity_get_state_id(Entity self, Registry& registry)
        {
            if (auto active_state = entity_try_get_active_state(self, registry))
            {
                if (active_state->name)
                {
                    return *active_state->name;
                }
            }

            return {};
        }

        inline Entity entity_set_state_id(Entity self, Registry& registry, Entity context_entity, const MetaEvaluationContext& context, MetaSymbolID state_id)
        {
            if (context.service)
            {
                context.service->queue_event<StateChangeCommand> // event
                (
                    context_entity,
                    self,
                    state_id
                );
            }

            return self;
        }

        inline std::string entity_get_state_name(Entity self, Registry& registry)
        {
            if (auto state_id = entity_get_state_id(self, registry))
            {
                return std::string { get_known_string_from_hash(state_id) };
            }

            return {};
        }

        inline Entity entity_set_state_name(Entity self, Registry& registry, Entity context_entity, const MetaEvaluationContext& context, const std::string& state_name)
        {
            const auto state_id = hash(state_name).value();

            return entity_set_state_id(self, registry, context_entity, context, state_id);
        }

        inline EntityStateIndex entity_get_state_index(Entity self, Registry& registry)
        {
            if (auto state_comp = entity_try_get_state_component(self, registry))
            {
                return state_comp->state_index;
            }

            return {};
        }

        inline Entity entity_set_state_index(Entity self, Registry& registry, Entity context_entity, const MetaEvaluationContext& context, EntityStateIndex state_index)
        {
            if (auto instance_comp = registry.try_get<InstanceComponent>(self))
            {
                auto& descriptor = instance_comp->get_descriptor();

                if (const auto state_id = descriptor.get_state_name(state_index))
                {
                    entity_set_state_id(self, registry, context_entity, context, *state_id);
                }
            }

            return self;
        }

        inline EntityStateIndex entity_get_prev_state_index(Entity self, Registry& registry)
        {
            if (auto state_comp = entity_try_get_state_component(self, registry))
            {
                return state_comp->prev_state_index;
            }

            //assert(false);

            return {};
        }

        inline MetaSymbolID entity_get_prev_state_id(Entity self, Registry& registry)
        {
            if (auto instance_comp = registry.try_get<InstanceComponent>(self))
            {
                auto& descriptor = instance_comp->get_descriptor();

                const auto prev_state_index = entity_get_prev_state_index(self, registry);

                if (auto state_name = descriptor.get_state_name(prev_state_index))
                {
                    return (*state_name);
                }
            }

            //assert(false);

            return {};
        }

        inline std::string entity_get_prev_state_name(Entity self, Registry& registry)
        {
            if (auto prev_state_id = entity_get_prev_state_id(self, registry))
            {
                return std::string { get_known_string_from_hash(prev_state_id) };
            }

            return {};
        }
	}
}