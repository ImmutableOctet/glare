#include <catch2/catch_test_macros.hpp>

#include <engine/entity/serial.hpp>
#include <engine/meta/meta.hpp>

#include <string_view>

TEST_CASE("engine::parse_instruction_header", "[engine:entity]")
{
	SECTION("Instruction + thread name as header")
	{
		auto [instruction, opt_thread_details] = engine::parse_instruction_header(std::string_view("some_thread.resume()"));

		REQUIRE(instruction == "resume()");
		REQUIRE(opt_thread_details.has_value());
	}

	SECTION("Thread-qualified yield statement with simplified condition syntax")
	{
		std::string_view instruction_raw = "some_thread.yield(OnButtonPressed::button|Button::Jump)";
		auto [instruction, opt_thread_details] = engine::parse_instruction_header(instruction_raw);

		REQUIRE(instruction == "yield(OnButtonPressed::button|Button::Jump)");
		REQUIRE(opt_thread_details.has_value());
		REQUIRE(opt_thread_details->thread_id.has_value());
		REQUIRE(*opt_thread_details->thread_id == engine::hash("some_thread").value());
	}

	SECTION("Explicit thread designation with yield instruction")
	{
		std::string_view instruction_raw = "thread(some_thread).yield(OnButtonPressed::button|Button::Jump)";
		auto [instruction, opt_thread_details] = engine::parse_instruction_header(instruction_raw);

		REQUIRE(instruction == "yield(OnButtonPressed::button|Button::Jump)");
		REQUIRE(opt_thread_details.has_value());
		REQUIRE(opt_thread_details->thread_id.has_value());
		REQUIRE(*opt_thread_details->thread_id == engine::hash("some_thread").value());
	}

	SECTION("Thread-local yield instruction with simplified condition syntax")
	{
		std::string_view instruction_raw = "yield(OnButtonPressed::button|Button::Jump)";
		auto [instruction, opt_thread_details] = engine::parse_instruction_header(instruction_raw);

		REQUIRE(instruction == instruction_raw);
		REQUIRE(!opt_thread_details.has_value());
	}
}