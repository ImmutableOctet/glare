#include "serial.hpp"
#include "meta.hpp"
#include "meta_type_descriptor.hpp"

#include <engine/types.hpp>
#include <util/string.hpp>

#include <vector>

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	namespace impl
	{
		// TODO: Determine if `util::small_vector` would work here.
		// See: https://github.com/skypjack/entt/blob/master/docs/md/meta.md#container-support
		//using ArrayType = util::small_vector<entt::meta_any, 8>;
		using ArrayType = std::vector<entt::meta_any>;

		using StringType = util::json::string_t; // std::string;
		
		// Resolves primitive types from an array input.
		static entt::meta_any resolve_array(const util::json& value)
		{
			ArrayType output;

			output.reserve(value.size());

			for (const auto& element : value)
			{
				output.emplace_back(resolve_meta_any(element));
			}

			return { std::move(output) }; // output;
		}
	};

	entt::meta_any resolve_meta_any(const util::json& value)
	{
		using jtype = util::json::value_t;

		switch (value.type())
		{
			case jtype::object:
			{
				// Completely opaque object detected.
				assert(false);

				// TODO: Maybe handle as array instead...?
				return {};
			}
			case jtype::array:
				return impl::resolve_array(value);
			case jtype::string:
				return meta_any_from_string(value);
			case jtype::boolean:
				return value.get<bool>();
			case jtype::number_integer:
				return value.get<std::int32_t>(); // TODO: Handle larger integers, etc.
			case jtype::number_unsigned:
				return value.get<std::uint32_t>(); // TODO: Handle larger integers, etc.
			case jtype::number_float:
				return value.get<float>(); // 32-bit floats are standard for this engine.
			case jtype::binary:
				return {}; // Currently unsupported.
		}

		return {};
	}

	entt::meta_any resolve_meta_any(const util::json& value, MetaType type_id)
	{
		using jtype = util::json::value_t;

		switch (value.type())
		{
			// Objects and arrays of a known type can
			// be immediately nested as a `MetaTypeDescriptor`:
			case jtype::array:
			case jtype::object:
				// NOTE: Nesting `MetaTypeDescriptor` objects within the any-chain
				// implies recursion during object construction later on.
				return MetaTypeDescriptor(type_id, value);
		}

		return resolve_meta_any(value);
	}

	entt::meta_any resolve_meta_any(const util::json& value, MetaTypeID type_id)
	{
		return resolve_meta_any(value, entt::resolve(type_id));
	}

	entt::meta_any meta_any_from_string(const util::json& value, bool resolve_symbol, bool strip_quotes)
	{
		auto string_value = value.get<engine::impl::StringType>();

		return meta_any_from_string(std::string_view{ string_value }, resolve_symbol, strip_quotes);
	}

	entt::meta_any meta_any_from_string(const util::json& value)
	{
		auto string_value = value.get<engine::impl::StringType>();

		return meta_any_from_string(std::string_view{ string_value });
	}

	entt::meta_any meta_any_from_string(std::string_view value)
	{
		if (util::is_quoted(value))
		{
			return meta_any_from_string(util::unquote(value), false, false);
		}

		return meta_any_from_string(value, true);
	}

	entt::meta_any meta_any_from_string(std::string_view value, bool resolve_symbol, bool strip_quotes)
	{
		if (strip_quotes)
		{
			value = util::unquote_safe(value);
		}

		if (resolve_symbol)
		{
			entt::meta_any output;
			entt::meta_type type;

			auto has_scope_resolution = util::split(value, "::", [&value, &output, &type](std::string_view symbol, bool is_last_symbol)
			{
				auto symbol_id = hash(symbol);

				auto as_type = resolve(symbol_id);

				if (as_type)
				{
					type = as_type;
				}
				else
				{
					if (!type)
					{
						return false;
					}

					if (auto as_data = type.data(symbol_id))
					{
						if (as_data.is_static())
						{
							if (auto result = as_data.get({}))
							{
								output = std::move(result);

								return false;
							}
						}
						else
						{
							type = as_data.type();
						}
					}
					/*
					else if (auto as_prop = type.prop(symbol_id))
					{
						output = as_prop.value();

						return false;
					}
					*/
					else if (auto str_fn = type.func(hash("string_to_value")))
					{
						if (auto result = str_fn.invoke({}, symbol))
						{
							output = std::move(result);

							return false;
						}
					}
				}

				// Last symbol:
				if (is_last_symbol)
				{
					print_warn("Unable to fully resolve symbol-string: \"{}\"", value);
				}

				return true;
			});

			if (output && has_scope_resolution)
			{
				return output;
			}

			// If we aren't able to resolve a symbol, fall back to the original string.
		}

		return { std::string(value) };
	}
}