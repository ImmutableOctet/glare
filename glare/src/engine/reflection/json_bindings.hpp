#pragma once

#include "json_extensions.hpp"
#include "common_extensions.hpp"

#include <engine/meta/hash.hpp>
#include <engine/meta/traits.hpp>
#include <engine/meta/function.hpp>
#include <engine/meta/serial.hpp>

#include <util/json.hpp>
#include <util/type_traits.hpp>

#include <utility>
#include <type_traits>

namespace engine
{
    struct MetaParsingInstructions;
    struct MetaTypeDescriptorFlags;

    namespace impl
    {
        template <typename T>
        auto define_basic_to_json_bindings(auto type)
        {
            using namespace engine::literals;

            type = type
                .func
                <
                    static_cast<bool(*)(const T&, util::json&, bool)>
                    (&engine::impl::default_save_impl<T, util::json&, bool>)
                >("to_json"_hs)
            ;

            return type;
        }

        template <typename T>
        auto define_exact_to_json_bindings(auto type)
        {
            using namespace engine::literals;

            if constexpr (has_method_to_json_v<T, bool, util::json&, bool>)
            {
                type = type
                    .func
                    <
                        static_cast<bool(T::*)(util::json&, bool) const>
                        (&T::to_json),
                    
                        entt::as_is_t
                    >("to_json"_hs)
                ;
            }

            /*
            // TODO: Handle default implementations for partial bindings.

            if constexpr (has_method_to_json_v<T, bool, util::json&>)
            {
                type = type
                    .func
                    <
                        static_cast<bool(T::*)(util::json&) const>
                        (&T::to_json),
                    
                        entt::as_ref_t
                    >("to_json"_hs)
                ;
            }
            else if constexpr (!has_custom_to_json_impl)
            {
                type = type
                    .func
                    <
                        //static_cast<bool(*)(util::json&)>
                        (&engine::save<T, util::json&>),
                    
                        entt::as_ref_t
                    >("to_json"_hs)
                ;
            }

            if constexpr (has_method_to_json_v<T, util::json>)
            {
                type = type
                    .func
                    <
                        static_cast<util::json(T::*)() const>
                        (&T::to_json)
                    >("to_json"_hs)
                ;
            }
            else if constexpr (!has_custom_to_json_impl)
            {
                type = type
                    .func
                    <
                        //static_cast<util::json(*)()>
                        (&engine::save<T>)
                    >("to_json"_hs)
                ;
            }

            if constexpr (has_method_to_json_v<T, util::json, bool>)
            {
                type = type
                    .func
                    <
                        static_cast<util::json(T::*)(bool) const>
                        (&T::to_json)
                    >("to_json"_hs)
                ;
            }
            else if constexpr (!has_custom_to_json_impl)
            {
                type = type
                    .func
                    <
                        //static_cast<util::json(*)(bool)>
                        (&engine::save<T, bool>)
                    >("to_json"_hs)
                ;
            }
            */

            return type;
        }
    }

    template <typename T>
    auto define_to_json_bindings(auto type)
    {
        using namespace engine::literals;

        constexpr bool is_arithmetic = std::is_arithmetic_v<T>;

        if constexpr (is_arithmetic)
        {
            type = impl::define_basic_to_json_bindings<T>(type);
        }
        else
        {
            constexpr bool has_custom_to_json_impl =
            (
                (has_method_to_json_v<T, bool, util::json&, bool>)
                ||
                (has_method_to_json_v<T, bool, util::json&>)
                ||
                (has_method_to_json_v<T, util::json>)
                ||
                (has_method_to_json_v<T, util::json, bool>)
            );

            if constexpr (has_custom_to_json_impl)
            {
                type = impl::define_exact_to_json_bindings<T>(type);
            }
            else
            {
                type = impl::define_basic_to_json_bindings<T>(type);
            }
        }

        return type;
    }

	template <typename T, bool generate_constructors=true, bool generate_functions=true>
	auto define_from_json_bindings(auto type)
	{
        using namespace engine::literals;

		if constexpr (util::is_constructible_without_conversion_v<T, const util::json&>)
        {
            if constexpr (generate_constructors)
            {
                type = type.ctor<const util::json&>();
            }

            if constexpr (generate_functions)
            {
                // Disabled for now.
                //type = type.func<&impl::wrap_constructor<T, const util::json&>>("from_json"_hs);
            }
        }
        else if constexpr (has_function_from_json_v<T, T, const util::json&>)
        {
            constexpr auto from_json_ptr = static_cast<T(*)(const util::json&)>(&T::from_json);

            if constexpr (generate_constructors)
            {
                type = type.ctor<from_json_ptr>();
            }

            if constexpr (generate_functions)
            {
                type = type.func<from_json_ptr>("from_json"_hs);
            }
        }
        else if constexpr (std::is_default_constructible_v<T>)
        {
            constexpr auto from_json_ptr = static_cast<T(*)(const util::json&)>(&impl::from_json<T>);

            if constexpr (generate_constructors)
            {
                type = type.ctor<from_json_ptr>();
            }

            if constexpr (generate_functions)
            {
                type = type.func<from_json_ptr>("from_json"_hs);
            }
        }

        if constexpr (std::is_constructible_v<T, const util::json&, const MetaParsingInstructions&>)
        {
            if constexpr (generate_constructors)
            {
                type = type.ctor<const util::json&, const MetaParsingInstructions&>();
            }

            if constexpr (generate_functions)
            {
                // Disabled for now.
                //type = type.func<&impl::wrap_constructor<T, const util::json&, const MetaParsingInstructions&>>("from_json"_hs);
            }
        }
        else if constexpr (has_function_from_json_v<T, T, const util::json&, const MetaParsingInstructions&>)
        {
            constexpr auto from_json_ptr = static_cast<T(*)(const util::json&, const MetaParsingInstructions&)>(&T::from_json);

            if constexpr (generate_constructors)
            {
                type = type.ctor<from_json_ptr>();
            }

            if constexpr (generate_functions)
            {
                type = type.func<from_json_ptr>("from_json"_hs);
            }
        }
        else if constexpr (std::is_default_constructible_v<T>)
        {
            constexpr auto from_json_ptr = static_cast<T(*)(const util::json&, const MetaParsingInstructions&)>(&impl::from_json_with_instructions<T>); // from_json<T>

            if constexpr (generate_constructors)
            {
                type = type.ctor<from_json_ptr>();
            }

            if constexpr (generate_functions)
            {
                type = type.func<from_json_ptr>("from_json"_hs);
            }
        }

        if constexpr (std::is_constructible_v<T, const util::json&, const MetaParsingInstructions&, const MetaTypeDescriptorFlags&>)
        {
            if constexpr (generate_constructors)
            {
                type = type.ctor<const util::json&, const MetaParsingInstructions&, const MetaTypeDescriptorFlags&>();
            }

            if constexpr (generate_functions)
            {
                // Disabled for now.
                //type = type.func<&impl::wrap_constructor<T, const util::json&, const MetaParsingInstructions&, const MetaTypeDescriptorFlags&>>("from_json"_hs);
            }
        }
        else if constexpr (has_function_from_json_v<T, T, const util::json&, const MetaParsingInstructions&, const MetaParsingInstructions&, const MetaTypeDescriptorFlags&>)
        {
            constexpr auto from_json_ptr = static_cast<T(*)(const util::json&, const MetaParsingInstructions&, const MetaTypeDescriptorFlags&)>(&T::from_json);

            if constexpr (generate_constructors)
            {
                type = type.ctor<from_json_ptr>();
            }

            if constexpr (generate_functions)
            {
                type = type.func<from_json_ptr>("from_json"_hs);
            }
        }
        else if constexpr (std::is_default_constructible_v<T>)
        {
            constexpr auto from_json_ptr = static_cast<T(*)(const util::json&, const MetaParsingInstructions&, const MetaTypeDescriptorFlags&)>(&impl::from_json_with_instructions_and_descriptor_flags<T>); // impl::from_json<T>

            if constexpr (generate_constructors)
            {
                type = type.ctor<from_json_ptr>();
            }

            if constexpr (generate_functions)
            {
                type = type.func<from_json_ptr>("from_json"_hs);
            }
        }

        return type;
	}
}