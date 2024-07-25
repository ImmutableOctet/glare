#pragma once

// NOTE:
// This is a standalone header for the `TransformComponent` type and related functionality.
// For most use-cases, including the `transform` header is preferred.

#include "types.hpp"

#include <util/enum_operators.hpp>
#include <math/math.hpp>

namespace engine
{
	class World;

	struct Transform;

	enum class _TransformComponent_Dirty : std::uint8_t
	{
		None = 0,

		M                    = 1, // Model
		W                    = 2, // World
		IW                   = 4, // Inverse World
		
		/*
			Indicates that a change has taken place;
			used for collision, dynamic cubemap matrices (shadows, reflections), etc.
			
			Order of operations is essentially a mark-and-sweep approach:
			Modify Transform (flag triggered; mark) -> Detect flags -> Trigger events -> Disable flags (sweep) -> Repeat
		*/
		EventFlag = 8,

		All = (M | W | IW),
	};

	FLAG_ENUM(std::uint8_t, _TransformComponent_Dirty);

	// Data-only component; logic is stored in 'Transformation', as several operations require additional registry access.
	// 
	// TODO: Look into implementing a better public constructor.
	struct TransformComponent
	{
		public:
			using Dirty = _TransformComponent_Dirty;
			using Flag  = Dirty;

			friend Transform;
			friend World;

			// Local transform:

			// Local position/translation.
			math::Vector translation = {};

			// Local scale/magnitude.
			math::Vector scale = { 1.0f, 1.0f, 1.0f }; // _s

			// Local orientation/basis.
			math::RotationMatrix basis = math::identity<math::RotationMatrix>(); // ('Basis') // _r

		protected:
			// Matrix caches:
			mutable math::Matrix _m  = math::affine_mat4(1.0f);        // Model matrix (Representation of 'local transform')
			mutable math::Matrix _w  = math::identity<math::Matrix>(); // World (Representation of scene-graph matrix multiplication chain)
			mutable math::Matrix _iw = glm::inverse(_w);               // Inverse world (Inverse of `_w`/world-matrix)

			// Internal cache-management bitfield.
			mutable Dirty _dirty = Dirty::All; // Dirty::None;
		public:
			bool invalid(Dirty flag) const;

			void invalidate(Dirty flag) const;
			Dirty validate(Dirty flag) const;

			// Helper function for event-flags, etc.
			template <typename Event>
			inline bool on_flag(Flag flag, Event&& event_fn) const
			{
				if (invalid(flag))
				{
					validate(flag);

					event_fn();

					return true;
				}

				return false;
			}
	};
}