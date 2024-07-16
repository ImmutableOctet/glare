#pragma once

#include "script_fiber.hpp"
#include "script_function_bootstrap.hpp"
#include "script_entry_point.hpp"
#include "script_traits.hpp"

#include "child_script.hpp"

#include <engine/types.hpp>

#include <engine/meta/types.hpp>
#include <engine/meta/hash.hpp>

#include <engine/entity/types.hpp>
#include <engine/entity/events.hpp>
#include <engine/entity/entity_thread_flags.hpp>
#include <engine/entity/components/entity_thread_component.hpp>
#include <engine/entity/commands/entity_thread_spawn_command.hpp>
#include <engine/entity/commands/entity_thread_fiber_spawn_command.hpp>

#include <util/type_traits.hpp>
#include <util/type_algorithms.hpp>
#include <util/function_traits.hpp>
#include <util/format.hpp>

#include <utility>
#include <type_traits>
#include <tuple>

#include <cassert>

namespace engine
{
	class ScriptBootstrapImpl
	{
		public:
			decltype(auto) start_script_impl(this auto&& self, auto&& script_entry_point, Entity source, Entity target, EntityThreadID thread_name, const EntityThreadFlags& thread_flags={}, auto&&... script_args)
			{
				using SelfType = std::remove_cvref_t<decltype(self)>;

				using CoreScriptType = typename SelfType::CoreScriptType;

				using ScriptEntryPoint = std::remove_cvref_t<decltype(script_entry_point)>;

				if (!thread_name)
				{
					thread_name = self.template get_thread_name<ScriptEntryPoint>();
				}

				auto script_handle = ScriptHandle {};

				auto script_fiber = impl::script_bootstrap_fn_impl
				(
					script_handle,

					[&]
					(
						Registry& registry,
						Entity entity,
						const MetaEvaluationContext context
					)
					{
						if constexpr (engine::script::traits::type_member_CoreScriptType_is_same_v<SelfType, ScriptEntryPoint>)
						{
							return std::move(script_entry_point);
						}
						else
						{
							return ChildScript<CoreScriptType, std::decay_t<decltype(script_entry_point)>>
							{
								registry,
								entity,
								context,

								std::move(script_entry_point),

								thread_name
							};
						}
					},

					self.get_registry(), target, self.get_context(),
					std::forward<decltype(script_args)>(script_args)...
				);

				// Script fibers are lazy, including the bootstrap function itself.
				// 
				// We call into `script_fiber` once here to initialize the `Script` object
				// described by our lambda, updating `script_handle` in the process.
				// 
				// NOTE: The lifetime and storage of the `Script` object is managed by the fiber.
				// Practically speaking, this is part of an implementation-defined heap allocated coroutine frame.
				// 
				// Allocation of the actual `script_entry_point` callable and its coroutine may be more easily elided.
				// (i.e. its state could be stored as part of the same frame that owns the `Script` object)
				script_fiber();

				return self.start_thread_impl
				(
					std::move(script_fiber),

					source, target,

					thread_name, thread_flags,

					script_handle
				);
			}

			decltype(auto) start_script_impl(this auto&& self, auto&& script_entry_point, Entity source, Entity target)
			{
				using EntryPointType = std::decay_t<decltype(script_entry_point)>;

				if constexpr ((std::is_same_v<EntryPointType, EntityThreadID>) || (std::is_same_v<EntryPointType, entt::hashed_string>))
				{
					return self.start_thread_impl(std::forward<decltype(script_entry_point)>(script_entry_point), source, target);
				}
				else
				{
					return self.start_script_impl(std::forward<decltype(script_entry_point)>(script_entry_point), source, target, EntityThreadID {});
				}
			}

			// TODO: Look at forwarding `additional_args` to entry-point for non-script fibers.
			decltype(auto) start_thread_impl(this auto&& self, auto&& thread_entry_point, Entity source, Entity target, EntityThreadID thread_name, const EntityThreadFlags& thread_flags={}, ScriptHandle script_handle={}, auto&&... additional_args)
			{
				using EntryPoint = decltype(thread_entry_point);

				auto fiber = ScriptFiber {};

				constexpr bool is_fiber = (std::is_same_v<std::remove_cvref_t<EntryPoint>, ScriptFiber>);

				if constexpr (is_fiber)
				{
					fiber = std::forward<EntryPoint>(thread_entry_point);
				}
				else
				{
					if (!thread_name)
					{
						if constexpr (util::is_standalone_function_v<EntryPoint>)
						{
							thread_name = self.get_thread_name(thread_entry_point);
						}
						else
						{
							thread_name = self.template get_thread_name<EntryPoint>();
						}
					}

					if constexpr (std::is_invocable_r_v<ScriptFiber, EntryPoint, GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST_TYPES>)
					{
						fiber = self.begin_fiber(std::forward<EntryPoint>(thread_entry_point));
					}
					else if constexpr (std::is_invocable_r_v<ScriptFiber, EntryPoint>)
					{
						fiber = thread_entry_point();
					}
					else
					{
						fiber = self.begin_fiber(thread_entry_point);

						if (!fiber)
						{
							if constexpr (std::is_invocable_v<EntryPoint, decltype(self)>)
							{
								fiber = [&self, &thread_entry_point]() -> ScriptFiber
								{
									thread_entry_point(self);

									co_return;
								}();
							}
							else if constexpr (std::is_invocable_v<EntryPoint>)
							{
								fiber = [&thread_entry_point]() -> ScriptFiber
								{
									thread_entry_point();

									co_return;
								}();
							}
						}
					}
				}

				const auto started_fiber_coroutine_address = fiber.get_coroutine_handle().address();

				self.template event<EntityThreadFiberSpawnCommand>
				(
					source,
					target,

					std::move(fiber),

					self.get_executing_state_index(),
					self.get_executing_thread_name(),

					thread_name,
					thread_flags,

					script_handle
				);

				using ConstScriptRef = const decltype(self)&;

				return self.template until<OnThreadComplete>
				(
					[started_fiber_coroutine_address, thread_name](ConstScriptRef self, const OnThreadComplete& completion_event)
					{
						if (thread_name)
						{
							return (completion_event.thread_id == thread_name);
						}

						if (const auto thread_component = self.try_get<EntityThreadComponent>())
						{
							const auto& thread = thread_component->threads[completion_event.local_instance];

							if (thread.has_fiber())
							{
								const auto& completed_fiber_wrapper = thread.get_fiber();
								const auto completed_fiber_coroutine_address = completed_fiber_wrapper.get_address();

								if (completed_fiber_coroutine_address == started_fiber_coroutine_address)
								{
									return true;
								}
							}
						}

						return false;
					}
				);
			}

			decltype(auto) start_thread_impl(this auto&& self, EntityThreadID thread_name, Entity source, Entity target, bool restart_existing, auto&&... additional_args)
			{
				using ConstScriptRef = const decltype(self)&;

				assert(thread_name);

				self.template event<EntityThreadSpawnCommand>
				(
					source,
					target,

					EntityThreadTarget { thread_name },

					self.get_executing_thread_name(),
					
					restart_existing,

					self.get_executing_state_index()
				);

				return self.template until<OnThreadComplete>
				(
					[thread_name](ConstScriptRef self, const OnThreadComplete& completion_event)
					{
						return (completion_event.thread_id == thread_name);
					}
				);
			}

			decltype(auto) start_thread_impl(this auto&& self, auto&& thread_entry_point, Entity source, Entity target)
			{
				using EntryPointType = std::decay_t<decltype(thread_entry_point)>;

				if constexpr ((std::is_same_v<EntryPointType, EntityThreadID>) || (std::is_same_v<EntryPointType, entt::hashed_string>))
				{
					return self.start_thread_impl(static_cast<EntityThreadID>(thread_entry_point), source, target, true);
				}
				else if constexpr (std::is_invocable_v<decltype(thread_entry_point), GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST_TYPES>)
				{
					return self.start_script_impl(std::forward<decltype(thread_entry_point)>(thread_entry_point), source, target);
				}
				else
				{
					return self.start_thread_impl(std::forward<decltype(thread_entry_point)>(thread_entry_point), source, target, EntityThreadID {});
				}
			}

			template <typename EntryPoint>
			ScriptFiber begin_fiber(this auto&& self, EntryPoint&& entry_point)
			{
				return self.begin_fiber_impl
				(
					[&entry_point](auto&&... args) -> ScriptFiber
					{
						auto fiber = ScriptFiber {};

						util::drop_last_until_success<0>
						(
							[&entry_point, &fiber](auto&&... args)
							{
								if constexpr (std::is_invocable_r_v<ScriptFiber, decltype(entry_point), decltype(args)...>)
								{
									fiber = entry_point(std::forward<decltype(args)>(args)...);

									return static_cast<bool>(fiber);
								}
								else
								{
									return false;
								}
							},

							std::forward<decltype(args)>(args)...
						);

						return fiber;
					}
				);
			}

		protected:
			// Initializes a new script fiber from the entry-point specified.
			template <typename EntryPoint>
			ScriptFiber begin_fiber_impl(this auto&& self, EntryPoint&& entry_point)
			{
				return entry_point
				(
					self,
					self,

					self.get_registry(),
					self.get_entity(),
					self.get_context(),
			
					self.get_service(),
					self.get_system_manager(),
			
					self.get_world(),
					self.get_delta_system(),

					engine::ScriptCurrentStateInterface  { self },
					engine::ScriptPreviousStateInterface { self }
				);
			}


			ScriptFiber basic_call_operator_impl(this auto&& self)
			{
				return self.begin_fiber_impl
				(
					[&self](auto&&... args)
					{
						return self.operator()(std::forward<decltype(args)>(args)...);
					}
				);
			}
	};
}