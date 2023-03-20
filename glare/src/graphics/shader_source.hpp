#pragma once

#include <string>

namespace graphics
{
	class ShaderSource
	{
		public:
			using StringType = std::string;

			static StringType load_source_string(const std::string& path, std::string* version_out=nullptr);

			// Implementation found in "shader.cpp".
			static ShaderSource Load(const std::string& vertex_path, const std::string& fragment_path, const std::string& geometry_path={}); // StringType

			StringType vertex;
			StringType fragment;
			StringType geometry;

			StringType version;
	};
}