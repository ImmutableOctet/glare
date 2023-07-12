#pragma once

#include <engine/meta/hash.hpp>
#include <engine/meta/traits.hpp>
#include <engine/meta/serial.hpp>

#include <engine/meta/binary_format_config.hpp>

#include <util/binary/binary_input_stream.hpp>
#include <util/binary/binary_output_stream.hpp>

#include <string_view>

#include <type_traits>

namespace engine
{
    namespace impl
    {
	    template <typename T>
	    auto define_basic_to_binary_bindings(auto type)
	    {
		    using namespace engine::literals;

            type = type
                .func
                <
                    static_cast<bool(*)(const T&, util::BinaryOutputStream&, const BinaryFormatConfig&)>
                    (&engine::impl::default_save_binary_impl<T, util::BinaryOutputStream&, const BinaryFormatConfig&>)
                >("to_binary"_hs)
            ;

            return type;
	    }

        template <typename T>
        auto define_exact_to_binary_bindings(auto type)
        {
            using namespace engine::literals;

            if constexpr (has_method_to_binary_v<T, bool, util::BinaryOutputStream&, const BinaryFormatConfig&>)
            {
                type = type
                    .func
                    <
                        static_cast<bool(T::*)(util::BinaryOutputStream&, const BinaryFormatConfig&) const>
                        (&T::to_binary),
                    
                        entt::as_is_t
                    >("to_binary"_hs)
                ;

                // TODO: Handle default implementations for partial bindings.
            }
        }
    }

    template <typename T>
    auto define_to_binary_bindings(auto type)
    {
        using namespace engine::literals;

        constexpr bool is_arithmetic = std::is_arithmetic_v<T>;

        if constexpr (is_arithmetic)
        {
            type = impl::define_basic_to_binary_bindings<T>(type);
        }
        else
        {
            constexpr bool has_custom_to_binary_impl =
            (
                (has_method_to_binary_v<T, bool, util::BinaryOutputStream&, const BinaryFormatConfig&>)
                ||
                (has_method_to_binary_v<T, bool, util::BinaryOutputStream&>)
            );

            if constexpr (has_custom_to_binary_impl)
            {
                type = impl::define_exact_to_binary_bindings<T>(type);
            }
            else
            {
                type = impl::define_basic_to_binary_bindings<T>(type);
            }
        }

        return type;
    }

    template <typename T, bool generate_constructors=true, bool generate_functions=true>
	auto define_from_binary_bindings(auto type)
	{
        using namespace engine::literals;

        if constexpr (!std::is_same_v<std::decay_t<T>, std::string_view>)
        {
            /*
            // Disabled due to potential constructor conflicts.
		    if constexpr (util::is_constructible_without_conversion_v<T, util::BinaryInputStream&>)
            {
                if constexpr (generate_constructors)
                {
                    type = type.ctor<util::BinaryInputStream&>();
                }

                if constexpr (generate_functions)
                {
                    // Disabled for now.
                    //type = type.func<&impl::wrap_constructor<T, util::BinaryInputStream&>>("from_binary"_hs);
                }
            }
            else
            */
            if constexpr (has_function_from_binary_v<T, T, util::BinaryInputStream&>)
            {
                constexpr auto from_binary_ptr = static_cast<T(*)(util::BinaryInputStream&)>(&T::from_binary);

                if constexpr (generate_constructors)
                {
                    type = type.ctor<from_binary_ptr>();
                }

                if constexpr (generate_functions)
                {
                    type = type.func<from_binary_ptr>("from_binary"_hs);
                }
            }
            else
            if constexpr (std::is_default_constructible_v<T>)
            {
                constexpr auto from_binary_ptr = static_cast<T(*)(util::BinaryInputStream&)>(&impl::load_binary_any_format<T>);

                if constexpr (generate_constructors)
                {
                    type = type.ctor<from_binary_ptr>();
                }

                if constexpr (generate_functions)
                {
                    type = type.func<from_binary_ptr>("from_binary"_hs);
                }
            }

            /*
            // Disabled due to potential constructor conflicts.
            if constexpr (std::is_constructible_v<T, util::BinaryInputStream&, const BinaryFormatConfig&>)
            {
                if constexpr (generate_constructors)
                {
                    type = type.ctor<util::BinaryInputStream&, const BinaryFormatConfig&>();
                }

                if constexpr (generate_functions)
                {
                    // Disabled for now.
                    //type = type.func<&impl::wrap_constructor<T, util::BinaryInputStream&, const BinaryFormatConfig&>>("from_binary"_hs);
                }
            }
            else
            */
            if constexpr (has_function_from_binary_v<T, T, util::BinaryInputStream&, const BinaryFormatConfig&>)
            {
                constexpr auto from_binary_ptr = static_cast<T(*)(util::BinaryInputStream&, const BinaryFormatConfig&)>(&T::from_binary);

                if constexpr (generate_constructors)
                {
                    type = type.ctor<from_binary_ptr>();
                }

                if constexpr (generate_functions)
                {
                    type = type.func<from_binary_ptr>("from_binary"_hs);
                }
            }
            else if constexpr (std::is_default_constructible_v<T>)
            {
                constexpr auto from_binary_ptr = static_cast<T(*)(util::BinaryInputStream&, const BinaryFormatConfig&)>(&impl::load_binary<T>);

                if constexpr (generate_constructors)
                {
                    type = type.ctor<from_binary_ptr>();
                }

                if constexpr (generate_functions)
                {
                    type = type.func<from_binary_ptr>("from_binary"_hs);
                }
            }
        }

        return type;
	}
}