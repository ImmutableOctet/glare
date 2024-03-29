#pragma once

#include "api.hpp"
#include "format.hpp"
#include "magic_enum.hpp"

// Module format headers:
#include <engine/format.hpp>
#include <math/format.hpp>

#include <memory>
//#include <type_traits>

#include <spdlog/spdlog.h>

/*
namespace spdlog
{
    class logger;
}
*/

namespace util
{
	using Logger = std::shared_ptr<spdlog::logger>;

	namespace log
	{
        Logger& get_console();
        Logger& get_error_logger();

        void init();

        template <typename ...Args>
        decltype(auto) print(fmt::format_string<Args...> fmt, Args &&...args)
        {
            auto& console = get_console();

            if (console)
            {
                console->info(fmt, std::forward<Args>(args)...);
            }

            return console;
        }

        template <typename T>
        decltype(auto) print(const T& msg)
        {
            auto& console = get_console();

            if (console)
            {
                console->info(msg);
            }

            return console;
        }

        template <typename ...Args>
        decltype(auto) print_warn(fmt::format_string<Args...> fmt, Args &&...args)
        {
            auto& console = get_console();

            if (console)
            {
                console->warn(fmt, std::forward<Args>(args)...);
            }

            return console;
        }

        template <typename T>
        decltype(auto) print_warn(const T& msg)
        {
            auto& console = get_console();

            if (console)
            {
                console->warn(msg);
            }

            return console;
        }

        template <typename ...Args>
        decltype(auto) print_error(fmt::format_string<Args...> fmt, Args &&...args)
        {
            auto& console = get_console();

            if (console)
            {
                console->error(fmt, std::forward<Args>(args)...);
            }

            return console;
        }

        template <typename T>
        decltype(auto) print_error(const T& msg)
        {
            auto& console = get_console();

            if (console)
            {
                console->error(msg);
            }

            return console;
        }

        template <typename EnumType>
        decltype(auto) print_enum_values()
        {
            magic_enum::enum_for_each<EnumType>
            (
                [](EnumType value)
                {
                    print
                    (
                        "{}: {}",

                        magic_enum::enum_name<EnumType>(value),
                        magic_enum::enum_integer<EnumType>(value) // static_cast<std::underlying_type<EnumType>>(value)
                    );
                }
            );

            return get_console();
        }
	}
}

using util::log::print;
using util::log::print_warn;
using util::log::print_error;
using util::log::print_enum_values;