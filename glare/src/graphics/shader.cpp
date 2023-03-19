#include "context.hpp"
#include "shader.hpp"
#include "shader_source.hpp"

#include <glm/glm.hpp>

namespace graphics
{
	// Shader:
	Shader::Shader(std::weak_ptr<Context> ctx, ContextHandle&& handle)
		: Resource(ctx, std::move(handle)) {}

	Shader::Shader(const std::shared_ptr<Context>& ctx, const std::string& vertex_path, const std::string& fragment_path, const std::string& geometry_path, std::optional<std::string_view> preprocessor)
		: Shader(ctx, Source::Load(vertex_path, fragment_path, geometry_path), preprocessor) {}

	Shader::Shader(const std::shared_ptr<Context>& ctx, const Source& source, std::optional<std::string_view> preprocessor)
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
}