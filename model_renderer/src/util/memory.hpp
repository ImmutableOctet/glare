#pragma once

#include <debug.hpp>

namespace memory
{
	using raw_string = const char*;

	template <typename T>
	using ref = std::shared_ptr<T>;

	template <typename T>
	using unique_ref = std::unique_ptr<T>;

	template <typename T>
	using weak_ref = std::weak_ptr<T>;

	template <typename T>
	using pass_ref = const ref<T>&;

	// TODO: Swap with 'std::span'.
	template <typename T>
	struct array_view
	{
		const T* ptr = nullptr;
		std::size_t length = 0; // int;

		array_view(const T* data=nullptr, std::size_t length=0) : ptr(data), length(length) {}

		template <std::size_t N>
		array_view(const std::array<T, N>& arr) : array_view(arr.data(), arr.size()) {}

		inline std::size_t size() const { return length; }
		inline const T* data() const { return ptr; }

		inline bool has_elements() const { return (length > 0); }
		inline bool has_data() const { return (data() != nullptr); }
		inline bool valid() const { return (has_elements() && has_data()); }

		inline operator bool() const { return valid(); }
		//inline operator const T*() const { return data; }

		inline bool operator==(const T* rhs) const { return (data() == rhs); }
		inline bool operator==(const array_view<T>* rhs) const { return ((*this == rhs->data()) && this->size() == rhs->size()); }
	};

	using memory_view = array_view<void>;

	template <typename T, typename ...Args>
	ref<T> allocate(Args&&... args)
	{
		return (std::make_shared<T>(args...)); // std::forward<Args>(args)
	}

	template <typename T, typename ...Args>
	unique_ref<T> unique(Args&&... args)
	{
		return (std::make_unique<T>(args...)); // std::forward<Args>(args)
	}
}