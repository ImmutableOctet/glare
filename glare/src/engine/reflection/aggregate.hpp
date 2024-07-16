#pragma once

#include "core.hpp"

#include <engine/meta/types.hpp>

#include <util/pfr.hpp>
#include <util/type_algorithms.hpp>

#include <entt/meta/meta.hpp>

#include <type_traits>
#include <string_view>
#include <utility>

#if (GLARE_BOOST_PFR_ENABLED) // || (GLARE_BOOST_REFLECT_ENABLED)
	#define GLARE_AGGREGATE_REFLECTION_SUPPORTED 1
#endif

namespace engine
{
	namespace impl
	{
		// Returns true if `type` has at least one data-member. (Does not check base types)
		inline bool type_has_data_member_exclusive(const MetaType& type)
		{
			if (!type)
			{
				return false;
			}

			const auto known_data_members = type.data();

			return (known_data_members.cbegin() != known_data_members.cend());
		}

#if GLARE_BOOST_PFR_ENABLED
		namespace boost_pfr
		{
			template <typename T, bool recursive=true>
			auto& reflect_aggregate_fields_impl(auto& type, auto&& recurse_fn)
			{
				// NOTE: Recursion handled at this level, rather than via `enumerate_aggregate_fields`.
				util::enumerate_aggregate_fields<T, false>
				(
					[&type, &recurse_fn]<typename SelfType, typename MemberType, auto member_count, auto member_index, const auto& member_name, auto member_id>()
					{
						static_assert(!std::is_same_v<std::remove_reference_t<SelfType>, std::remove_reference_t<MemberType>>);

						if constexpr (recursive)
						{
							if constexpr (!std::is_fundamental_v<MemberType>)
							{
								if (!impl::type_has_data_member_exclusive(engine::resolve<MemberType>()))
								{
									recurse_fn.operator()<MemberType>();
								}
							}
						}
				
						static constexpr auto getter_ptr = +[](const SelfType& instance) { return util::pfr::get<member_index>(instance); };

						static constexpr auto copy_assignment_ptr = []
						{
							if constexpr (std::is_copy_assignable_v<MemberType>)
							{
								return +[](SelfType& instance, const MemberType& member_value) { return (util::pfr::get<member_index>(instance) = member_value); };
							}
							else
							{
								return nullptr;
							}
						}();

						static constexpr auto move_assignment_ptr = []
						{
							if constexpr (std::is_move_assignable_v<MemberType>)
							{
								return +[](SelfType& instance, MemberType&& member_value) { return (util::pfr::get<member_index>(instance) = std::move(member_value)); };
							}
							else
							{
								return nullptr;
							}
						}();

						if constexpr ((copy_assignment_ptr) && (move_assignment_ptr))
						{
							//type = type.data<entt::value_list<copy_assignment_ptr, move_assignment_ptr>, getter_ptr>(member_id);
							type = type.data<copy_assignment_ptr, getter_ptr>(member_id);
						}
						else if constexpr (copy_assignment_ptr)
						{
							type = type.data<copy_assignment_ptr, getter_ptr>(member_id);
						}
						else if constexpr (move_assignment_ptr)
						{
							type = type.data<move_assignment_ptr, getter_ptr>(member_id);
						}
						else
						{
							type = type.data<nullptr, getter_ptr>(member_id);
						}
					}
				);

				return type;
			}
		}
#endif // GLARE_BOOST_PFR_ENABLED

#if GLARE_BOOST_REFLECT_ENABLED
		namespace boost_reflect
		{
			static_assert(false, "Support for Boost-ext reflect is unfinished.")
		}
#endif // GLARE_BOOST_REFLECT_ENABLED

#if GLARE_BOOST_PFR_ENABLED
		using boost_pfr::reflect_aggregate_fields_impl;
#elif GLARE_BOOST_REFLECT_ENABLED
		using boost_reflect::reflect_aggregate_fields_impl;
#endif
	}


	template <typename T, bool recursive=true>
	auto reflect_aggregate_fields(auto&& new_type_fn, auto&& recurse_fn)
	{
		auto type = new_type_fn.operator()<T>();

		impl::reflect_aggregate_fields_impl<T, recursive>(type, std::forward<decltype(recurse_fn)>(recurse_fn));

		return type;
	}

	template <typename T, bool recursive=true>
	decltype(auto) reflect_aggregate_fields()
	{
		auto new_type_fn = []<typename NewType>()
		{
			return engine_meta_type<NewType>();
		};

		auto recurse_fn = []<typename MemberType>()
		{
			// Alternative implementation:
			//reflect_aggregate_fields<MemberType, recursive>(new_type_fn);

			reflect<MemberType>();
		};

		return reflect_aggregate_fields<T, recursive>
		(
			new_type_fn,
			recurse_fn
		);
	}
}