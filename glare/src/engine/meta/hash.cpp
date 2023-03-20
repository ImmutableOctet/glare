#include "hash.hpp"

namespace engine
{
	std::unordered_map<StringHash, std::variant<std::string, std::string_view>> known_string_hashes;

	std::string_view get_known_string_from_hash(StringHash hash_value)
	{
		auto it = known_string_hashes.find(hash_value);

		if (it == known_string_hashes.end())
		{
			return {};
		}

		const auto& entry = it->second;

		auto output = std::string_view {};

		util::visit
		(
			entry,

			[&output](const std::string& str)
			{
				output = str;
			},

			[&output](const std::string_view& str)
			{
				output = str;
			}
		);

		return output;
	}
}