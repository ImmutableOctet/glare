#pragma once

#include <type_traits>
#include <bit>

#include <cstddef>

namespace util
{
    // TODO: Implement pre-C++20 versions of these functions as fallbacks:
    constexpr bool is_big_endian()
    {
        return (std::endian::native == std::endian::big);
    }

    constexpr bool is_little_endian()
    {
        return (std::endian::native == std::endian::little);
    }

    constexpr bool is_mixed_endian()
    {
        return ((!is_big_endian()) && (!is_little_endian()));
    }

    template <typename T, typename=std::enable_if_t<std::is_arithmetic_v<T>>>
    T byte_swap(T value_in)
    {
        using byte_t = char; // unsigned char; // std::uint8_t;

        T value_out; // = {};
    
        const auto value_in_as_bytes = reinterpret_cast<const byte_t*>(&value_in);
    
        auto value_out_as_bytes = reinterpret_cast<byte_t*>(&value_out);

        constexpr auto value_size = sizeof(T);

        for (std::size_t byte_index = 0; byte_index < value_size; byte_index++)
        {
            const auto opposite_byte_index = ((value_size - 1) - byte_index);

            value_out_as_bytes[opposite_byte_index] = value_in_as_bytes[byte_index];
        }

        return value_out;
    }

    template <typename T, typename=std::enable_if_t<std::is_arithmetic_v<T>>>
    T host_to_network_byte_order(T value_in)
    {
        if constexpr (is_big_endian()) // (std::endian::native == std::endian::big)
        {
            return value_in;
        }
        else // if constexpr (std::endian::native == std::endian::little)
        {
            return byte_swap(value_in);
        }
    }

    template <typename T, typename=std::enable_if_t<std::is_arithmetic_v<T>>>
    T network_to_host_byte_order(T value_in)
    {
        return host_to_network_byte_order(value_in);
    }
}