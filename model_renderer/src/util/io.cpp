#include "io.hpp"

//#include <iostream>
#include <fstream>
#include <sstream>

namespace util
{
	namespace io
	{
		std::string load_string(const std::string& path)
		{
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