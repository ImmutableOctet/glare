#pragma once

#include "format.hpp"

// Module format headers:
#include <engine/format.hpp>
#include <math/format.hpp>

#include <memory>

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
	}
}

using util::log::print;
using util::log::print_warn;