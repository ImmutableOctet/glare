#pragma once

#include "conditional_event_yield_request.hpp"
#include "conditional_yield_request.hpp"
#include "opaque_value.hpp"
#include "script_traits.hpp"

#include <engine/types.hpp>
#include <engine/timer.hpp>

#include <engine/meta/types.hpp>

#include <engine/entity/entity_target.hpp>
#include <engine/entity/entity_instruction.hpp>

#include <util/type_traits.hpp>
#include <util/function_traits.hpp>

#include <utility>
#include <type_traits>
#include <string_view>
#include <tuple>

#if GLARE_DEBUG
#include <entt/core/type_info.hpp>
#endif // GLARE_DEBUG

namespace engine
{
	class Script;

	class EntityThreadInterface
	{
		public:
			template <typename EntryPointType>
			static EntityThreadID get_thread_name()
			{
				if constexpr (!util::is_standalone_function_v<EntryPointType>)
				{
					if (const auto type = engine::resolve<EntryPointType>())
					{
						const auto type_id = static_cast<EntityThreadID>(type.id());
#if GLARE_DEBUG
						const auto entry_point_type_name = entt::type_name<EntryPointType>::value();

						const auto entry_point_thread_name = util::format("{} ({})", entry_point_type_name, type_id);
						//const auto& entry_point_thread_name = entry_point_type_name;

						return engine::hash(entry_point_thread_name);
#else
						return type_id;
#endif // GLARE_DEBUG
					}
				}

				return {};
			}

			template <typename EntryPointType>
			static EntityThreadID get_thread_name(EntryPointType&& entry_point_fn)
			{
				using EntryPointTypeDecayed = std::decay_t<EntryPointType>;

				if constexpr ((std::is_same_v<EntryPointTypeDecayed, EntityThreadID>) || (std::is_same_v<EntryPointTypeDecayed, entt::hashed_string>))
				{
					return entry_point_fn;
				}
				else if constexpr (std::is_same_v<std::decay_t<EntryPointType>, std::string_view>)
				{
					return engine::hash(entry_point_fn);
				}
				else if constexpr (util::is_standalone_function_v<EntryPointType>)
				{
					// Generate a name at runtime based on the specific function pointer specified:
					const auto entry_point_function_name = util::format("Function 0x{:X}", reinterpret_cast<std::uintptr_t>(entry_point_fn));

					return engine::hash(entry_point_function_name);
				}
				else
				{
					return get_thread_name<EntryPointType>();
				}
			}

			template <auto entry_point_fn>
			static EntityThreadID get_thread_name()
			{
				return get_thread_name(entry_point_fn);
			}

			template <typename InstructionType, typename ...InstructionArgs>
			[[nodiscard]] EntityInstruction instruct(const EntityTarget& target, InstructionArgs&&... instruction_args)
			{
				return InstructionType { target, EntityThreadID {}, std::forward<InstructionArgs>(instruction_args)...};
			}

			template <typename InstructionType, typename ...InstructionArgs>
			[[nodiscard]] EntityInstruction instruct(const EntityTarget& target, EntityThreadID thread_id, InstructionArgs&&... instruction_args)
			{
				return InstructionType { target, { thread_id }, std::forward<InstructionArgs>(instruction_args)... };
			}

			template <typename InstructionType, typename ...InstructionArgs>
			[[nodiscard]] EntityInstruction instruct(InstructionArgs&&... instruction_args)
			{
				return InstructionType { EntityTarget {}, EntityThreadID {}, std::forward<InstructionArgs>(instruction_args)...};
			}

			[[nodiscard]] decltype(auto) sleep(auto&&... args) { return instruct<instructions::Sleep>(std::forward<decltype(args)>(args)...); }

			// Generates a request to yield until any event has been emitted from a relevant `Service`. (Yield operation)
			// This does not directly affect control-flow, but may be used in conjunction with `co_yield` to submit a request.
			[[nodiscard]] constexpr EventYieldRequest until_any_event() const
			{
				return {};
			}

			// Yields until the next frame/update. (Suspend operation)
			// This does not directly affect control-flow, but may be used in conjunction with `co_yield` to submit a request.
			[[nodiscard]] constexpr ControlFlowToken until_next_frame() const
			{
				return ControlFlowToken::NextFrame;
			}

			// Yields until the next frame/update. (Suspend operation)
			// This does not directly affect control-flow, but may be used in conjunction with `co_yield` to submit a request.
			[[nodiscard]] constexpr ControlFlowToken until_next_update() const
			{
				return ControlFlowToken::NextUpdate;
			}

			// Yields until the thread controlling this script is woken up by an external source. (Pause operation)
			// This does not directly affect control-flow, but may be used in conjunction with `co_yield` to submit a request.
			[[nodiscard]] constexpr ControlFlowToken until_wake() const
			{
				return ControlFlowToken::UntilWake;
			}

			// Yields until the thread controlling this script is woken up by an external source. (Pause operation)
			// This does not directly affect control-flow, but may be used in conjunction with `co_yield` to submit a request.
			[[nodiscard]] constexpr ControlFlowToken until_woken() const
			{
				return ControlFlowToken::UntilWoken;
			}

			template <typename EventType>
			[[maybe_unused]] decltype(auto) event(this auto&& self, EventType&& event_instance)
			{
				self.template event_impl<EventType>(std::forward<EventType>(event_instance));

				return self.template until_event<EventType>();
			}

			template <typename EventType, typename... Args>
			[[maybe_unused]] decltype(auto) event(this auto&& self, Args&&... args)
			{
				self.template event_impl<EventType>(std::forward<Args>(args)...);

				return self.template until_event<EventType>();
			}

			template <typename EventType, typename... Args>
			[[maybe_unused]] decltype(auto) event(this auto&& self, Timer timer, Args&&... args)
			{
				self.template event_impl<EventType>(std::move(timer), std::forward<Args>(args)...);

				return self.template until_event<EventType>();
			}

			// Generates a request to yield until an `EventType` object has been emitted from a relevant `Service`. (Yield operation)
			// This does not directly affect control-flow, but may be used in conjunction with `co_yield` to submit a request.
			template <typename EventType>
			[[nodiscard]] decltype(auto) until(this auto&& self)
			{
				return self.template until_event<EventType>();
			}

			template <typename EventType, auto value>
			[[nodiscard]] decltype(auto) until(this auto&& self)
			{
				return self.template until_event<EventType, value>();
			}
			
			[[nodiscard]] decltype(auto) until(this auto&& self, auto&& predicate_or_predicate_factory, auto&& first_factory_arg, auto&&... factory_args)
			{
				return self.until_impl_event_or_pause(std::forward<decltype(predicate_or_predicate_factory)>(predicate_or_predicate_factory), std::forward<decltype(first_factory_arg)>(first_factory_arg), std::forward<decltype(factory_args)>(factory_args)...);
			}

			template <typename EventType=void>
			[[nodiscard]] auto until(this auto&& self, auto&& predicate_or_value)
			{
				using PredicateOrValue = decltype(predicate_or_value);

				if constexpr (std::is_same_v<EventType, void>)
				{
					using expected_type = decltype(self.until_impl_event_or_pause(std::declval<PredicateOrValue>()));

					constexpr bool event_type_failed_to_resolve = false; // (util::is_specialization_v<expected_type, ConditionalYieldRequest>);

					if constexpr (event_type_failed_to_resolve)
					{
						static_assert(util::always_false<expected_type>, "Failed to resolve event type based on `predicate_or_value`.");
					}
					else
					{
						return self.until_impl_event_or_pause(std::forward<PredicateOrValue>(predicate_or_value));
					}
				}
				else
				{
					return self.template until_event<EventType>(std::forward<PredicateOrValue>(predicate_or_value));
				}
			}

			// Generates a request to yield until an emitted event supplies the `yield_value` specified. (Yield operation)
			// This does not directly affect control-flow, but may be used in conjunction with `co_yield` to submit a request.
			template <auto yield_value>
			[[nodiscard]] decltype(auto) until(this auto&& self)
			{
				using yield_value_t = decltype(yield_value);
				using yield_value_decayed_t = std::remove_cv_t<decltype(yield_value)>;

				if constexpr (util::is_standalone_function_v<yield_value_t>)
				{
					return self.until(std::forward<decltype(yield_value)>(yield_value));
				}
				else if constexpr (std::is_same_v<yield_value_decayed_t, Timer>)
				{
					return self.template wait_for(yield_value);
				}
				else if constexpr (Timer::IsDurationType<yield_value_decayed_t>)
				{
					return self.template wait_for(Timer { yield_value });
				}
				else
				{
					return self.template until_value<yield_value>();
				}
			}

			// Generates a request to yield until an `EventType` object has been emitted from a relevant `Service`. (Yield operation)
			// This does not directly affect control-flow, but may be used in conjunction with `co_yield` to submit a request.
			// 
			// TODO: Optimize. -- This was originally equivalent to `until_event_request_impl`,
			// but returning `EventYieldRequest` directly had the downside of not allowing for `co_await` syntax.
			template <typename EventType>
			[[nodiscard]] auto until_event(this auto&& self)
			{
				return self.template until_event<EventType>([](const EventType&) { return true; });
			}

			template <typename EventType, auto value>
			[[nodiscard]] auto until_event(this auto&& self)
			{
				return self.template until_event<EventType>(value);
			}

			// Generates a request to yield until an `EventType` object has been emitted and the `predicate` specified returns true. (Yield operation)
			// This does not directly affect control-flow, but may be used in conjunction with `co_yield` to submit a request.
			template <typename EventType, typename PredicateOrValue>
			[[nodiscard]] auto until_event(this auto&& self, PredicateOrValue&& predicate_or_value)
			{
				using EventTypeDecayed = std::remove_cvref_t<EventType>;

				if constexpr (engine::script::traits::is_continuation_predicate_wrappable_v<EventTypeDecayed, PredicateOrValue>)
				{
					return ConditionalEventYieldRequest<EventTypeDecayed, PredicateOrValue> { self, self.template until_event_request_impl<EventTypeDecayed>(), std::forward<PredicateOrValue>(predicate_or_value) };
				}
				else
				{
					return self.template until_value<EventTypeDecayed>(std::forward<PredicateOrValue>(predicate_or_value));
				}
			}

			// Yields until the thread controlling this script is woken up by an external source. (Pause operation)
			// This does not directly affect control-flow, but may be used in conjunction with `co_yield` to submit a request.
			[[nodiscard]] constexpr auto pause() const
			{
				return until_woken();
			}

			template <typename PredicateType>
			[[nodiscard]] auto pause(this auto&& self, PredicateType&& predicate)
			{
				return ConditionalYieldRequest
				{
					self,

					self.until_woken(),

					std::forward<PredicateType>(predicate)
				};
			}

			// Indicates a request to wait for the temporary `timer` specified.
			// This does not directly yield, but may be used in conjunction with `co_yield` to submit a request.
			[[nodiscard]] auto wait_until(this auto&& self, const Timer& timer)
			{
				return ConditionalYieldRequest
				{
					self,
					timer,

					[](const Timer& timer)
					{
						return static_cast<bool>(timer);
					}
				};

				//return timer;
			}

			[[nodiscard]] decltype(auto) wait_for(this auto&& self, auto&&... args)
			{
				return self.wait_until(std::forward<decltype(args)>(args)...);
			}

			// Generates a request to yield until an `EventType` event is emitted holding the `yield_value` specified. (Yield operation)
			// This does not directly yield, but may be used in conjunction with `co_yield` to submit a request.
			template <typename EventType, typename YieldValueType, typename ...AdditionalArgs>
			[[nodiscard]] decltype(auto) until_value(this auto&& self, YieldValueType&& yield_value, AdditionalArgs&&... additional_args)
			{
				using yield_value_t = std::remove_cvref_t<YieldValueType>;

				if constexpr (std::is_same_v<yield_value_t, ControlFlowToken>)
				{
					return yield_value;
				}
				else
				{
					return self.template until_event<EventType>
					(
						[yield_value](const Script& self, const MetaAny& opaque_value) -> bool
						{
							return impl::compare_opaque_value_impl<EventType>(self, opaque_value, yield_value);
						},

						std::forward<AdditionalArgs>(additional_args)...
					);
				}
			}

			// Generates a request to yield until an emitted event supplies the `yield_value` specified. (Yield operation)
			// This does not directly yield, but may be used in conjunction with `co_yield` to submit a request.
			template <auto yield_value>
			[[nodiscard]] decltype(auto) until_value(this auto&& self)
			{
				return self.template until_any_event_has_value<yield_value>();
			}

			// Generates a request to yield until any event supplies the `yield_value` specified. (Yield operation)
			// This does not directly yield, but may be used in conjunction with `co_yield` to submit a request.
			template <auto yield_value>
			[[nodiscard]] decltype(auto) until_any_event_has_value(this auto&& self)
			{
				using yield_value_t = std::remove_cvref_t<decltype(yield_value)>;

				if constexpr (std::is_same_v<yield_value_t, ControlFlowToken>)
				{
					return yield_value;
				}
				else
				{
					self.set_yield_continuation_predicate
					(
						[](const Script& self, const MetaAny& opaque_value) -> bool
						{
							return impl::compare_opaque_value_impl<yield_value_t>(self, opaque_value, yield_value);
						}
					);

					return self.until_any_event();
				}
			}

			// Generates a request to yield until any event supplies the `yield_value` specified. (Yield operation)
			// This does not directly yield, but may be used in conjunction with `co_yield` to submit a request.
			template <typename YieldType>
			[[nodiscard]] decltype(auto) until_any_event_has_value(this auto&& self, YieldType&& yield_value)
			{
				using yield_value_t = std::remove_cvref_t<YieldType>;

				if constexpr (std::is_same_v<yield_value_t, ControlFlowToken>)
				{
					return yield_value;
				}
				else
				{
					self.set_yield_continuation_predicate
					(
						[yield_value](const Script& self, const MetaAny& opaque_value) -> bool
						{
							return impl::compare_opaque_value_impl<yield_value_t>(self, opaque_value, yield_value);
						}
					);

					return self.until_any_event();
				}
			}

		protected:
			// Indicates a request for a state change using a hash value, representing the state's identifier/name.
			// This does not directly alter any entity, but may be used in conjunction with `co_yield` to submit a request.
			// 
			// NOTE: If a state change is successful, and this script's thread is attached to the old state, this script will be terminated.
			[[nodiscard]] constexpr EntityStateHash state_change(EntityStateHash state_id) const
			{
				return state_id;
			}

			// Indicates a request for a state change using a string view.
			// This does not directly alter any entity, but may be used in conjunction with `co_yield` to submit a request.
			// 
			// NOTE: If a state change is successful, and this script's thread is attached to the old state, this script will be terminated.
			[[nodiscard]] constexpr EntityStateHash state_change(std::string_view state_name) const
			{
				return engine::hash(state_name);
			}

			template <typename ConditionFunctionType, typename PredicateType, typename... Args>
			decltype(auto) until_impl_event_or_pause_ex_deduce_event_from_function(this auto&& self, PredicateType&& predicate, Args&&... args)
			{
				using function_traits = util::function_traits<ConditionFunctionType>;
				using argument_types = function_traits::argument_types;

				constexpr auto argument_count = std::tuple_size_v<argument_types>;

				if constexpr (argument_count > 0)
				{
					using first_argument = std::tuple_element_t<0, argument_types>;
					using first_argument_decayed = std::remove_cvref_t<first_argument>;

					constexpr bool first_argument_is_script = (std::is_same_v<first_argument_decayed, Script>);

					if constexpr (argument_count == 1)
					{
						constexpr bool is_invocable_as_event_only = (!first_argument_is_script);
						constexpr bool first_argument_is_meta_any = (std::is_same_v<first_argument_decayed, MetaAny>);

						if constexpr (first_argument_is_meta_any)
						{
							// Produce an error at compile-time.
							static_assert(util::always_false<first_argument_decayed>, "Unable to determine event type: First argument of callable produced by `predicate` cannot be `MetaAny`.");

							// Alternative implementation:
							//return self.pause(std::forward<PredicateType>(predicate));
						}
						else if constexpr (!first_argument_is_script)
						{
							using event_type = first_argument;

							return self.template until_event<event_type>(std::forward<PredicateType>(predicate));
						}
						else
						{
							return self.pause(std::forward<PredicateType>(predicate));
						}
					}
					else if constexpr (first_argument_is_script)
					{
						using second_argument = std::tuple_element_t<1, argument_types>;
						using second_argument_decayed = std::remove_cvref_t<second_argument>;

						constexpr bool second_argument_is_meta_any = (std::is_same_v<second_argument_decayed, MetaAny>);

						if constexpr (second_argument_is_meta_any)
						{
							// Produce an error at compile-time.
							static_assert(util::always_false<second_argument_decayed>, "Unable to determine event type: Second argument of callable produced by `predicate` cannot be `MetaAny`.");

							// Alternative implementation:
							//return self.pause(std::forward<PredicateType>(predicate));
						}
						else
						{
							using event_type = second_argument;

							return self.template until_event<event_type>(std::forward<PredicateType>(predicate));
						}
					}
					else
					{
						return self.pause(std::forward<PredicateType>(predicate));
					}
				}
				else
				{
					return self.pause(std::forward<PredicateType>(predicate));
				}
			}

			template <typename EventType>
			constexpr EventYieldRequest until_event_request_impl() const
			{
				return { engine::resolve<EventType>().id() };
			}

			template <typename PredicateType, typename... Args>
			decltype(auto) until_impl_event_or_pause_ex(this auto&& self, PredicateType&& predicate, Args&&... args)
			{
				if constexpr (std::is_invocable_v<PredicateType, Args...>)
				{
					using result_type = std::invoke_result_t<PredicateType, Args...>;
					using result_type_decayed = std::decay_t<result_type>;

					if constexpr (std::is_same_v<bool, result_type_decayed> || std::is_same_v<void, result_type_decayed>)
					{
						return self.pause(std::forward<PredicateType>(predicate));
					}
					else
					{
						return self.template until_impl_event_or_pause_ex_deduce_event_from_function<result_type>(std::forward<PredicateType>(predicate), std::forward<Args>(args)...);
					}
				}
				else
				{
					return self.template until_impl_event_or_pause_ex_deduce_event_from_function<PredicateType>(std::forward<PredicateType>(predicate), std::forward<Args>(args)...);
				}
			}

			template <typename PredicateOrFactory, typename... Args>
			decltype(auto) until_impl_event_or_pause(this auto&& self, PredicateOrFactory&& predicate_or_factory, Args&&... args)
			{
				if constexpr (std::is_invocable_v<PredicateOrFactory, decltype(self), Args...>)
				{
					return self.until_impl_event_or_pause_ex(std::forward<PredicateOrFactory>(predicate_or_factory), self, std::forward<Args>(args)...);
				}
				else
				{
					return self.until_impl_event_or_pause_ex(std::forward<PredicateOrFactory>(predicate_or_factory), std::forward<Args>(args)...);
				}
			}
	};
}