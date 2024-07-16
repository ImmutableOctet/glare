#pragma once

//#define GLARE_UTIL_FIXED_STRING_USE_STD_ARRAY 1

#include <type_traits>
#include <algorithm>
#include <array>
#include <cstring>
#include <cstddef>

namespace util
{
    // A constexpr-friendly fixed-size string wrapper type.
    // This is used primarily for capturing strings as non-type template parameters.
	template <typename CharType, auto Length>
    struct basic_fixed_string
    {
        using value_type = CharType;
    
        using pointer_type = value_type*;
        using const_pointer_type = const value_type*;

        using iterator = pointer_type;
        using const_iterator = const_pointer_type;
    
        using size_type = std::size_t; // decltype(Length);
    
        inline static constexpr auto string_length         = static_cast<size_type>(Length);
        inline static constexpr auto string_buffer_size    = (string_length + static_cast<size_type>(1));
        inline static constexpr auto null_terminator       = static_cast<value_type>('\0');
        inline static constexpr auto null_terminator_index = string_length;

        template <typename StringType>
        static constexpr basic_fixed_string from_string(const StringType& string_value)
        {
            auto fixed_str = basic_fixed_string {};

            fixed_str.copy_from(string_value);

            return fixed_str;
        }

        constexpr basic_fixed_string() = default;

        template <auto raw_string_length>
        constexpr basic_fixed_string(const CharType (&raw_string)[raw_string_length])
        {
            static_assert(raw_string_length <= string_buffer_size);

            copy_from_raw(raw_string, raw_string_length);
        }

        // NOTE: Uses `std::strlen` in non-consteval situations, full fixed length otherwise.
        constexpr basic_fixed_string(const_pointer_type raw_string)
        {
            if (std::is_constant_evaluated()) // consteval
            {
                copy_from_raw(raw_string, string_length);
            }
            else
            {
                copy_from_raw(raw_string, std::strlen(raw_string));
            }
        }

        constexpr basic_fixed_string(const_pointer_type raw_string, size_type raw_string_length)
        {
            copy_from_raw(raw_string, raw_string_length);
        }
    
        constexpr basic_fixed_string& copy_from_raw(const_pointer_type raw_string, size_type raw_string_length)
        {
            const auto raw_string_begin = raw_string;
            const auto raw_string_end = (raw_string_begin + limit_length(raw_string_length));

            std::copy(raw_string_begin, raw_string_end, begin());

            return set_null_terminator();
        }

        template <typename StringType>
        constexpr basic_fixed_string& copy_from(const StringType& string_value)
        {
            const auto copy_length = limit_length(string_value.size());

            std::copy(string_value.begin(), string_value.end(), begin());

            return set_null_terminator();
        }

        constexpr pointer_type data()
        {
#if (GLARE_UTIL_FIXED_STRING_USE_STD_ARRAY)
            return data_buffer.data();
#else
            return data_buffer;
#endif // GLARE_UTIL_FIXED_STRING_USE_STD_ARRAY
        }

        constexpr const_pointer_type data() const
        {
#if (GLARE_UTIL_FIXED_STRING_USE_STD_ARRAY)
            return data_buffer.data();
#else
            return data_buffer;
#endif // GLARE_UTIL_FIXED_STRING_USE_STD_ARRAY
        }

        constexpr size_type length() const
        {
            return string_length;
        }

        constexpr size_type size() const
        {
            return length();
        }

        constexpr size_type capacity() const
        {
            return string_buffer_size;
        }

        constexpr bool empty() const
        {
            return static_cast<bool>(length());
        }

        constexpr const_iterator cbegin() const
        {
            return data();
        }

        constexpr const_iterator cend() const
        {
            return (data() + size());
        }

        constexpr iterator begin()
        {
            return data();
        }

        constexpr const_iterator begin() const
        {
            return cbegin();
        }

        constexpr const_iterator end() const
        {
            return cend();
        }

        constexpr iterator end()
        {
            return (data() + size());
        }

        constexpr const_pointer_type c_str() const
        {
            return data();
        }
    
        constexpr basic_fixed_string& set_null_terminator()
        {
            data_buffer[null_terminator_index] = null_terminator;

            return *this;
        }

        constexpr size_type limit_length(size_type intended_length) const
        {
            return std::min(intended_length, length());
        }
    
        constexpr operator const_pointer_type() const
        {
            return data();
        }
    
        constexpr explicit operator bool() const
        {
            return (!empty());
        }

        template <typename StringType>
        constexpr StringType string() const
        {
            return StringType { begin(), end() };
        }

        template <typename StringViewType>
        constexpr StringViewType string_view() const
        {
            return StringViewType { data(), size() };
        }

        friend constexpr auto operator<=>(const basic_fixed_string&, const basic_fixed_string&) = default;
    
        template <typename StringType>
        constexpr bool operator==(const StringType& str) const
        {
            return std::equal(begin(), end(), str.begin());
        }

        template <typename StringType>
        constexpr bool operator!=(const StringType& str) const
        {
            return (!operator==(str));
        }

        template <auto raw_string_length>
        constexpr bool operator==(const CharType (&raw_string)[raw_string_length]) const
        {
            return std::equal(begin(), (begin() + limit_length(raw_string_length)), raw_string);
        }

        template <auto raw_string_length>
        constexpr bool operator!=(const CharType (&raw_string)[raw_string_length]) const
        {
            return (!operator==(raw_string));
        }

        // NOTE: Uses `std::strlen` in non-consteval situations, full fixed length otherwise.
        constexpr bool operator==(const_pointer_type raw_string) const
        {
            if (std::is_constant_evaluated()) // consteval
            {
                return std::equal(begin(), end(), raw_string);
            }
            else
            {
                return (std::strcmp(data(), raw_string) == 0);
            }
        }

        // NOTE: Uses `std::strlen` in non-consteval situations, full fixed length otherwise.
        constexpr bool operator!=(const_pointer_type raw_string) const
        {
            return (!operator==(raw_string));
        }

#if (GLARE_UTIL_FIXED_STRING_USE_STD_ARRAY)
        std::array<value_type, string_buffer_size> data_buffer = {};
#else
        value_type data_buffer[string_buffer_size] = {};
#endif // GLARE_UTIL_FIXED_STRING_USE_STD_ARRAY
    };

    // NOTE: We use a separate derived type here due to language limitations
    // with deduction guides on template aliases:
    template <auto Length>
    struct fixed_string : basic_fixed_string<char, Length>
    {
        using base = basic_fixed_string<char, Length>;
        using const_pointer_type = const char*;
        using size_type = std::size_t;

        template <typename StringType>
        static constexpr fixed_string from_string(const StringType& string_value)
        {
            auto fixed_str = fixed_string {};

            fixed_str.copy_from(string_value);

            return fixed_str;
        }

        constexpr fixed_string() = default;
    
        template <auto raw_string_length>
        constexpr fixed_string(const char (&raw_string)[raw_string_length]) : base(raw_string) {}

        constexpr fixed_string(const_pointer_type raw_string) : base(raw_string) {}

        constexpr fixed_string(const_pointer_type raw_string, size_type raw_string_length) : base(raw_string, raw_string_length) {}
    };

    // Deduction guides for string literals:
    template <typename CharType, std::size_t SizeWithTerminator>
    basic_fixed_string(const CharType (&)[SizeWithTerminator]) -> basic_fixed_string<CharType, (SizeWithTerminator - 1)>;

    template <std::size_t SizeWithTerminator>
    fixed_string(const char (&)[SizeWithTerminator]) -> fixed_string<(SizeWithTerminator - 1)>;

    //template <std::size_t SizeWithTerminator>
    //using fixed_string = basic_fixed_string<char, SizeWithTerminator>;

    //template <std::size_t SizeWithTerminator>
    //fixed_string(const char (&)[SizeWithTerminator]) -> fixed_string<(SizeWithTerminator - 1)>;
}