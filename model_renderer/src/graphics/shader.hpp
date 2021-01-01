#pragma once

#include <debug.hpp>
#include <math/math.hpp>

#include <utility>

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

			//template <typename string_t>
			//friend class Uniform;

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
			//Uniform<const std::string&> operator[](const std::string& str);
			Uniform operator[](const std::string& str); // std::string_view // Uniform<std::string_view>

			inline const UniformMap& get_uniforms() const
			{
				return uniforms;
			}
		protected:
			Shader(weak_ref<Context> ctx, ContextHandle&& resource_handle);

			//void on_bind(Context& context) override;
	};

	//template <typename string_t>
	class Uniform
	{
		public:
			using string_t = std::string; // std::string_view;
		private:
			Shader& shader;
			string_t name; // memory::raw_string // const std::string&
		public:
			inline Uniform(Shader& shader, const string_t& name) // const std::string&
				: shader(shader), name(name) {} // std::forward<string_t>(...)

			template <typename T>
			inline auto& operator=(const T& value) // T&&
			{
				shader.uniforms[name] = value; // std::string(name)
				shader.get_context()->set_uniform(shader, name, value); // name.c_str()

				return *this;
			}

			UniformData data() const
			{
				auto it = shader.uniforms.find(name); // std::string(name)

				if (it == shader.uniforms.end())
				{
					return 0;
				}

				return it->second; // return shader.uniforms[name];
			}

			//inline const UniformData* operator*() const { return data(); }
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