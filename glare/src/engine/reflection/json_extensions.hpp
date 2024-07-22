#pragma once

#include <engine/meta/serial.hpp>

namespace engine::impl
{
	template <typename T>
	T from_json(const util::json& data)
    {
        return engine::load<T>(data);
    }

    // NOTE: Named differently from `from_json` due to ICE on latest MSVC compiler.
    template <typename T>
	T from_json_with_instructions
	(
		const util::json& data,
        const MetaParsingInstructions& parse_instructions
	)
    {
        return engine::load<T>(data, parse_instructions);
    }

    // NOTE: Named differently from `from_json` due to ICE on latest MSVC compiler.
    template <typename T>
	T from_json_with_instructions_and_descriptor_flags
	(
		const util::json& data,
		const MetaParsingInstructions& parse_instructions,
		const MetaTypeDescriptorFlags& descriptor_flags
	)
    {
        return engine::load<T>(data, parse_instructions, descriptor_flags);
    }
}