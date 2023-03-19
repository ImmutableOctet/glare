#pragma once

namespace util
{
	// TODO: Look into 'std::variant' as an alternative to this.
	template <typename T>
	class defaultable_ref
	{
		private:
			T default_value = T();
		public:
			T* ptr; // = nullptr

			inline defaultable_ref() : defaultable_ref(default_value) {}
			inline defaultable_ref(T& value) : ptr(&value) {}

			inline bool is_default() const
			{
				return (ptr == (&default_value));
			}

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

			inline T& operator->() const
			{
				return ptr;
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
}