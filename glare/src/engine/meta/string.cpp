#include "string.hpp"

#include "hash.hpp"
#include "indirection.hpp"
#include "meta_any.hpp"
#include "resolve.hpp"

namespace engine
{
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

	std::optional<StringHash> meta_any_to_string_hash
	(
		const MetaAny& value,

		bool try_raw_hash_type,
		bool try_conversion_to_raw_hash_type,
		bool try_conversion_to_string
	)
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
				value,
				
				[&hash_out](const auto& str_value)
				{
					hash_out = hash(str_value);
				}
			)
		)
		{
			return hash_out;
		}

		if (try_raw_hash_type)
		{
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
		}

		if (try_conversion_to_raw_hash_type)
		{
			if (auto converted_to_hash_value = value.allow_cast<StringHash>())
			{
				return converted_to_hash_value.cast<StringHash>();
			}

			if (auto converted_to_hash_value = value.allow_cast<std::optional<StringHash>>())
			{
				return converted_to_hash_value.cast<std::optional<StringHash>>();
			}
		}

		if (try_conversion_to_string)
		{
			if (auto converted_to_string = value.allow_cast<std::string_view>())
			{
				return hash(converted_to_string.cast<std::string_view>()).value();
			}

			if (auto converted_to_string = value.allow_cast<std::string>())
			{
				return hash(converted_to_string.cast<std::string>()).value();
			}
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
}