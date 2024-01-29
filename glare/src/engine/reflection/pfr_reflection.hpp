#pragma once

#include <engine/meta/types.hpp>

#include <util/pfr.hpp>
#include <util/type_algorithms.hpp>

#include <entt/meta/meta.hpp>

#include <type_traits>
#include <string_view>
#include <utility>

namespace engine
{
	//=[]<typename NewType>(){ return engine_meta_type<NewType>(); }
	template <typename T, bool recursive=true>
	decltype(auto) reflect_aggregate_fields(auto&& type, auto&& new_type_fn)
	{
		// NOTE: Recursion handled at this level, rather than via `enumerate_aggregate_fields`.
		util::enumerate_aggregate_fields<T, false>
		(
			[&type, &new_type_fn]<typename SelfType, typename MemberType, auto total_member_count, auto member_index, auto member_name, auto member_id>()
			{
				static_assert(!std::is_same_v<std::remove_reference_t<SelfType>, std::remove_reference_t<MemberType>>);

				if constexpr (recursive)
				{
					const auto nested_type_already_reflected = engine::resolve<MemberType>();

					if (!nested_type_already_reflected)
					{
						reflect_aggregate_fields<T, recursive>(new_type_fn);
					}
				}
				
				constexpr auto getter_ptr = (+[](const SelfType& instance) { return util::pfr::get<member_index>(instance); });

				if constexpr (std::is_copy_assignable_v<MemberType>)
				{
					constexpr auto setter_ptr = (+[](SelfType& instance, const MemberType& member_value) { util::pfr::get<member_index>(instance) = member_value; });

					type = type.data<setter_ptr, getter_ptr>(member_id);
				}
				else if constexpr (std::is_move_assignable_v<MemberType>)
				{
					constexpr auto setter_ptr = (+[](SelfType& instance, MemberType&& member_value) { util::pfr::get<member_index>(instance) = std::move(member_value); });

					type = type.data<setter_ptr, getter_ptr>(member_id);
				}
				else
				{
					type = type.data<nullptr, getter_ptr>(member_id);
				}
			}
		);

		return type;
	}

	template <typename T, bool recursive=true>
	decltype(auto) reflect_aggregate_fields(auto&& new_type_fn)
	{
		auto type = new_type_fn();

		return reflect_aggregate_fields<T, recursive>(std::forward<decltype(type)>(type), std::forward<decltype(new_type_fn)>(new_type_fn));
	}
}