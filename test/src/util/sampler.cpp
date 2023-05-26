#include <catch2/catch_test_macros.hpp>

#include <engine/meta/reflect_all.hpp>

#include <engine/meta/serial.hpp>

#include <math/types.hpp>

#include <util/sampler.hpp>
#include <util/json.hpp>

TEST_CASE("util::Sampler", "[engine]")
{
	engine::reflect_all();

	SECTION("Basic sampler from static array")
	{
		auto sampler = util::FloatSampler
		{
			std::array
			{
				std::pair { 0.0f, 1.0f },
				std::pair { 1.0f, 2.0f },
				std::pair { 2.0f, 3.0f }
			}
		};

		REQUIRE(!sampler.empty());
		REQUIRE(sampler.size() == 3);

		auto sum = 0.0f;

		for (const auto& entry : sampler)
		{
			sum += (std::get<0>(entry) * std::get<1>(entry));
		}

		REQUIRE
		(
			sum
			>=
			(
				(0.0f * 1.0f)
				+
				(1.0f * 2.0f)
				+
				(2.0f * 3.0f)
			)
		);
	}

	SECTION("Basic value sampler")
	{
		using Sampler = util::FloatSampler;
		using Entry   = Sampler::Entry;

		auto sampler = util::FloatSampler // util::SamplerImpl<float, float, std::vector<std::pair<float, float>>>
		{
			std::array
			{
				std::pair { 0.0f, 0.0f },
				std::pair { 1.0f, 2.0f },
				std::pair { 4.0f, 3.0f },
				std::pair { 5.0f, 7.0f },
				std::pair { 8.0f, 10.0f },
				std::pair { 10.0f, 3.0f },
				std::pair { 12.0f, -1.0f },
				std::pair { 14.0f, 1.0f }
			}
		};

		REQUIRE(sampler.get_value(0.5f) >= 1.0f);
		REQUIRE(sampler.get_value(2.5f) >= 2.5f);
		REQUIRE(sampler.get_value(5.0f) >= 7.0f);
		REQUIRE(sampler.get_value(9.0f) <= 6.5f);
		REQUIRE(sampler.get_value(13.0f) <= 0.0f);
		REQUIRE(sampler.get_value(12.5f) <= -0.5f);
		REQUIRE(sampler.get_value(15.0f) >= 1.0f);

		REQUIRE(sampler.get_cumulative_value(1.0f) >= 2.0f);
		REQUIRE(sampler.get_cumulative_value(4.0f) >= 5.0f);
		REQUIRE(sampler.get_cumulative_value(4.5f) >= 10.0f);
		REQUIRE(sampler.get_cumulative_value(6.5f) >= 20.5f);
		REQUIRE(sampler.get_cumulative_value(8.0f) >= 22.0f);
		REQUIRE(sampler.get_cumulative_value(13.5f) >= 24.5f);
		REQUIRE(sampler.get_cumulative_value(15.0f) >= 25.0f);
	}

	SECTION("Basic sampler from JSON")
	{
		const auto data = util::json::parse
		(
			"[ \
				{\"x\": 0.0, \"y\": 0.0 },  \
				{\"x\": 1.0, \"y\": 2.0 },  \
				{\"x\": 4.0, \"y\": 3.0 },  \
				{\"x\": 5.0, \"y\": 7.0 },  \
				{\"x\": 8.0, \"y\": 10.0 }, \
				{\"x\": 10.0, \"y\": 3.0 }, \
				{\"x\": 12.0, \"y\": 1.0 }  \
			]"
		);

		auto instance = engine::load<util::Sampler1D>(data);

		REQUIRE(instance.size() == 7);
		
		REQUIRE(instance.get_value_by_index(0) >= 0.0f);
		REQUIRE(instance.get_value_by_index(1) >= 2.0f);
		REQUIRE(instance.get_value_by_index(2) >= 3.0f);
		REQUIRE(instance.get_value_by_index(3) >= 7.0f);
		REQUIRE(instance.get_value_by_index(4) >= 10.0f);
		REQUIRE(instance.get_value_by_index(5) >= 3.0f);
		REQUIRE(instance.get_value_by_index(6) >= 1.0f);

		REQUIRE(instance.get_key_by_index(0) >= 0.0f);
		REQUIRE(instance.get_key_by_index(1) >= 1.0f);
		REQUIRE(instance.get_key_by_index(2) >= 4.0f);
		REQUIRE(instance.get_key_by_index(3) >= 5.0f);
		REQUIRE(instance.get_key_by_index(4) >= 8.0f);
		REQUIRE(instance.get_key_by_index(5) >= 10.0f);
		REQUIRE(instance.get_key_by_index(6) >= 12.0f);

		REQUIRE(instance.get_value(4.5f) >= 5.0f);
	}
}