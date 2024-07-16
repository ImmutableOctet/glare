#include <engine/platform/reflection.hpp>

#include <string>
#include <cstddef>

#include <windows.h>
//#include <libloaderapi.h>

// Debugging related:
#include <iostream>

namespace engine
{
	bool reflect_exported_functions(std::string_view module_name)
	{
		// Work in progress; disabled for now.
		return false;
		
		/*
		auto target_module = HMODULE {};

		if (module_name.empty())
		{
			target_module = GetModuleHandleA(nullptr);
		}
		else
		{
			const auto module_name_null_terminated = std::string { module_name };

			target_module = GetModuleHandleA(module_name_null_terminated.c_str());
		}

		if (!target_module)
		{
			return false;
		}

		const auto DOS_header = reinterpret_cast<const IMAGE_DOS_HEADER*>(target_module);

		if (DOS_header->e_magic != IMAGE_DOS_SIGNATURE)
		{
			return false;
		}

		const auto target_module_data = reinterpret_cast<const std::uint8_t*>(target_module);

		const auto nt_header = reinterpret_cast<const IMAGE_NT_HEADERS*>(target_module_data + DOS_header->e_lfanew);

		if (nt_header->Signature != IMAGE_NT_SIGNATURE)
		{
			return false;
		}

		if (!nt_header->OptionalHeader.NumberOfRvaAndSizes)
		{
			return false;
		}

		const auto& export_meta_data = nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

		const auto exports = reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(target_module_data + export_meta_data.VirtualAddress);

		if (!exports->AddressOfNames)
		{
			return false;
		}

		const auto name_offsets = reinterpret_cast<const std::int32_t*>(target_module_data + exports->AddressOfNames);

		const auto name_entry_count = exports->NumberOfNames;

		for (auto name_index = 0; name_index < name_entry_count; name_index++)
		{
			const auto export_name = reinterpret_cast<const char*>(target_module_data + name_offsets[name_index]);

			std::cout << export_name << '\n';
		}
		*/
	}
}