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

        inline math::RotationMatrix entity_get_basis(Entity self, Registry& registry)
        {
            return Transform(registry, self).get_basis();
        }

        inline math::RotationMatrix entity_set_basis(Entity self, Registry& registry, const math::RotationMatrix& basis)
        {
            Transform(registry, self).set_basis(basis);

            return basis;
        }

        inline math::RotationMatrix entity_set_basis(Entity self, Registry& registry, const math::RotationMatrix& basis, float turn_speed)
        {
            auto tform = Transform(registry, self);
            
            tform.set_basis(basis, turn_speed);

            return tform.get_basis();
        }

        inline math::RotationMatrix entity_get_local_basis(Entity self, Registry& registry)
        {
            return Transform(registry, self).get_local_basis();
        }

        inline math::RotationMatrix entity_set_local_basis(Entity self, Registry& registry, const math::RotationMatrix& basis)
        {
            Transform(registry, self).set_local_basis(basis);

            return basis;
        }

        inline math::RotationMatrix entity_set_local_basis(Entity self, Registry& registry, const math::RotationMatrix& basis, float turn_speed)
        {
            auto tform = Transform(registry, self);

            tform.set_local_basis(basis, turn_speed);

            return tform.get_local_basis();
        }

        inline math::Quaternion entity_get_basis_q(Entity self, Registry& registry)
        {
            return Transform(registry, self).get_basis_q();
        }

        inline math::Quaternion entity_set_basis_q(Entity self, Registry& registry, const math::Quaternion& basis)
        {
            Transform(registry, self).set_basis_q(basis);

            return basis;
        }

        inline math::Quaternion entity_set_basis_q(Entity self, Registry& registry, const math::Quaternion& basis, float turn_speed)
        {
            auto tform = Transform(registry, self);
            
            tform.set_basis_q(basis, turn_speed);

            return tform.get_basis_q();
        }

        inline math::Quaternion entity_get_local_basis_q(Entity self, Registry& registry)
        {
            return Transform(registry, self).get_local_basis_q();
        }

        inline math::Quaternion entity_set_local_basis_q(Entity self, Registry& registry, const math::Quaternion& basis)
        {
            Transform(registry, self).set_local_basis_q(basis);

            return basis;
        }

        inline math::Quaternion entity_set_local_basis_q(Entity self, Registry& registry, const math::Quaternion& basis, float turn_speed)
        {
            auto tform = Transform(registry, self);
            
            tform.set_local_basis_q(basis, turn_speed);

            return tform.get_local_basis_q();
        }

        inline math::RotationMatrix entity_apply_basis(Entity self, Registry& registry, const math::RotationMatrix& basis, bool local=false)
        {
            auto tform = Transform(registry, self);

            tform.apply_basis(basis, local);

            return tform.get_basis();
        }

        inline math::RotationMatrix entity_apply_basis(Entity self, Registry& registry, const math::RotationMatrix& basis, float turn_speed, bool local=false)
        {
            auto tform = Transform(registry, self);

            tform.apply_basis(basis, turn_speed, local);

            return tform.get_basis();
        }

        inline math::Quaternion entity_apply_basis_q(Entity self, Registry& registry, const math::Quaternion& basis, bool local=false)
        {
            auto tform = Transform(registry, self);

            tform.apply_basis_q(basis, local);

            return tform.get_basis_q();
        }

        inline math::Quaternion entity_apply_basis_q(Entity self, Registry& registry, const math::Quaternion& basis, float turn_speed, bool local=false)
        {
            auto tform = Transform(registry, self);

            tform.apply_basis_q(basis, turn_speed, local);

            return tform.get_basis_q();
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

        inline math::Vector entity_set_direction_vector(Entity self, Registry& registry, const math::Vector& direction_vector)
        {
            Transform(registry, self).set_direction_vector(direction_vector);

            return direction_vector;
        }

        inline math::Vector entity_set_direction_vector(Entity self, Registry& registry, const math::Vector& direction_vector, float turn_speed)
        {
            auto tform = Transform(registry, self);

            tform.set_direction_vector(direction_vector, turn_speed);

            return tform.get_direction_vector();
        }

        inline math::Vector entity_set_direction_vector(Entity self, Registry& registry, const math::Vector& direction_vector, float turn_speed, bool apply_x, bool apply_y=true, bool apply_z=true)
        {
            auto tform = Transform(registry, self);

            tform.set_direction_vector
            (
                direction_vector, turn_speed,
                apply_x, apply_y, apply_z
            );

            return tform.get_direction_vector();
        }

        inline math::Vector entity_get_flat_direction_vector(Entity self, Registry& registry)
        {
            return Transform(registry, self).get_flat_direction_vector();
        }

        inline math::Vector entity_set_flat_direction_vector(Entity self, Registry& registry, const math::Vector& direction_vector)
        {
            auto tform = Transform(registry, self);

            tform.set_flat_direction_vector(direction_vector);

            return tform.get_flat_direction_vector(); // get_direction_vector();
        }

        inline math::Vector entity_set_flat_direction_vector(Entity self, Registry& registry, const math::Vector& direction_vector, float turn_speed)
        {
            auto tform = Transform(registry, self);

            tform.set_flat_direction_vector(direction_vector, turn_speed);

            return tform.get_flat_direction_vector(); // get_direction_vector();
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

        inline float entity_get_position_x(Entity self, Registry& registry)
        {
            return Transform(registry, self).get_position().x;
        }

        inline float entity_set_position_x(Entity self, Registry& registry, float value)
        {
            auto tform = Transform(registry, self);

            auto position = tform.get_position();

            position.x = value;

            tform.set_position(position);

            return value;
        }

        inline float entity_get_position_y(Entity self, Registry& registry)
        {
            return Transform(registry, self).get_position().y;
        }

        inline float entity_set_position_y(Entity self, Registry& registry, float value)
        {
            auto tform = Transform(registry, self);

            auto position = tform.get_position();

            position.y = value;

            tform.set_position(position);

            return value;
        }

        inline float entity_get_position_z(Entity self, Registry& registry)
        {
            return Transform(registry, self).get_position().z;
        }

        inline float entity_set_position_z(Entity self, Registry& registry, float value)
        {
            auto tform = Transform(registry, self);

            auto position = tform.get_position();

            position.z = value;

            tform.set_position(position);

            return value;
        }

        inline float entity_get_rotation_pitch(Entity self, Registry& registry)
        {
            return Transform(registry, self).get_pitch();
        }

        inline float entity_set_rotation_pitch(Entity self, Registry& registry, float value)
        {
            Transform(registry, self).set_pitch(value);

            return value;
        }

        inline float entity_set_rotation_pitch(Entity self, Registry& registry, float value, float turn_speed)
        {
            Transform(registry, self).set_pitch(value, turn_speed);

            return value;
        }

        inline float entity_get_rotation_yaw(Entity self, Registry& registry)
        {
            return Transform(registry, self).get_yaw();
        }

        inline float entity_set_rotation_yaw(Entity self, Registry& registry, float value)
        {
            Transform(registry, self).set_yaw(value);

            return value;
        }

        inline float entity_set_rotation_yaw(Entity self, Registry& registry, float value, float turn_speed)
        {
            Transform(registry, self).set_yaw(value, turn_speed);

            return value;
        }

        inline float entity_get_rotation_roll(Entity self, Registry& registry)
        {
            return Transform(registry, self).get_roll();
        }

        inline float entity_set_rotation_roll(Entity self, Registry& registry, float value)
        {
            Transform(registry, self).set_roll(value);

            return value;
        }

        inline float entity_set_rotation_roll(Entity self, Registry& registry, float value, float turn_speed)
        {
            Transform(registry, self).set_roll(value, turn_speed);

            return value;
        }

        inline float entity_get_local_rotation_pitch(Entity self, Registry& registry)
        {
            return Transform(registry, self).get_local_pitch();
        }

        inline float entity_set_local_rotation_pitch(Entity self, Registry& registry, float value)
        {
            Transform(registry, self).set_local_pitch(value);

            return value;
        }

        inline float entity_set_local_rotation_pitch(Entity self, Registry& registry, float value, float turn_speed)
        {
            Transform(registry, self).set_local_pitch(value, turn_speed);

            return value;
        }

        inline float entity_get_local_rotation_yaw(Entity self, Registry& registry)
        {
            return Transform(registry, self).get_local_yaw();
        }

        inline float entity_set_local_rotation_yaw(Entity self, Registry& registry, float value)
        {
            Transform(registry, self).set_local_yaw(value);

            return value;
        }

        inline float entity_set_local_rotation_yaw(Entity self, Registry& registry, float value, float turn_speed)
        {
            Transform(registry, self).set_local_yaw(value, turn_speed);

            return value;
        }

        inline float entity_get_local_rotation_roll(Entity self, Registry& registry)
        {
            return Transform(registry, self).get_local_roll();
        }

        inline float entity_set_local_rotation_roll(Entity self, Registry& registry, float value)
        {
            Transform(registry, self).set_local_roll(value);

            return value;
        }

        inline float entity_set_local_rotation_roll(Entity self, Registry& registry, float value, float turn_speed)
        {
            Transform(registry, self).set_local_roll(value, turn_speed);

            return value;
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

        inline math::Vector entity_rotate(Entity self, Registry& registry, const math::Vector& rotation, bool local=false)
        {
            auto tform = Transform(registry, self);

            tform.rotate(rotation, local);

            if (local)
            {
                return tform.get_local_rotation();
            }

            return tform.get_rotation();
        }

        inline math::Vector entity_rotate(Entity self, Registry& registry, const math::Vector& rotation, float turn_speed, bool local=false)
        {
            auto tform = Transform(registry, self);

            tform.rotate(rotation, turn_speed, local);

            if (local)
            {
                return tform.get_local_rotation();
            }

            return tform.get_rotation();
        }

        inline float entity_rotate_x(Entity self, Registry& registry, float value, bool local=false)
        {
            auto tform = Transform(registry, self);

            tform.rotateX(value, local);

            if (local)
            {
                return tform.get_local_rotation().x;
            }

            return tform.rx();
        }

        inline float entity_rotate_y(Entity self, Registry& registry, float value, bool local=false)
        {
            auto tform = Transform(registry, self);

            tform.rotateY(value, local);

            if (local)
            {
                return tform.get_local_rotation().y;
            }

            return tform.ry();
        }

        inline float entity_rotate_z(Entity self, Registry& registry, float value, bool local=false)
        {
            auto tform = Transform(registry, self);

            tform.rotateZ(value, local);

            if (local)
            {
                return tform.get_local_rotation().z;
            }

            return tform.rz();
        }

        inline math::Vector entity_move(Entity self, Registry& registry, const math::Vector& translation_vector, bool local=false)
        {
            auto tform = Transform(registry, self);

            tform.move(translation_vector, local);

            if (local)
            {
                return tform.get_local_position();
            }

            return tform.get_position();
        }
	}
}