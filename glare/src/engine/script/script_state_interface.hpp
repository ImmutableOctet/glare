#pragma once

#include <engine/entity/types.hpp>

#include <cassert>

namespace engine
{
	class Script;
	class EntityState;

	struct ScriptStateInterface
	{
		using EntityStatePtr = const EntityState*;
		using EntityStateRef = const EntityState&;

		Script& script;

		// Disabled for now to avoid vtable generation:
		/*
			virtual EntityStateID get() const = 0;
			virtual EntityStatePtr get_description() const;

			virtual bool set(EntityStateID state_id) = 0;
			virtual bool set(EntityStateRef state) = 0;
			
			inline operator EntityStateID()  const { return get(); }
			inline operator EntityStatePtr() const { return get_description(); }

			inline explicit operator bool() const
			{
				return static_cast<bool>(get());
			}

			inline bool operator==(EntityStatePtr state) const
			{
				return (get_description() == state);
			}

			inline bool operator!=(EntityStatePtr state) const
			{
				return (!operator==(state));
			}

			inline bool operator==(EntityStateRef state) const
			{
				return operator==(&state);
			}

			inline bool operator!=(EntityStateRef state) const
			{
				return (!operator==(state));
			}

			inline bool operator==(EntityStateID state_id) const
			{
				return (get() == state_id);
			}

			inline bool operator!=(EntityStateID state_id) const
			{
				return (!operator==(state_id));
			}

			inline ScriptStateInterface& operator=(EntityStateRef state)
			{
				set(state);

				return *this;
			}

			inline ScriptStateInterface& operator=(EntityStatePtr state)
			{
				if (state)
				{
					return operator=(*state);
				}

				return *this;
			}
		*/
	};

	struct ScriptCurrentStateInterface final : public ScriptStateInterface
	{
		EntityStateID get() const;
		EntityStatePtr get_description() const;

		bool set(EntityStateID state_id);
		bool set(EntityStateRef state);

		// Boilerplate to avoid vtable generation:
		inline operator EntityStateID() const
		{
			return get();
		}

		inline operator EntityStatePtr() const
		{
			return get_description();
		}

		inline explicit operator bool() const
		{
			return static_cast<bool>(get());
		}

		inline bool operator==(EntityStatePtr state) const
		{
			return (get_description() == state);
		}

		inline bool operator!=(EntityStatePtr state) const
		{
			return (!operator==(state));
		}

		inline bool operator==(EntityStateRef state) const
		{
			return operator==(&state);
		}

		inline bool operator!=(EntityStateRef state) const
		{
			return (!operator==(state));
		}

		inline bool operator==(EntityStateID state_id) const
		{
			return (get() == state_id);
		}

		inline bool operator!=(EntityStateID state_id) const
		{
			return (!operator==(state_id));
		}

		inline ScriptStateInterface& operator=(EntityStateID state_id)
		{
			set(state_id);

			return *this;
		}

		inline ScriptStateInterface& operator=(EntityStatePtr state)
		{
			if (state)
			{
				set(*state);
			}

			return *this;
		}

		inline ScriptStateInterface& operator=(EntityStateRef state)
		{
			set(state);

			return *this;
		}
	};

	struct ScriptPreviousStateInterface final : public ScriptStateInterface
	{
		EntityStateID get() const;
		EntityStatePtr get_description() const;

		bool set(EntityStateID state_id);
		bool set(EntityStateRef state);

		// Boilerplate to avoid vtable generation:
		inline operator EntityStateID() const
		{
			return get();
		}

		inline operator EntityStatePtr() const
		{
			return get_description();
		}

		inline explicit operator bool() const
		{
			return static_cast<bool>(get());
		}

		inline bool operator==(EntityStatePtr state) const
		{
			return (get_description() == state);
		}

		inline bool operator!=(EntityStatePtr state) const
		{
			return (!operator==(state));
		}

		inline bool operator==(EntityStateRef state) const
		{
			return operator==(&state);
		}

		inline bool operator!=(EntityStateRef state) const
		{
			return (!operator==(state));
		}

		inline bool operator==(EntityStateID state_id) const
		{
			return (get() == state_id);
		}

		inline bool operator!=(EntityStateID state_id) const
		{
			return (!operator==(state_id));
		}

		inline ScriptStateInterface& operator=(EntityStatePtr state)
		{
			assert(false);

			return *this;
		}

		inline ScriptStateInterface& operator=(EntityStateRef state)
		{
			assert(false);

			return *this;
		}
	};
}