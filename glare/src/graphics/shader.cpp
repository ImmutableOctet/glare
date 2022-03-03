#include <util/io.hpp>
#include <glm/glm.hpp>

#include "context.hpp"
#include "shader.hpp"

#include <regex>

// Debugging related:
#include <iostream>

namespace graphics
{
	// Shader:
	Shader::Shader(weak_ref<Context> ctx, ContextHandle&& handle)
		: Resource(ctx, std::move(handle)) {}

	Shader::Shader(pass_ref<Context> ctx, const std::string& vertex_path, const std::string& fragment_path, const std::string& geometry_path, std::optional<std::string_view> preprocessor)
		: Shader(ctx, Source::Load(vertex_path, fragment_path, geometry_path), preprocessor) {}

	Shader::Shader(pass_ref<Context> ctx, const Source& source, std::optional<std::string_view> preprocessor)
		: Shader(ctx, ctx->build_shader(source, preprocessor)) {}

	Shader::~Shader()
	{
		get_context()->release_shader(std::move(handle));
	}

	/*
	Uniform<const std::string&> Shader::operator[](const std::string& str)
	{
		return { *this, str };
	}
	*/

	Uniform Shader::operator[](const std::string& str) // std::string_view // Uniform<std::string_view>
	{
		return { *this, str };
	}

	ShaderSource::StringType ShaderSource::load_source_string(const std::string& path, std::string* version_out)
	{
		if (path.empty())
		{
			return {};
		}

		auto source_raw = util::io::load_string(path);

		if (source_raw.empty())
		{
			return {};
		}

		if (version_out)
		{
			auto ver_regex = std::regex("(\\#version[^\n]+\n+)");

			std::smatch match;

			auto result = std::regex_search(source_raw, match, ver_regex);

			if (result)
			{
				//std::cout << "match: [" << match[1] << ']' << '\n';
				//std::cout << "suffix: " << match.suffix().str() << '\n';

				const auto& version_in = match[1];

				// Extract the version string:
				if (!version_out->empty())
				{
					ASSERT(version_in == *version_out);
				}

				*version_out = version_in;

				// Retrieve the rest of the shader source.
				//source_raw = match.suffix().str();

				return match.suffix().str();
			}
		}

		/*
		if (check_for_version)
		{
			if (path.find("#version") == std::string::npos)
			{
				return "#version 330 core\n" + source_raw;
			}
		}
		*/

		return source_raw;
	}

	// ShaderSource:
	ShaderSource ShaderSource::Load(const std::string& vertex_path, const std::string& fragment_path, const std::string& geometry_path)
	{
		std::string version;

		auto vertex_src   = load_source_string(vertex_path,   &version);
		auto fragment_src = load_source_string(fragment_path, &version);
		auto geometry_src = load_source_string(geometry_path, &version);

		return { std::move(vertex_src), std::move(fragment_src), std::move(geometry_src), std::move(version) };
	}
}