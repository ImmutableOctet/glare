#pragma once

#include "script_base.hpp"

#include <engine/entity/types.hpp>

#include <utility>
#include <optional>
#include <functional>
#include <type_traits>

#include <cassert>

namespace game
{
	class ScriptImpl;
}

namespace engine
{
	class EntitySystem;

	struct EntityThread;
	struct EntityThreadComponent;

	// Shared API for C++ script functionality.
	class Script : public ScriptBase
	{
		public:
			friend class EntitySystem;
			friend struct EntityThreadComponent;

			using Base = ScriptBase;
			using CoreScriptType = Script;

			using YieldContinuationPredicate = std::function<bool(const Script&, const MetaAny&)>;

			Script() = default;

			Script
			(
				const MetaEvaluationContext& context,
				Registry& registry,
				ScriptID script_id,
				Entity entity
			);

			Script(const Script&) = delete;
			Script(Script&&) noexcept = default;

			virtual ~Script();

			Script& operator=(const Script&) = delete;
			Script& operator=(Script&&) noexcept = default;

			virtual bool on_update();

			using ScriptBase::operator bool;

			using ScriptBase::operator();

			engine::ScriptFiber operator()() override;

			void set_pending_event_type(const MetaType& event_type);
			void set_pending_event_type(MetaTypeID event_type_id);

			void set_captured_event(MetaAny&& newly_captured_event);
			
			void clear_captured_event();

			bool waiting_for_event(const MetaType& event_type) const override;
			bool waiting_for_event(MetaTypeID event_type_id) const override;
			bool waiting_for_event() const override;

			MetaAny get_captured_event() override;
			MetaAny get_captured_event() const override;

			World& get_world() override;
			const World& get_world() const override;

			float get_delta() const;

			EntityThread* get_executing_thread() override;
			const EntityThread* get_executing_thread() const override;

			EntityThreadID get_executing_thread_name() const override;

			std::optional<EntityStateIndex> get_executing_state_index() const override;

			MetaTypeID get_pending_event_type_id() const;

			bool has_yield_continuation_predicate() const;

			const YieldContinuationPredicate& get_yield_continuation_predicate() const;

			template <typename PredicateType>
			void set_yield_continuation_predicate(PredicateType&& predicate)
			{
				if constexpr (std::is_same_v<std::remove_cvref_t<PredicateType>, std::nullptr_t>)
				{
					_yield_continuation_predicate = {};
				}
				else
				{
					_yield_continuation_predicate = std::forward<PredicateType>(predicate);
				}
			}

			// Attempts to retrieve a pointer to the most recently captured event as `EventType`.
			// NOTE: If the most recent event is not of type `EventType`, this will safely return a null value.
			template <typename EventType>
			const EventType* try_get_event() const
			{
				if (_captured_event)
				{
					return _captured_event.try_cast<EventType>();
				}

				return {};
			}

			// Retrieves the most recently captured event as `EventType`.
			// NOTE: If the most recent event is not of type `EventType`, this may throw or terminate.
			template <typename EventType>
			const EventType& get_event() const
			{
				auto event_ptr = try_get_event<EventType>();

				assert(event_ptr);

				return *event_ptr;
			}

			inline operator Entity() const
			{
				return ScriptBase::operator Entity();
			}

			inline explicit operator bool() const
			{
				return ScriptBase::operator bool();
			}

		protected:
			// A floating-point value used to scale incremental updates.
			// 
			// This value is updated automatically when execution is resumed, but is
			// indicative of the overall pace of the game, rather than the script itself.
			// 
			// This field exists primarily for ease-of-use when writing scripts.
			float delta = 1.0f;

		private:
			void set_executing_thread(EntityThread& thread);

			void clear_executing_thread();

			// Retrieves the latest delta-time state.
			void update_deltatime();

			// The thread responsible for execution and ownership of this script.
			EntityThread*              _executing_thread             = {};

			// Stores a copy of the most recent event captured.
			// While awaiting a captured event, this field may also be used to store the ID of the requested event type.
			MetaAny                    _captured_event               = {};

			// A predicate used to determine if a yield operation should complete.
			YieldContinuationPredicate _yield_continuation_predicate = {};
	};
}