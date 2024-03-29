#include "io.hpp"
#include "log.hpp"

//#include <iostream>
#include <fstream>
#include <sstream>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace util
{
	namespace log
	{
		Logger console;
		Logger error_logger;

		Logger& get_console()
		{
			return console; // spdlog::get("console");
		}

		Logger& get_error_logger()
		{
			return error_logger; // return spdlog::get("stderr");
		}

		void init()
		{
			if (!console)
			{
				console = spdlog::stdout_color_mt("console");
			}

			if (!error_logger)
			{
				error_logger = spdlog::stderr_color_mt("stderr");
			}
		}
	}

	namespace impl
	{
		static std::string load_string_impl(const auto& path)
		{
			if (path.empty())
			{
				return {};
			}

			auto file = std::ifstream {};

			file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

			std::stringstream buffer;

			//try
			{
				file.open(path);

				buffer << file.rdbuf();

				file.close();
			}
			/*
			catch (std::ifstream::failure)
			{
				return {};
			}
			*/

			return buffer.str();
		}

		static bool save_string_impl(const std::string& value, const auto& path)
		{
			if (path.empty())
			{
				return false;
			}

			auto file = std::ofstream {};

			file.exceptions(std::ofstream::failbit | std::ofstream::badbit);

			//try
			{
				file.open(path);

				file << value;

				file.close();
			}
			/*
			catch (std::ofstream::failure)
			{
				return false;
			}
			*/

			return true;
		}
	}

	std::string load_string(const std::string& path)
	{
		return impl::load_string_impl(path);
	}

	std::string load_string(const std::filesystem::path& path)
	{
		return impl::load_string_impl(path);
	}

	void save_string(const std::string& value, const std::string& path)
	{
		impl::save_string_impl(value, path);
	}

	void save_string(const std::string& value, const std::filesystem::path& path)
	{
		impl::save_string_impl(value, path);
	}
}