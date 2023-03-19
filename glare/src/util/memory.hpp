#pragma once

#include <memory>

namespace memory
{
	using raw_ptr    = void*;
	using raw_string = const char*;

	/*
		A type-erased `unique_ptr` type:
		Used to store uniquely-allocated objects without
		the need to track type information and lifetimes.

		Note: This type is only safe to use if the deleter function (provided during construction)
		has the necessary static type information available.

		For this reason, please use `make_unique_void` and similar factory
		routines in order to construct objects of this type.
	*/
	using opaque_unique_ptr = std::unique_ptr<void, void(*)(void*)>;

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

		inline explicit operator bool() const { return valid(); }
		//inline operator const T*() const { return data; }

		inline bool operator==(const T* rhs) const { return (data() == rhs); }
		inline bool operator==(const array_view<T>* rhs) const { return ((*this == rhs->data()) && this->size() == rhs->size()); }
	};

	using memory_view = array_view<void>;

	/*
		Allocates an object of type `T` as an opaque unique-pointer. For more details, please view `opaque_unique_ptr`.
		Ownership of this object is tied to the lifetime of the returned `opaque_unique_ptr` object.
		
		To further reference the allocated `T` object, use `reinterpret_cast` on the result of `opaque_unique_ptr::get()` -- Example:
		```
		auto x = make_opaque<T>(...);
		auto* x_as_t = reinterpret_cast<T>(x.get());

		x_as_t->some_member_function();
		```
	*/
	template <typename T, typename ...Args>
	opaque_unique_ptr make_opaque(Args&&... args)
	{
		return opaque_unique_ptr
		(
			new T(std::forward<Args>(args)...),
			[](void* ptr_no_type)
			{
				/*
				Since this is an embedded lambda that already knows statically that `ptr_no_type` is of type `T` (since we're making `T` right away),
				we can safely `reinterpret_cast` back to `T` in order to delete appropriately.
				
				`void_unique_ptr` stores a function-pointer to this deleter without needing to know what `T` is at runtime,
				but as stated previously, we can already safely convert back to `T` as part of our opaque implementation.
				*/
				auto* ptr = reinterpret_cast<T*>(ptr_no_type);

				//std::default_delete<T>(ptr)();
				delete ptr;
			}
		);
	}
}