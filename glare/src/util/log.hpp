#pragma once

#include "api.hpp"
#include "format.hpp"
#include "magic_enum.hpp"

// Module format headers:
#include <engine/format.hpp>
#include <math/format.hpp>

#include <cstdint>
#include <memory>
//#include <type_traits>

#define SPDLOG_EOL ""

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
    using LogLevel = spdlog::level::level_enum;

	namespace log
	{
        class DebugDataInterface
        {
            public:
                virtual std::uint64_t get_log_frame_number() const = 0;
                virtual std::uint64_t get_log_time_ms() const = 0;
        };

        void register_debug_data(const DebugDataInterface& debug_data);
        void unregister_debug_data(const DebugDataInterface& debug_data);

        const DebugDataInterface* get_debug_data();

        Logger& get_console();
        Logger& get_error_logger();

        void init();

        namespace impl
        {
            template <typename ...Args>
            decltype(auto) print_debug_data_impl(auto& console, LogLevel log_level)
            {
                if (const auto debug_data = get_debug_data())
                {
                    fmt::print
                    (
                        "[F{}] [{}ms] ",

                        debug_data->get_log_frame_number(),
                        debug_data->get_log_time_ms()
                    );
                }

                return console;
            }

            template <typename ...Args>
            decltype(auto) print_impl(LogLevel log_level, fmt::format_string<Args...> fmt, Args &&...args)
            {
                auto& console = get_console();

                if (console)
                {
                    print_debug_data_impl(console, log_level);

                    console->log(log_level, fmt, std::forward<Args>(args)...);

                    fmt::print("\n");
                }

                return console;
            }

            template <typename T>
            decltype(auto) print_impl(LogLevel log_level, const T& msg)
            {
                auto& console = get_console();

                if (console)
                {
                    print_debug_data_impl(console, log_level);

                    console->log(log_level, msg);

                    fmt::print("\n");
                }

                return console;
            }
        }

        template <typename ...Args>
        decltype(auto) print(fmt::format_string<Args...> fmt, Args&&... args)
        {
            return impl::print_impl(spdlog::level::info, std::move(fmt), std::forward<Args>(args)...);
        }

        template <typename T>
        decltype(auto) print(const T& message)
        {
            return impl::print_impl(spdlog::level::info, message);
        }

        template <typename ...Args>
        decltype(auto) print_warn(fmt::format_string<Args...> fmt, Args&&... args)
        {
            return impl::print_impl(spdlog::level::warn, std::move(fmt), std::forward<Args>(args)...);
        }

        template <typename T>
        decltype(auto) print_warn(const T& message)
        {
            return impl::print_impl(spdlog::level::warn, message);
        }

        template <typename ...Args>
        decltype(auto) print_error(fmt::format_string<Args...> fmt, Args&&... args)
        {
            return impl::print_impl(spdlog::level::err, std::move(fmt), std::forward<Args>(args)...);
        }

        template <typename T>
        decltype(auto) print_error(const T& message)
        {
            return impl::print_impl(spdlog::level::err, message);
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