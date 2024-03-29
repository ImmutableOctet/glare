#pragma once

#include <concurrencpp/concurrencpp.h>

namespace util
{
	namespace concurrency = concurrencpp;

	template <typename T>
	using Result = concurrency::result<T>;

	template <typename T>
	using Task = Result<T>;

	using VoidTask = Task<void>; // concurrencpp::null_result;
	using VoidResult = VoidTask; // Result<void>;

	using ResultStatus = concurrencpp::result_status;

	template <typename T>
	using Generator = concurrencpp::generator<T>;
}