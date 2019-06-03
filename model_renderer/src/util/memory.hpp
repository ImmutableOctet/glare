#pragma once

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