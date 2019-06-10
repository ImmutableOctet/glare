/*
	OpenGL graphics context implementation.
*/

#include <app/window.hpp>
#include <util/lib.hpp>

#include <graphics/context_state.hpp>
#include <graphics/context.hpp>

#include <graphics/vertex.hpp>
#include <graphics/canvas.hpp>
#include <graphics/shader.hpp>

#include <graphics/native/opengl.hpp>

#include <sdl2/SDL_video.h>

namespace graphics
{
	// Implementation detail:
	static gl_uniform_location get_uniform(Shader& shader, raw_string name)
	{
		return glGetUniformLocation(shader.get_handle(), name);
	}

	// Returns the native OpenGL equivalent of 'VertexElementType'.
	static GLenum gl_element_type(VertexElementType type)
	{
		switch (type)
		{
			case VertexElementType::Byte:    return GL_BYTE;
			case VertexElementType::UByte:   return GL_UNSIGNED_BYTE;
			case VertexElementType::Short:   return GL_SHORT;
			case VertexElementType::UShort:  return GL_UNSIGNED_SHORT;
			case VertexElementType::Int:     return GL_INT;
			case VertexElementType::UInt:    return GL_UNSIGNED_INT;
			case VertexElementType::Half:    return GL_HALF_FLOAT;
			case VertexElementType::Float:   return GL_FLOAT;
			case VertexElementType::Double:  return GL_DOUBLE;
		}

		return GL_NONE;
	}

	// OpenGL specific functionality:
	static Context::Handle gl_compile_shader(const std::string& source, GLenum type)
	{
		const GLchar* source_raw = reinterpret_cast<const GLchar*>(source.c_str());

		// Allocate a native OpenGL shader source container.
		auto obj = glCreateShader(type);

		// Upload our shader source code.
		glShaderSource(obj, 1, &source_raw, nullptr); // source.length;

		// Compile the source code associated with our container.
		glCompileShader(obj);

		return obj;
	}

	// Context API:
	Context::Context(app::Window& wnd, Backend gfx)
		: graphics_backend(gfx), state(memory::unique<Context::State>())
	{
		ASSERT(get_backend() == Backend::OpenGL);
		ASSERT(util::lib::establish_gl());

		native_context = SDL_GL_CreateContext(wnd.get_handle());

		glewInit();

		// TODO: Move this to a separate member-function:
		// Initial configuration:
		glEnable(GL_DEPTH_TEST);
	}

	Context::~Context()
	{
		//ASSERT(get_backend() == Backend::OpenGL);

		SDL_GL_DeleteContext(native_context);
	}

	void Context::flip(app::Window& wnd)
	{
		SDL_GL_SwapWindow(wnd.get_handle());
	}

	void Context::clear(float red, float green, float blue, float alpha)
	{
		// TODO: Graphics Abstraction.
		glClearColor(red, green, blue, alpha);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	Shader& Context::bind(Shader& shader)
	{
		// Retrieve the currently bound shader.
		auto& prev_shader = state->shader;

		// Bind the shader to the current graphical context.
		glUseProgram(shader.get_handle());

		// Assign the newly bound shader.
		state->shader = shader;

		// Return the previously bound shader.
		return prev_shader;
	}

	// Mesh related:
	MeshComposition Context::generate_mesh(memory::memory_view vertices, std::size_t vertex_size, memory::array_view<VertexAttribute> attributes, memory::array_view<MeshIndex> indices) noexcept
	{
		ASSERT(vertices);
		ASSERT(vertex_size);
		ASSERT(attributes);

		GLMeshComposition mesh = {};

		const void* vertex_data_offset = 0;
		const bool indices_available = (indices);

		glGenVertexArrays(1, &mesh.VAO);

		// TODO: Handle shared VBOs for memory optimization:
		glGenBuffers(1, &mesh.VBO);

		if (indices_available)
		{
			glGenBuffers(1, &mesh.EBO);
		}

		// TODO: Potentially unify this implementation with standard bind operations.
		glBindVertexArray(mesh.VAO);


		// Vertex Data:
		glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);

		// TODO: Handle Static vs. Dynamic mesh data.
		glBufferData(GL_ARRAY_BUFFER, (vertices.size() * vertex_size), vertices.data(), GL_STATIC_DRAW);

		const auto* fields = attributes.data();

		// TODO: Look into unsigned sizes.
		for (std::size_t i = 0; i < (attributes.size()); i++) // static_cast<int>(...)
		{
			const auto& attribute = fields[i];

			const auto type = gl_element_type(attribute.type);
			const auto element_count = static_cast<GLint>(attribute.num_elements);
			const auto stride = static_cast<GLsizei>(vertex_size);
			const void* offset = (reinterpret_cast<const std::int8_t*>(vertex_data_offset) + attribute.offset);

			const auto index = static_cast<GLuint>(i);

			// TODO: Look into normalization flag for OpenGL driver.
			glVertexAttribPointer(index, element_count, type, GL_FALSE, stride, offset);
			glEnableVertexAttribArray(index);
		}

		// Index Data:
		if (indices_available)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);

			// TODO: Handle Static vs. Dynamic mesh data. (Less of a problem for indices)
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, (indices.size() * sizeof(MeshIndex)), indices.data(), GL_STATIC_DRAW);
		}

		return { mesh };
	}

	// Shader related:
	Context::Handle Context::build_shader(const ShaderSource& source)
	{
		auto vertex   = gl_compile_shader(source.vertex, GL_VERTEX_SHADER);
		auto fragment = gl_compile_shader(source.fragment, GL_FRAGMENT_SHADER);

		// Allocate a shader-program object.
		auto program = glCreateProgram();

		// Attach our compiled shader objects.
		glAttachShader(program, vertex);
		glAttachShader(program, fragment);

		// Link together the program.
		glLinkProgram(program);

		glDeleteShader(vertex);
		glDeleteShader(fragment);

		return program;
	}

	void Context::release_mesh(MeshComposition&& mesh)
	{
		auto& native_rep = mesh.gl;

		glDeleteVertexArrays(1, &native_rep.VAO);

		// Could be simplified to one call, but isn't considered not safe:
		glDeleteBuffers(1, &native_rep.EBO);
		glDeleteBuffers(1, &native_rep.VBO);
	}

	void Context::release_shader(Context::Handle&& handle)
	{
		glDeleteProgram(handle);
	}

	bool Context::set_uniform(Shader& shader, raw_string name, int value)
	{
		glUniform1i(get_uniform(shader, name), value);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, bool value)
	{
		return set_uniform(shader, name, static_cast<int>(value));
	}

	bool Context::set_uniform(Shader& shader, raw_string name, float value)
	{
		glUniform1f(get_uniform(shader, name), value);

		return true;
	}
	
	bool Context::set_uniform(Shader& shader, raw_string name, const math::Vector2D& value)
	{
		glUniform2fv(get_uniform(shader, name), 1, &value[0]);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, const math::Vector3D& value)
	{
		glUniform3fv(get_uniform(shader, name), 1, &value[0]);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, const math::Vector4D& value)
	{
		glUniform4fv(get_uniform(shader, name), 1, &value[0]);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, const math::Matrix2x2& value)
	{
		glUniformMatrix2fv(get_uniform(shader, name), 1, GL_FALSE, &value[0][0]);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, const math::Matrix3x3& value)
	{
		glUniformMatrix3fv(get_uniform(shader, name), 1, GL_FALSE, &value[0][0]);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, const math::Matrix4x4& value)
	{
		glUniformMatrix4fv(get_uniform(shader, name), 1, GL_FALSE, &value[0][0]);

		return true;
	}
}