#include <catch2/catch_test_macros.hpp>

#include <engine/entity/serial.hpp>
#include <engine/meta/meta.hpp>

#include <string_view>

TEST_CASE("engine:entity", "[engine]")
{
	SECTION("parse_instruction_header")
	{
		{
			auto [instruction, opt_thread_details] = engine::parse_instruction_header(std::string_view("some_thread.resume()"));

			REQUIRE(instruction == "resume()");
			REQUIRE(opt_thread_details.has_value());
		}

		{
			std::string_view instruction_raw = "some_thread.yield(OnButtonPressed::button|Button::Jump)";
			//std::string_view instruction_raw = "thread(some_thread).yield(OnButtonPressed::button|Button::Jump)";
			auto [instruction, opt_thread_details] = engine::parse_instruction_header(instruction_raw);

			REQUIRE(instruction == "yield(OnButtonPressed::button|Button::Jump)");
			REQUIRE(opt_thread_details.has_value());
			REQUIRE(opt_thread_details->thread_id.has_value());
			REQUIRE(*opt_thread_details->thread_id == engine::hash("some_thread").value());
		}

		{
			std::string_view instruction_raw = "thread(some_thread).yield(OnButtonPressed::button|Button::Jump)";
			auto [instruction, opt_thread_details] = engine::parse_instruction_header(instruction_raw);

			REQUIRE(instruction == "yield(OnButtonPressed::button|Button::Jump)");
			REQUIRE(opt_thread_details.has_value());
			REQUIRE(opt_thread_details->thread_id.has_value());
			REQUIRE(*opt_thread_details->thread_id == engine::hash("some_thread").value());
		}

		{
			std::string_view instruction_raw = "yield(OnButtonPressed::button|Button::Jump)";
			auto [instruction, opt_thread_details] = engine::parse_instruction_header(instruction_raw);

			REQUIRE(instruction == instruction_raw);
			REQUIRE(!opt_thread_details.has_value());
		}
	}
}