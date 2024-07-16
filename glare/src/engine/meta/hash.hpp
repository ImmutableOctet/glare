#pragma once

#include "types.hpp"

#include <util/type_traits.hpp>
#include <util/variant.hpp>

#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <utility>
//#include <cstring>

namespace engine
{
    namespace impl
    {
        using StringHashLookupTable = std::unordered_map<StringHash, std::variant<std::string, std::string_view>>;

        StringHashLookupTable& get_known_string_hashes();
    }

	// Computes a hash for the string specified.
    template <typename StringType>
    constexpr entt::hashed_string hash(StringType&& str, bool allow_result_storage=true)
    {
        if constexpr (util::is_c_str_v<StringType>)
        {
            auto result = entt::hashed_string(str);

            if (!std::is_constant_evaluated()) // !consteval
            {
                if (allow_result_storage)
                {
                    auto& known_string_hashes = impl::get_known_string_hashes();

                    if (true) // !known_string_hashes.contains(result)
                    {
                        known_string_hashes[result] = std::string_view { str };
                    }
                }
            }

            return result;
        }
        else
        {
            //assert(!str.empty());

            /*
            if (str.empty())
            {
                return {};
            }
            */

            auto result = entt::hashed_string(str.data(), str.length());

            if (!std::is_constant_evaluated()) // !consteval
            {
                if (allow_result_storage)
                {
                    auto& known_string_hashes = impl::get_known_string_hashes();

                    if (!known_string_hashes.contains(result)) // std::is_same_v<std::decay_t<StringType>, std::string> || ...
                    {
                        known_string_hashes[result] = std::string(std::forward<StringType>(str));
                    }
                }
            }

            return result;
        }
    }

    namespace literals
    {
        [[nodiscard]] constexpr entt::hashed_string operator"" _hs(const char* str, std::size_t)
        {
            return hash(str);
        }
    }

    // Attempts to retrieve a view to a string that has been hashed before.
    // 
    // If `hash_value` has not been generated by a previous (non-consteval)
    // call to `hash`, this will return an empty string view.
    std::string_view get_known_string_from_hash(StringHash hash_value);
}