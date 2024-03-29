#include <catch2/catch_test_macros.hpp>

#include <util/vector_queue.hpp>
#include <util/small_vector_queue.hpp>

#include <cstdint>

TEST_CASE("util::vector_queue", "[util]")
{
	using T = std::int32_t;

	auto data = util::vector_queue<T> {};

	// Run twice to ensure determinism.
	for (auto i = 1; i <= 2; i++)
	{
		data.emplace(100);
		data.emplace(200);
		data.emplace(300);
		data.emplace(424);

		auto sum = T {};

		while (!data.empty())
		{
			sum += data.front();

			data.pop();
		}

		REQUIRE(data.size() == static_cast<std::size_t>(0));
		REQUIRE(sum == 1024);
	}
}

TEST_CASE("util::small_vector_queue", "[util]")
{
	using T = std::int32_t;

	auto data = util::small_vector_queue<T, 6> {};

	// Run twice to ensure determinism.
	for (auto i = 1; i <= 2; i++)
	{
		data.emplace(16);
		data.emplace(32);
		data.emplace(64);
		data.emplace(128);
		data.emplace(256);
		data.emplace(512);

		auto sum = T {};

		while (!data.empty())
		{
			sum += data.front();

			data.pop();
		}

		REQUIRE(data.size() == static_cast<std::size_t>(0));
		REQUIRE(sum == 1008);
	}
}