#include <catch2/catch_test_macros.hpp>

#include <math/conversion.hpp>

#include <cstdint>

TEST_CASE("Normalized device coordinates", "[math:conversion]")
{
	const auto device_size = math::vec2f { 1600.0f, 900.0f };

	const auto normalized_result = math::to_normalized_device_coordinates(device_size, { 1500.0f, 300.0f });

	REQUIRE(normalized_result.x >= 0.8f); // 0.875f
	REQUIRE(normalized_result.y <= -0.3f); // -0.3333f

	const auto device_result = math::from_normalized_device_coordinates(device_size, normalized_result);

	REQUIRE(static_cast<std::int32_t>(device_result.x + 0.5f) >= 1500);
	REQUIRE(static_cast<std::int32_t>(device_result.y + 0.5f) >= 300);
}