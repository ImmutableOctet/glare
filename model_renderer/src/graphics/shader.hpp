#pragma once

#include <string>
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

			Shader(pass_ref<Context> ctx, const std::string& vertex_path, const std::string& fragment_path);
			inline Shader() : Shader({}, {}) {}

			Shader(const Shader&)=delete;

			~Shader();
		protected:
			Shader(weak_ref<Context> ctx, ContextHandle&& resource_handle);
	};

	template <typename T>
	class ShaderVar
	{
		public:
			using value_t = T;

			ShaderVar(raw_string name="", weak_ref<Shader> shader={}, value_t value={})
				: name(name), program(shader), value(value) {}
			
			inline raw_string get_name_raw() const { return name; }
			inline std::string get_name() const { return get_name_raw(); }

			inline value_t get_value() const { return value; }

			inline bool upload(Shader& shader, value_t value)
			{
				auto& context = *(shader.get_context());

				if (context.set_uniform(shader, name, value))
				{
					this->value = value;

					return true;
				}

				return false;
			}

			inline bool upload(const value_t& value)
			{
				auto program_lock = program.lock();

				//ASSERT(program_lock);

				if (program_lock == nullptr)
				{
					return false;
				}

				auto& shader = *program_lock;

				return upload(shader, value);
			}

			inline void upload() { upload(value); } // reupload() { ... }

			// Operators:
			inline operator value_t() const { return get_value(); }
			inline ShaderVar& operator=(const value_t& value) { upload(value); return *this; }
		protected:
			raw_string name; // std::string
			weak_ref<Shader> program;
			value_t value;
	};
}