#include "cling.hpp"

#ifdef GLARE_SCRIPT_CLING

#include <cling/Interpreter/Interpreter.h>
#include <cling/Interpreter/Value.h>
#include <cling/Utils/Casting.h>

#include <iostream>

namespace glare
{
	void script_test()
	{
		int argc = 1;
		const char* argv[] = { "cling.exe" };

		const char* llvm_dir = nullptr; // "E:\\Downloads\\cling\\build\\Debug\\bin";

		auto interpreter = cling::Interpreter { argc, argv, llvm_dir };

		interpreter.process("int x = 100;");
		interpreter.process("int y = 300;");
		interpreter.process("int z = (x * y);");

		auto current_value = cling::Value {};

		interpreter.process("z;", &current_value);

		const auto z = current_value.getInt();

		std::cout << "Z: " << z << '\n';

		auto other_interpreter = cling::Interpreter { argc, argv, llvm_dir };

		other_interpreter.process("int w = 1;");
		other_interpreter.process("int a = 2;");

		other_interpreter.process("auto b = a + w;", &current_value);

		const auto b = current_value.getInt();

		auto third_interpreter = cling::Interpreter { other_interpreter, argc, argv };

		third_interpreter.process("auto c = b;", &current_value);

		const auto c = current_value.getInt();

		third_interpreter.process("auto d = ++b;", &current_value);

		const auto d = current_value.getInt();

		other_interpreter.process("b;", &current_value);

		const auto new_b = current_value.getInt();

		//other_interpreter.AddIncludePath("engine/script/include");
	}
}

#endif