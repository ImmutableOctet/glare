#include "hash.hpp"

namespace engine
{
	namespace impl
	{
		StringHashLookupTable& get_known_string_hashes()
		{
			// NOTE: Function-local static variable used to ensure safe initialization.
			// 
			// TODO: Add some kind of locking mechanism to avoid race conditions on access.
			// 
			// i.e. This being defined inside of a function ensures that construction is handled exactly once for all threads,
			// but accessing or assigning entries to the lookup table is not thread safe.
			static auto known_string_hashes = StringHashLookupTable {};

			return known_string_hashes;
		}
	}

	std::string_view get_known_string_from_hash(StringHash hash_value)
	{
		if (!hash_value)
		{
			return {};
		}

		auto& known_string_hashes = impl::get_known_string_hashes();

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