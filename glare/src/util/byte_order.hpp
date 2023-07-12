#pragma once

#include <version>

#include <type_traits>

#if (defined(__cpp_lib_endian) || defined(__cpp_lib_byteswap))
    #include <bit>
#endif

#include <cstddef>

namespace util
{
#ifdef __cpp_lib_endian
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
#else
    // NOTE: Prior to C++20 there is no portable way to check endianness at compile-time.
    // With that in mind, however, type-punning still technically works on the major compilers.
    constexpr bool is_big_endian() // inline bool is_big_endian()
    {
        using byte_t = char; // unsigned char; // std::int8_t;

        /*
        // `reinterpret_cast` approach:
        std::uint32_t integer_value = 0x01020304;

        const auto* raw_bytes = reinterpret_cast<byte_t*>(&integer_value);

        const auto& left_byte = raw_bytes[0];

        return (left_byte == 1);
        */
    
        /*
        // Safe/portable approach, but doesn't work under `constexpr`:
        std::uint32_t integer_value = 0x01020304;

        byte_t raw_bytes[sizeof(integer_value)];

        std::memcpy(raw_bytes, &integer_value, sizeof(integer_value));

        const auto& left_byte = raw_bytes[0];

        return (left_byte == 1);
        */

        // Type-punning approach (technically UB):
        union
        {
            std::uint32_t integer_value;
            byte_t raw_bytes[sizeof(integer_value)];
        } value = { 0x01020304 };

        const auto& left_byte = value.raw_bytes[0];

        return (left_byte == 1);
    }

    constexpr bool is_little_endian() // inline bool is_little_endian()
    {
        return !is_big_endian();
    }

    constexpr bool is_mixed_endian()
    {
        //return ((!is_big_endian()) && (!is_little_endian()));

        return false;
    }
#endif

    template <typename T, typename=std::enable_if_t<std::is_arithmetic_v<T>>>
    T byte_swap(T value_in)
    {
        if constexpr (sizeof(value_in) > 1)
        {
#ifdef __cpp_lib_byteswap
            // If `std::byteswap` is available, use that implementation instead.
            // (Usually faster/safer)
            if constexpr (std::is_integral_v<T>)
            {
                return std::byteswap(value_in);
            }
#endif

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
        else
        {
            return value_in;
        }
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