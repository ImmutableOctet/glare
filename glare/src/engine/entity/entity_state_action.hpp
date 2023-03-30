#pragma once

#include "types.hpp"

#include "actions/actions.hpp"

#include <engine/timer.hpp>

#include <engine/meta/types.hpp>
#include <engine/meta/meta_evaluation_context.hpp>

#include <variant>
#include <optional>

namespace engine
{
	class Service;
	class EntityDescriptor;

	using EntityStateAction = std::variant
	<
		EntityStateTransitionAction,
		EntityStateCommandAction,
		EntityStateUpdateAction,
		EntityThreadSpawnAction,
		EntityThreadStopAction,
		EntityThreadPauseAction,
		EntityThreadResumeAction,
		EntityThreadAttachAction,
		EntityThreadDetachAction,
		EntityThreadUnlinkAction,
		EntityThreadSkipAction,
		EntityThreadRewindAction
	>;

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityStateTransitionAction& transition,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay=std::nullopt,
		const MetaEvaluationContext& context={},
		const MetaAny& event_instance={}
	);

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityStateCommandAction& command,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay=std::nullopt,
		const MetaEvaluationContext& context={},
		const MetaAny& event_instance={}
	);

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityStateUpdateAction& update,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay=std::nullopt,
		const MetaEvaluationContext& context={},
		const MetaAny& event_instance={},

		bool defer_evaluation=false, // true
		bool use_member_assignment_on_patch=false // true
	);

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityThreadSpawnAction& thread_spawn,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay=std::nullopt,
		const MetaEvaluationContext& context={},
		const MetaAny& event_instance={}
	);

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityThreadStopAction& thread_stop,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay=std::nullopt,
		const MetaEvaluationContext& context={},
		const MetaAny& event_instance={}
	);

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityThreadPauseAction& thread_pause,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay=std::nullopt,
		const MetaEvaluationContext& context={},
		const MetaAny& event_instance={}
	);

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityThreadResumeAction& thread_resume,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay=std::nullopt,
		const MetaEvaluationContext& context={},
		const MetaAny& event_instance={}
	);

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityThreadAttachAction& thread_attach,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay=std::nullopt,
		const MetaEvaluationContext& context={},
		const MetaAny& event_instance={}
	);

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityThreadDetachAction& thread_detach,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay=std::nullopt,
		const MetaEvaluationContext& context={},
		const MetaAny& event_instance={}
	);

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityThreadUnlinkAction& thread_unlink,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay=std::nullopt,
		const MetaEvaluationContext& context={},
		const MetaAny& event_instance={}
	);

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityThreadSkipAction& thread_skip,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay=std::nullopt,
		const MetaEvaluationContext& context={},
		const MetaAny& event_instance={}
	);

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityThreadRewindAction& thread_rewind,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay=std::nullopt,
		const MetaEvaluationContext& context={},
		const MetaAny& event_instance={}
	);

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityStateAction& action,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay=std::nullopt,
		const MetaEvaluationContext& context={},
		const MetaAny& event_instance={}
	);

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityStateAction& action,
		Entity source_entity, const EntityTarget& target,
		std::optional<engine::Timer::Duration> delay=std::nullopt,
		const MetaEvaluationContext& context={},
		const MetaAny& event_instance={}
	);

	void execute_action
	(
		Service& service,
		const EntityStateAction& action,
		Entity source_entity, const EntityTarget& target,
		std::optional<engine::Timer::Duration> delay = std::nullopt,
		const MetaEvaluationContext& context={},
		const MetaAny& event_instance={}
	);
}