#pragma once

#include <engine/types.hpp>
#include <engine/transform.hpp>

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

        inline Entity entity_from_integer(std::underlying_type_t<Entity> value)
        {
            return static_cast<Entity>(value);
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
	}
}