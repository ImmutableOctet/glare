#pragma once

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
        //Logger get_console();
        //Logger get_error_logger();

        extern Logger console;
        extern Logger err_logger;

        void init();

        template <typename ...Args>
        inline Logger print(fmt::format_string<Args...> fmt, Args &&...args)
        {
            console->info(fmt, std::forward<Args>(args)...);

            return console;
        }

        template <typename T>
        inline Logger print(const T& msg)
        {
            console->info(msg);

            return console;
        }

        template <typename ...Args>
        inline Logger print_warn(fmt::format_string<Args...> fmt, Args &&...args)
        {
            console->warn(fmt, std::forward<Args>(args)...);

            return console;
        }

        template <typename T>
        inline Logger print_warn(const T& msg)
        {
            console->warn(msg);

            return console;
        }

        template <typename EnumType>
        inline void print_enum_values()
        {
            magic_enum::enum_for_each<EnumType>([](EnumType value)
            {
                print
                (
                    "{}: {}",

                    magic_enum::enum_name<EnumType>(value),
                    magic_enum::enum_integer<EnumType>(value) // static_cast<std::underlying_type<EnumType>>(value)
                );
            });
        }
	}
}

using util::log::print;
using util::log::print_warn;
using util::log::print_enum_values;