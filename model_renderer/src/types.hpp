#pragma once

#include <memory>
#include <cstdint>
//#include <cfloat>

//#include <half/half.hpp>

#include "util/memory.hpp"
#include "util/shorthand.hpp"

using raw_string = memory::raw_string;

template <typename T>
using ref = memory::ref<T>;

template <typename T>
using unique_ref = memory::unique_ref<T>;

template <typename T>
using weak_ref = memory::weak_ref<T>;

template <typename T>
using pass_ref = memory::pass_ref<T>;

// TODO: Look into 'std::variant' as an alternative to this.
template <typename T>
class defaultable_ref
{
	private:
		T default_value = T();
	public:
		T* ptr = &default_value;
		
		inline operator T&() const
		{
			return *ptr;
		}

		inline T* operator&() const
		{
			return ptr;
		}

		inline T& operator*() const
		{
			return *ptr;
		}

		inline T& operator=(T& new_ref)
		{
			ptr = &new_ref;

			return *ptr;
		}

		inline void make_default()
		{
			ptr = &default_value;
		}
};