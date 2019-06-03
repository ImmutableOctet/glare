#pragma once

#include <string>
#include <type_traits>

#include <debug.hpp>
#include <math/math.hpp>

#include "primitives.hpp"
#include "context.hpp"

namespace graphics
{
	// Declare the 'Shader' class for early use.
	class Shader;
	
	class ShaderSource
	{
		public:
			static ShaderSource Load(const std::string& vertex_path, const std::string& fragment_path);

			std::string vertex;
			std::string fragment;
	};

	template <typename T>
	class ShaderVar
	{
		public:
			using value_t = T;

			ShaderVar(raw_string name, weak_ref<Shader> shader, value_t value)
				: name(name), program(shader), value(value) {}
			
			inline raw_string get_name_raw() const { return name; }
			inline std::string get_name() const { return get_name_raw(); }

			inline value_t get_value() const { return value; }

			inline bool set_value(const value_t& value)
			{
				auto program_lock = program.lock();

				//ASSERT(program_lock);

				if (program_lock == nullptr)
				{
					return false;
				}

				auto& shader = *program_lock;
				auto& context = *(shader.get_context());

				if (context.set_uniform(shader, name, value))
				{
					this->value = value;

					return true;
				}

				return false;
			}

			// Operators:
			inline operator value_t() const { return get_value(); }
			inline ShaderVar& operator=(const value_t& value) { set_value(value); return *this; }
		protected:
			raw_string name; // std::string
			weak_ref<Shader> program;
			value_t value;
	};

	class Shader
	{
		public:
			friend Context;
			friend ContextState;

			using Source = ShaderSource;

			template <typename T>
			using Var = ShaderVar<T>;

			Shader(weak_ref<Context> ctx, const std::string& vertex_path, const std::string& fragment_path);
			inline Shader() : Shader({}, {}) {}

			~Shader();

			inline Context::Handle get_handle() const { return handle; }
			inline ref<Context> get_context() const { return context.lock(); }

			inline bool operator==(const Shader& rhs) const
			{
				if (rhs.get_handle() != get_handle())
				{
					return false;
				}

				if (rhs.get_context() != get_context())
				{
					return false;
				}

				return true;
			}
			inline bool operator!=(const Shader& rhs) const { return !operator==(rhs); }
		protected:
			Shader(weak_ref<Context> ctx, Context::Handle&& resource_handle);

			weak_ref<Context> context;
			Context::Handle handle = Context::NoHandle;
	};
}