#include <util/io.hpp>

#include <glm/glm.hpp>

#include "context.hpp"

#include "shader.hpp"

namespace graphics
{
	// Shader:
	Shader::Shader(weak_ref<Context> ctx, ContextHandle&& handle)
		: Resource(ctx, std::move(handle)) {}

	Shader::Shader(pass_ref<Context> ctx, const std::string& vertex_path, const std::string& fragment_path)
		: Shader(ctx, ctx->build_shader(Source::Load(vertex_path, fragment_path))) {}

	Shader::~Shader()
	{
		get_context()->release_shader(std::move(handle));
	}

	Uniform Shader::operator[](const std::string& str)
	{
		return { *this, str };
	}

	// ShaderSource:
	ShaderSource ShaderSource::Load(const std::string& vertex_path, const std::string& fragment_path)
	{
		return { util::io::load_string(vertex_path), util::io::load_string(fragment_path) };
	}
}