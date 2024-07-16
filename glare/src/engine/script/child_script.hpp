#pragma once

//#include "script.hpp"
#include "script_entry_point.hpp"
#include "script_function_bootstrap.hpp"

#include <engine/types.hpp>

#include <util/type_algorithms.hpp>

#include <utility>
#include <type_traits>

namespace engine
{
	template <typename BaseScriptType, typename EntryPointFn>
	class ChildScript : public BaseScriptType
	{
		protected:
			ChildScript() = default;

		public:
			using ScriptID = StringHash;

			ChildScript
			(
				engine::Registry& registry,
				engine::Entity entity,
				const engine::MetaEvaluationContext& context,

				EntryPointFn&& entry_point,

				ScriptID script_id={}
			) :
				BaseScriptType(context, registry, script_id, entity),

				script_entry_point(std::move(entry_point))
			{}

			ChildScript(ChildScript&&) noexcept = default;

			ChildScript& operator=(const ChildScript&) = delete;
			ChildScript& operator=(ChildScript&&) noexcept = default;

			using BaseScriptType::operator();

			engine::ScriptFiber operator()(GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST)
			{
				return exhaustive_call_operator_impl(GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST_NAMES);
			}

			template
			<
				typename... EntryPointArgs,

				typename std::enable_if
				<
					(
						(sizeof...(EntryPointArgs) > 0)
						&&
						(
							!std::is_same_v
							<
								std::tuple<EntryPointArgs...>,
								std::tuple<GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST_TYPES>
							>
						)
					),
					
					int
				>::type=0
			>
			engine::ScriptFiber operator()(EntryPointArgs&&... entry_point_args)
			{
				return exhaustive_call_operator_impl(std::forward<EntryPointArgs>(entry_point_args)...);
			}
		private:
			template <typename... CallArgs>
			ScriptFiber exhaustive_call_operator_impl(CallArgs&&... call_args)
			{
				return util::drop_last_until_success<0>
				(
					[&](auto&&... args)
					{
						if constexpr (std::is_invocable_v<EntryPointFn, decltype(args)...>)
						{
							return script_entry_point(std::forward<decltype(args)>(args)...);
						}
						else
						{
							return ScriptFiber {};
						}
					},

					std::forward<CallArgs>(call_args)...
				);
			}

			EntryPointFn script_entry_point;
	};
}