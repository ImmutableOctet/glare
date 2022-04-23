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
		// General debugging information.
		Logger console;
		Logger err_logger;

		/*
		Logger get_console()
		{
			return spdlog::get("console");
		}

		Logger get_error_logger()
		{
			return spdlog::get("stderr");
		}
		*/

		void init()
		{
			if (!console)
			{
				console = spdlog::stdout_color_mt("console");
			}

			if (!err_logger)
			{
				err_logger = spdlog::stderr_color_mt("stderr");
			}
		}
	}

	namespace io
	{
		std::string load_string(const std::string& path)
		{
			if (path.empty())
			{
				return {};
			}

			std::ifstream file;
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

			}
			*/

			return buffer.str();
		}
	}
}