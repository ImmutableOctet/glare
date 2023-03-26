#include "meta.hpp"

#include "indirect_meta_any.hpp"

#include "meta_evaluation_context.hpp"

#include "meta_function_call.hpp"
#include "meta_type_reference.hpp"
#include "meta_data_member.hpp"
#include "indirect_meta_data_member.hpp"

namespace engine
{
	std::optional<PlayerIndex> resolve_player_index(const MetaAny& instance)
	{
		using namespace engine::literals;

		const auto type = instance.type();

		if (!type)
		{
			return std::nullopt;
		}

		auto [player_index_member_id, player_index_member] = resolve_data_member(type, true, "player_index");

		if (!player_index_member)
		{
			return std::nullopt;
		}

		const auto player_idx = player_index_member.get(instance);

		if (player_idx)
		{
			if (const auto idx_out = player_idx.try_cast<PlayerIndex>())
			{
				return *idx_out;
			}
		}

		return std::nullopt;
	}

	bool meta_any_is_string(const MetaAny& value)
	{
		if (!value)
		{
			return false;
		}

		const auto type = value.type();

		return meta_type_is_string(type);
	}

	bool meta_type_is_string(const MetaType& type)
	{
		//using namespace engine::literals;

		if (!type)
		{
			return false;
		}

		const auto type_id = type.id();

		return
		(
			(type_id == resolve<std::string>().id()) // "String"_hs // entt::type_id<std::string>().hash()
			||
			(type_id == resolve<std::string_view>().id()) // entt::type_id<std::string_view>().hash()
		);
	}

	std::optional<StringHash> meta_any_to_string_hash(const MetaAny& value)
	{
		if (!value)
		{
			return std::nullopt;
		}

		std::optional<StringHash> hash_out = std::nullopt;

		if
		(
			try_string_value
			(
				value, [&hash_out](const auto& str_value)
				{
					hash_out = hash(str_value);
				}
			)
		)
		{
			return hash_out;
		}

		if
		(
			try_value<StringHash>
			(
				value,
				[&hash_out](const auto& hash)
				{
					hash_out = hash;
				}
			)
		)
		{
			return hash_out;
		}

		if
		(
			try_value<std::optional<StringHash>>
			(
				value,
				[&hash_out](const auto& opt_hash)
				{
					hash_out = opt_hash;
				}
			)
		)
		{
			return hash_out;
		}

		return hash_out; // std::nullopt;
	}

	bool meta_any_string_compare(const MetaAny& left, const MetaAny& right)
	{
		auto left_hash = meta_any_to_string_hash(left);

		if (!left_hash)
		{
			return false;
		}

		auto right_hash = meta_any_to_string_hash(right);

		if (!right_hash)
		{
			return false;
		}

		return (left_hash == right_hash);
	}

	entt::meta_type meta_type_from_name(std::string_view type_name)
	{
		return util::meta_type_from_name(type_name, "engine");
	}

	std::optional<std::pair<entt::id_type, entt::meta_data>>
	get_local_data_member_by_index(const entt::meta_type& type, std::size_t variable_index)
    {
        auto data_range = type.data();
        auto data_it = (data_range.begin() + static_cast<std::ptrdiff_t>(variable_index));

        if (data_it < data_range.end()) // !=
        {
            return *data_it;
        }

        return std::nullopt;
    }

	std::optional<std::pair<entt::id_type, entt::meta_data>>
	get_data_member_by_index(const entt::meta_type& type, std::size_t variable_index, bool recursive)
    {
        if (!recursive)
        {
            return get_local_data_member_by_index(type, variable_index);
        }

        std::optional<std::pair<entt::id_type, entt::meta_data>> output = std::nullopt;
        std::size_t count = 0;

        enumerate_data_members
        (
            type,

            [&output, &count, variable_index](auto&& data_member_id, auto&& data_member)
            {
                if (count != variable_index)
                {
                    count++;

                    return true;
                }

                if (data_member)
                {
                    output = { data_member_id, data_member };
                }
                
                return false;
            }
        );

        return output;
    }

	MetaAny get_component_ref(Registry& registry, Entity entity, const MetaType& component_type)
	{
		using namespace engine::literals;

		if (!component_type)
		{
			return {};
		}

		if (entity == null)
		{
			return {};
		}

		if (auto get_component_fn = component_type.func("get_component"_hs))
		{
			auto component_ptr = get_component_fn.invoke
			(
				{},
				entt::forward_as_meta(registry),
				entt::forward_as_meta(entity)
			);

			if (component_ptr)
			{
				return *component_ptr;
			}
		}

		return {};
	}

	MetaAny get_component_ref(Registry& registry, Entity entity, MetaTypeID component_type_id)
	{
		return get_component_ref(registry, entity, resolve(component_type_id));
	}

	bool try_direct_cast(MetaAny& current_value, const MetaType& cast_type, bool ignore_same_type)
	{
		auto current_type = current_value.type();

		if ((current_type == cast_type) && ignore_same_type)
		{
			// No need to cast, the specified cast-type is the same.
			return true;
		}
		else
		{
			if (!type_has_indirection(current_type))
			{
				if (current_value.allow_cast(cast_type))
				{
					return true;
				}
				else if (auto construct_from = cast_type.construct(current_value))
				{
					current_value = construct_from;

					return true;
				}
			}
		}

		return false;
	}

	bool function_overload_has_meta_any_argument(const MetaFunction& function)
	{
		if (!function)
		{
			return false;
		}

		for (std::size_t i = 0; i < function.arity(); i++)
		{
			const auto arg_type = function.arg(i);

			if (arg_type.id() == entt::type_hash<MetaAny>::value()) // resolve<MetaAny>().id()
			{
				return true;
			}
		}

		return false;
	}

	bool function_has_meta_any_argument(const MetaFunction& function, bool check_overloads)
	{
		if (!check_overloads)
		{
			return function_overload_has_meta_any_argument(function);
		}

		if (!function)
		{
			return false;
		}

		auto target_overload = function;

		do
		{
			if (function_overload_has_meta_any_argument(target_overload))
			{
				return true;
			}

			target_overload = target_overload.next();
		} while (target_overload);

		return false;
	}

	MetaType try_get_underlying_type(const MetaAny& type_reference_value)
	{
		using namespace engine::literals;

		if (!type_reference_value)
		{
			return {};
		}
		
		const auto reference_type = type_reference_value.type();

		if (auto type_fn = reference_type.func("try_get_underlying_type"_hs))
		{
			if (auto result = invoke_any_overload(type_fn, type_reference_value))
			{
				if (auto result_as_type = result.try_cast<MetaType>())
				{
					return *result_as_type; // std::move(*result_as_type);
				}
				
				if (auto result_as_type_id = result.try_cast<MetaTypeID>())
				{
					if (auto result_as_type = resolve(*result_as_type_id))
					{
						return result_as_type;
					}
				}
			}
		}

		return {};
	}

	MetaType get_underlying_or_direct_type(const MetaAny& direct_or_indirect_value)
	{
		if (!direct_or_indirect_value)
		{
			return {};
		}

		if (auto type = try_get_underlying_type(direct_or_indirect_value))
		{
			return type;
		}

		if (!value_has_indirection(direct_or_indirect_value))
		{
			return direct_or_indirect_value.type();
		}

		return {};
	}
}