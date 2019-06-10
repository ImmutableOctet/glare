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

template <typename T>
class defaultable_ref
{
	private:
		T default_value = T();
	public:
		T& ref = default_value;
		
		inline operator T&() const
		{
			return ref;
		}

		inline T* operator&() const
		{
			return &ref;
		}

		inline T& operator*() const
		{
			return *ref;
		}

		inline T& operator=(T& new_ref)
		{
			ref = new_ref;

			return ref;
		}

		inline void make_default()
		{
			ref = default_value;
		}
};