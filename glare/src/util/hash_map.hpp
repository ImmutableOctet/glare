#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <functional>
#include <utility>

namespace util
{
	// TODO: Look into using entt's string hashing function instead of `std::hash`.
	struct string_hash
	{
		protected:
			template <typename StringType, typename HashType=StringType>
			inline static std::size_t hash_impl(const StringType& str)
			{
				auto hash_functor = std::hash<HashType>{};

				return hash_functor(str);
			}
		public:
			using is_transparent = void;

			[[nodiscard]]
			inline std::size_t operator()(const char* str) const
			{
				return hash_impl(std::string_view { str }); // operator()(...);
			}

			[[nodiscard]]
			inline std::size_t operator()(std::string_view str) const
			{
				return hash_impl(str);
			}

			[[nodiscard]]
			inline std::size_t operator()(const std::string& str) const
			{
				return hash_impl(str);
			}

			/*
			[[nodiscard]]
			inline std::size_t operator()(std::size_t hash) const
			{
				return hash;
			}
			*/
	};

	template <typename KeyType, typename ValueType>
	using basic_hash_map = std::unordered_map<KeyType, ValueType, string_hash, std::equal_to<>>;

	template <typename ValueType>
	using hash_map = basic_hash_map<std::string, ValueType>;
}