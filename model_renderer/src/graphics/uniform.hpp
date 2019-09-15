#pragma once

#include <utility>
#include <string>

#include "types.hpp"

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

		inline operator const T&() const { return value; }
		inline Uniform& operator=(const T& value) { set_value(value); return (*this); }
	protected:
		std::string name; // raw_string
		T value;
};