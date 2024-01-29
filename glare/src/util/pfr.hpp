#pragma once

#include <engine/meta/hash.hpp>

#include <boost/pfr.hpp>

#include "type_algorithms.hpp"

#include <type_traits>
#include <string_view>
#include <utility>

namespace util
{
	namespace pfr = boost::pfr;

	template <typename T>
	inline constexpr bool is_pfr_reflective_v = static_cast<bool>(pfr::tuple_size_v<std::remove_reference_t<T>>);

	template <typename T, bool recursive=false, typename MemberCallback>
	constexpr void enumerate_aggregate_fields_ex(MemberCallback&& member_fn)
	{
		constexpr auto field_count = pfr::tuple_size_v<std::remove_reference_t<T>>;

		if constexpr (field_count)
		{
			using Test = std::remove_reference_t<T>;

			constexpr auto field_names = pfr::names_as_array<Test>();

			static_iota<field_count>
			(
				[&member_fn, &field_names, field_count, recursive]<auto field_index>()
				{
					constexpr auto& field_name_raw = field_names[field_index];
					constexpr auto field_name = std::string_view { field_name_raw.data(), field_name_raw.size() };

					using self_type = T;
					using member_type = std::remove_reference_t<decltype(pfr::get<field_index, T>(std::declval<T>()))>;

					if constexpr (recursive)
					{
						enumerate_aggregate_fields_ex<member_type, recursive>(member_fn);
					}

					member_fn.operator()<self_type, member_type, field_count, field_index, field_name>();
				}
			);
		}
	}

	template <typename T, bool recursive=false, typename MemberCallback>
	constexpr void enumerate_aggregate_fields(MemberCallback&& member_fn)
	{
		enumerate_aggregate_fields_ex<T, recursive>
		(
			[&member_fn]<typename SelfType, typename MemberType, auto total_member_count, auto member_index, auto member_name>()
			{
				constexpr auto member_id = engine::hash(member_name).value();
				
				member_fn.operator()<SelfType, MemberType, total_member_count, member_index, member_name, member_id>();
			}
		);
	}
}