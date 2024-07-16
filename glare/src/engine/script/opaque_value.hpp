#pragma once

#include <engine/common_type_traits.hpp>

#include <engine/meta/types.hpp>
#include <engine/meta/hash.hpp>
#include <engine/meta/common_members.hpp>

#include <util/type_traits.hpp>

#include <type_traits>

namespace engine
{
	namespace impl
	{
		template <typename SelfType, typename IntendedType, typename CallbackType>
		bool observe_resolved_opaque_value_impl(SelfType&& self, const MetaAny& opaque_value, CallbackType&& callback)
		{
			using intended_value_t = std::remove_cvref_t<IntendedType>;

			const auto opaque_value_type = opaque_value.type();

			if constexpr (engine::is_string_type_v<intended_value_t>)
			{
				if (const auto as_string = opaque_value.try_cast<std::string>())
				{
					if (callback(*as_string))
					{
						return true;
					}
				}

				if (const auto as_string_view = opaque_value.try_cast<std::string_view>())
				{
					if (callback(*as_string_view))
					{
						return true;
					}
				}

				if (const auto as_entity = try_get_entity(opaque_value); (as_entity != null))
				{
					const auto& registry = self.get_registry();

					if (const auto name_component = registry.try_get<NameComponent>())
					{
						if (callback(name_component->hash()))
						{
							return true;
						}
					}
				}
			}
			else
			{
				if (const auto as_value = opaque_value.try_cast<intended_value_t>())
				{
					if (callback(*as_value))
					{
						return true;
					}
				}
			}

			if constexpr (std::is_same_v<intended_value_t, Entity>)
			{
				if (callback(try_get_entity(opaque_value)))
				{
					return true;
				}
			}

			return false;
		}

		template <typename IntendedType, typename SelfType, typename YieldValueType>
		bool compare_resolved_opaque_value_impl(SelfType&& self, const MetaAny& opaque_value, YieldValueType&& yield_value)
		{
			using intended_value_t = std::remove_cvref_t<IntendedType>;
			
			return observe_resolved_opaque_value_impl<SelfType, IntendedType>
			(
				self,
				opaque_value,

				[&](auto&& value) -> bool
				{
					if constexpr (std::is_invocable_r_v<bool, decltype(yield_value), decltype(value)>)
					{
						if (yield_value(value))
						{
							return true;
						}
					}

					if constexpr (std::is_invocable_r_v<bool, decltype(yield_value), decltype(self)>)
					{
						if (yield_value(self))
						{
							return true;
						}
					}

					if constexpr (std::is_invocable_r_v<bool, decltype(yield_value), decltype(self), decltype(value)>)
					{
						if (yield_value(self, value))
						{
							return true;
						}
					}

					auto try_nested_overload = [&](auto&&... passthrough_args)
					{
						if constexpr (std::is_invocable_v<decltype(yield_value), decltype(passthrough_args)...>)
						{
							using result_type = std::invoke_result_t<decltype(yield_value), decltype(passthrough_args)...>;

							if constexpr (std::is_same_v<result_type, void>)
							{
								yield_value(std::forward<decltype(passthrough_args)>(passthrough_args)...);

								return true;
							}
							else
							{
								auto nested_fn = yield_value(std::forward<decltype(passthrough_args)>(passthrough_args)...);

								if constexpr (std::is_invocable_r_v<bool, result_type, decltype(value)>)
								{
									if (nested_fn(value))
									{
										return true;
									}
								}
								else if constexpr (std::is_invocable_r_v<bool, result_type>)
								{
									return nested_fn();
								}
								else if constexpr (std::is_invocable_v<result_type, decltype(value)>)
								{
									nested_fn(value);

									return true;
								}
								else if constexpr (std::is_invocable_v<result_type>)
								{
									nested_fn();

									return true;
								}
							}
						}

						return false;
					};

					if (try_nested_overload())
					{
						return true;
					}

					if (try_nested_overload(self))
					{
						return true;
					}

					if (try_nested_overload(self, opaque_value))
					{
						return true;
					}

					using supplied_value_t = std::remove_cvref_t<decltype(value)>;

					if constexpr (engine::is_string_type_v<intended_value_t>)
					{
						constexpr auto yield_value_hash = engine::hash(yield_value);

						if constexpr (std::is_same_v<supplied_value_t, StringHash>)
						{
							if (value == yield_value_hash)
							{
								return true;
							}
						}
						else
						{
							if (engine::hash(value) == yield_value_hash)
							{
								return true;
							}
						}
					}
					else if constexpr (std::is_same_v<intended_value_t, Entity>)
					{
						// TODO: Expand support for value comparison with entity types.
						if (value == yield_value)
						{
							return true;
						}
					}
					else if constexpr (util::is_equality_comparable_v<decltype(value), decltype(yield_value)>)
					{
						if (value == yield_value)
						{
							return true;
						}
					}

					return false;
				}
			);
		}

		template <typename IntendedType, typename SelfType, typename YieldValueType>
		bool compare_opaque_value_impl(SelfType&& self, const MetaAny& opaque_value, YieldValueType&& yield_value)
		{
			auto&       registry = self.get_registry();
			const auto  entity   = self.get_entity();
			const auto& context  = self.get_context();

			if (const auto resolved_value = get_indirect_value_or_ref(opaque_value, registry, entity, context))
			{
				if (compare_resolved_opaque_value_impl<IntendedType>(self, resolved_value, yield_value))
				{
					return true;
				}
				else
				{
					if (const auto opaque_type = resolved_value.type())
					{
						if (const auto data_member = std::get<1>(resolve_trigger_condition_member(opaque_type)))
						{
							if (const auto data_member_value = data_member.get(resolved_value))
							{
								if (compare_resolved_opaque_value_impl<IntendedType>(self, data_member_value, yield_value))
								{
									return true;
								}
							}
						}
					}
				}
			}

			return false;
		}
	}
}