#pragma once

#include <string>
#include <unordered_map>
#include <type_traits>

#include <debug.hpp>

#include "resource.hpp"

namespace graphics
{
	// Forward declarations:
	class ContextState;

	// Declare the 'Shader' class for early use.
	class Shader;
	
	class ShaderSource
	{
		public:
			static ShaderSource Load(const std::string& vertex_path, const std::string& fragment_path);

			std::string vertex;
			std::string fragment;
	};

	class Shader : public Resource
	{
		public:
			friend Context;
			friend ContextState;

			using Source = ShaderSource;

			inline friend void swap(Shader& x, Shader& y)
			{
				swap(static_cast<Resource&>(x), static_cast<Resource&>(y));
			}

			Shader(pass_ref<Context> ctx, const std::string& vertex_path, const std::string& fragment_path);

			inline Shader() : Shader({}, {}) {}
			inline Shader(Shader&& shader) : Shader() { swap(*this, shader); }

			Shader(const Shader&)=delete;

			~Shader();

			inline Shader& operator=(Shader shader)
			{
				swap(*this, shader);

				return *this;
			}
		protected:
			Shader(weak_ref<Context> ctx, ContextHandle&& resource_handle);

			//void on_bind(Context& context) override;
	};
}