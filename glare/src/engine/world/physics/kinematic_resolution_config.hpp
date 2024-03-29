#pragma once

#include "collision_cast_method.hpp"

#include <math/types.hpp>

#include <variant>

namespace engine
{
	struct CollisionComponent;

	// Collision resolution information for kinematic objects.
	// This is used to supply data to 
	struct KinematicResolutionConfig
	{
		// Uses an AABB for the shape/body. (Single 'length' value extracted)
		// (based on Bullet's `getAabb` and related functions)
		// NOTE: Bullet sometimes pads the size of AABBs.
		struct AABBType {};

		/*
			Uses an AABB for the shape/body. (All three dimensions)
			Similar to `AABBType`, but uses exact lengths for each dimension.
		*/
		struct AABBVectorType {};

		/*
			Uses an AABB for the shape/body. (All three dimensions)
			
			Similar to `AABBVectorType` and `AABBType`, but lengths are
			relative to the orientation of the object.

			NOTE: We do not further account for approach angle, just the transform.
		*/
		struct OrientedAABBVectorType {};

		/*
			Uses sphere-radius of object/shape.
			This is based on Bullet's `getBoundingSphere` command.
			
			NOTE:
			
			Because this is based on Bullet's logic, it uses AABBs under the hood,
			which effectively makes this: [sphere -> cube -> sphere] when using a spherical collision shape.
			
			The effect of this is a radius/size large enough to hold a cube, that itself is
			large enough to hold the original sphere shape, making the result unexpectedly large.

			As a workaround, I have also implemented `InnerSphereType`, which should give the
			expected result for spherical collision objects.
		*/
		struct SphereType {};

		// Uses inner sphere-radius of object/shape.
		// This is based on `CollisionComponent::get_inner_radius`.
		struct InnerSphereType {};

		// Implementation template for user-supplied size types.
		template <typename SizeType, bool apply_orientation=false>
		struct SizeTypeImpl
		{
			protected:
				SizeType half_size;

				// Alternate implementation:
				//SizeType size;
			public:
				using ValueType = SizeType;

				inline SizeTypeImpl(SizeType size):
					//size(size),
					half_size(size * 0.5f)
				{}

				//SizeType(const SizeType&) = default;
				//SizeType(SizeType&&) noexcept = default;

				inline void set_size(SizeType value)
				{
					half_size = (value * 0.5f);
				}

				inline void set_half_size(SizeType value)
				{
					half_size = value;
				}

				inline SizeType get_size() const { return (half_size * 2.0f); }
				inline SizeType get_half_size() const { return half_size; }

				// Alternate implementation:
				//SizeType get_size() const { return size; }
				//inline SizeType get_half_size() const { return (size / 2.0f); }
		};

		// Allows the user to specify an exact size.
		struct SizeType : SizeTypeImpl<float, false> { using SizeTypeImpl::SizeTypeImpl; };

		// Allows the user to specify an exact size on all three axes.
		struct VectorSizeType : SizeTypeImpl<math::Vector, false> { using SizeTypeImpl::SizeTypeImpl; };

		// Similar to `VectorSizeType`, but accounts for object orientation.
		// NOTE: We do not further account for approach angle, just the transform.
		struct OrientedVectorSizeType : SizeTypeImpl<math::Vector, true> { using SizeTypeImpl::SizeTypeImpl; };

		// Used to select between different size-type options at runtime.
		struct SizeConfig
		{
			using Type = std::variant
			<
				std::monostate,
				AABBType, AABBVectorType, OrientedAABBVectorType,
				SphereType, InnerSphereType,
				SizeType, VectorSizeType, OrientedVectorSizeType
			>;

			Type value;
		};

		CollisionCastMethod cast_method = CollisionCastMethod::None;
		SizeConfig size = { std::monostate {} };

		// If true, this object can influence the position of other kinematic objects.
		// The affected object must also be accepting influences. (see `accepts_influence`)
		// NOTE: Kinematic influences are only applicable to objects that can collide with one another.
		bool is_influencer          : 1 = false;

		// If true, this object can be kinematically influenced by another object.
		bool accepts_influence      : 1 = false;

		// If true, this object is subject to intersection resolution outside of casting.
		bool resolve_intersections  : 1 = true;

		// If true, an object can influence its children.
		// (Children in this case includes motion attachments)
		// 
		// This is useful for entities like platforms where you don't want to influence the objects on top of them,
		// but you do want to affect their position/rotation using a parent-child relationship.
		// NOTE: This also includes adjustments to children.
		bool can_influence_children : 1 = false;

		// Similar to `can_influence_children`, this controls whether a child entity can influence this entity.
		// NOTE: This also includes adjustments made because of potential overlap with children.
		bool can_be_influenced_by_children : 1 = false;

		inline bool get_is_influencer() const                 { return is_influencer;                 }
		inline bool get_accepts_influence() const             { return accepts_influence;             }
		inline bool get_resolve_intersections() const         { return resolve_intersections;         }
		inline bool get_can_influence_children() const        { return can_influence_children;        }
		inline bool get_can_be_influenced_by_children() const { return can_be_influenced_by_children; }

		inline void set_is_influencer(bool value)                 { is_influencer = value;                 }
		inline void set_accepts_influence(bool value)             { accepts_influence = value;             }
		inline void set_resolve_intersections(bool value)         { resolve_intersections = value;         }
		inline void set_can_influence_children(bool value)        { can_influence_children = value;        }
		inline void set_can_be_influenced_by_children(bool value) { can_be_influenced_by_children = value; }

		float get_size(const CollisionComponent& collision) const;
		float get_half_size(const CollisionComponent& collision) const;

		math::Vector get_size_vector(const CollisionComponent& collision) const;
		math::Vector get_half_size_vector(const CollisionComponent& collision) const;
	};
}