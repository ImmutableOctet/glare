#include <catch2/catch_test_macros.hpp>

#include <engine/entity/serial.hpp>

#include <string_view>

TEST_CASE("engine::parse_instruction_header", "[engine:entity]")
{
	SECTION("Thread variable assignment")
	{
		auto [instruction, opt_thread_details] = engine::parse_instruction_header(std::string_view("some_thread.thread_local_var = A::B"));

		REQUIRE(instruction == "thread_local_var = A::B");
		REQUIRE(opt_thread_details.has_value());
	}

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

	SECTION("Explicit entity designation + explicit thread designation with wait instruction")
	{
		std::string_view instruction_raw = "child(player_model).thread(some_thread).wait(OnButtonPressed::button == Button::Pause)";
		auto [instruction, opt_thread_details] = engine::parse_instruction_header(instruction_raw);

		REQUIRE(instruction == "wait(OnButtonPressed::button == Button::Pause)");
		REQUIRE(opt_thread_details.has_value());
		REQUIRE(opt_thread_details->thread_id.has_value());
		REQUIRE(*opt_thread_details->thread_id == engine::hash("some_thread").value());
		REQUIRE(opt_thread_details->target_entity.is_child_target());
	}

	SECTION("Thread-local yield instruction with simplified condition syntax")
	{
		std::string_view instruction_raw = "yield(OnButtonPressed::button|Button::Jump)";
		auto [instruction, opt_thread_details] = engine::parse_instruction_header(instruction_raw);

		REQUIRE(instruction == instruction_raw);
		REQUIRE(!opt_thread_details.has_value());
	}

	SECTION("Unintended yield expression")
	{
		std::string_view unintended = "local x:= yield(OnButtonPressed::button|Button::Jump)";

		auto [instruction, opt_thread_details] = engine::parse_instruction_header(unintended);

		REQUIRE(instruction == unintended);
		REQUIRE(!opt_thread_details.has_value());
	}
}