#pragma once

#include <debug.hpp>
#include <math/math.hpp>

#include "types.hpp"
#include "resource.hpp"

#include "context.hpp"

namespace graphics
{
	// Forward declarations:
	class Context;
	class ContextState;
	class Shader;
	class Uniform;

	class ShaderSource
	{
		public:
			static ShaderSource Load(const std::string& vertex_path, const std::string& fragment_path);

			std::string vertex;
			std::string fragment;
	};

	class Shader : public Resource
	{
		protected:
			UniformMap uniforms;
		public:
			friend Context;
			friend ContextState;
			friend Uniform;

			using Source = ShaderSource;

			friend void swap(Shader& x, Shader& y)
			{
				swap(static_cast<Resource&>(x), static_cast<Resource&>(y));
				swap(x.uniforms, y.uniforms);
			}

			Shader(pass_ref<Context> ctx, const std::string& vertex_path, const std::string& fragment_path);

			inline Shader() : Shader({}, {}) {}
			inline Shader(Shader&& shader) noexcept : Shader() { swap(*this, shader); }

			Shader(const Shader&) = delete;

			~Shader();

			inline Shader& operator=(Shader shader)
			{
				swap(*this, shader);

				return *this;
			}

			//Uniform operator[](memory::raw_string str);
			Uniform operator[](const std::string& str);

			inline const UniformMap& get_uniforms() const
			{
				return uniforms;
			}
		protected:
			Shader(weak_ref<Context> ctx, ContextHandle&& resource_handle);

			//void on_bind(Context& context) override;
	};

	class Uniform
	{
		private:
			Shader& shader;
			std::string name; // memory::raw_string // const std::string&
		public:
			inline Uniform(Shader& shader, const std::string& name)
				: shader(shader), name(name) {}

			template <typename T>
			inline Uniform& operator=(const T& value)
			{
				shader.uniforms[name] = value;
				shader.get_context()->set_uniform(shader, name.c_str(), value);

				return *this;
			}

			const UniformData& data() const
			{
				return shader.uniforms[name];
			}

			inline const UniformData& operator*() const { return data(); }
	};

	/*
	template <typename T>
	class Uniform
	{
		public:
			Uniform(std::string variable_name, const T& value={})
				: name(std::move(variable_name)), value(value) {}

			inline const std::string& get_name() const { return name; }
			inline const T& get_value() const { return value; }

			inline raw_string get_name_raw() const { return get_name().c_str(); }

			inline void set_value(const T& value) { this->value = value; }

			inline T& operator*() { return value; }
			inline operator const T&() const { return value; }
			inline Uniform& operator=(const T& value) { set_value(value); return (*this); }
		protected:
			std::string name; // raw_string
			T value;
	};
	*/
}