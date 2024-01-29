#pragma once

#include <engine/meta/hash.hpp>

#include <util/pfr.hpp>
#include <util/type_algorithms.hpp>

#include <type_traits>
#include <utility>

namespace engine
{
	template <typename T>
	auto reflect_aggregate_fields(auto type)
	{
		constexpr auto field_count = util::pfr::tuple_size_v<std::remove_reference_t<T>>;

		if constexpr (field_count)
		{
			using Test = std::remove_reference_t<T>;

			constexpr auto field_names = util::pfr::names_as_array<Test>();

			/*
			util::static_iota<field_count>
			(
				[&field_names, &type]<auto field_index>()
				{
					const auto& field_name = field_names[field_index];
					const auto field_name_id = engine::hash(std::string_view { field_name.data(), field_name.size() }).value();

					using member_type = std::remove_reference_t<decltype(util::pfr::get<field_index, T>(std::declval<T>()))>;

					constexpr auto getter_ptr = (+[](const T& instance) { return util::pfr::get<field_index>(instance); });

					if constexpr (std::is_copy_assignable_v<member_type>)
					{
						constexpr auto setter_ptr = (+[](T& instance, const member_type& member_value) { util::pfr::get<field_index>(instance) = member_value; });

						type = type.data<setter_ptr, getter_ptr>(field_name_id);
					}
					else
					{
						type = type.data<nullptr, getter_ptr>(field_name_id);
					}
				}
			);
			*/
		}

		return type;
	}
}