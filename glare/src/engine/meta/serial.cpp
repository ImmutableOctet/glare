#include "serial.hpp"
#include "meta.hpp"
#include "meta_type_descriptor.hpp"

#include <engine/types.hpp>

#include <engine/entity/entity_thread_target.hpp>

#include <util/string.hpp>
#include <util/parse.hpp>

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
		static entt::meta_any resolve_array(const util::json& value, const MetaAnyParseInstructions& instructions={})
		{
			ArrayType output;

			output.reserve(value.size());

			for (const auto& element : value)
			{
				output.emplace_back(resolve_meta_any(element, instructions));
			}

			return { std::move(output) }; // output;
		}
	};

	entt::meta_any meta_any_from_string(const util::json& value, const MetaAnyParseInstructions& instructions)
	{
		auto string_value = value.get<engine::impl::StringType>();

		return meta_any_from_string(std::string_view { string_value }, instructions);
	}

	static entt::meta_any meta_any_from_string_resolve_impl(std::string_view value, const MetaAnyParseInstructions& instructions)
	{
		if (auto as_integer = util::from_string<std::int32_t>(value)) // std::int64_t
		{
			return as_integer;
		}
		
		if (auto as_float = util::from_string<float>(value)) // double
		{
			return as_float;
		}

		entt::meta_any output;
		entt::meta_type type;

		auto has_scope_resolution = util::split
		(
			value, "::",

			[&value, &output, &type](std::string_view symbol, bool is_last_symbol)
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
				/*
				if (is_last_symbol)
				{
					print_warn("Unable to fully resolve symbol-string: \"{}\"", value);
				}
				*/

				return true;
			}
		);

		if (output && has_scope_resolution)
		{
			return output;
		}

		if (instructions.resolve_component_member_references)
		{
			if (auto result = indirect_meta_data_member_from_string(value))
			{
				return *result;
			}
		}

		return {};
	}

	entt::meta_any meta_any_from_string(std::string_view value, const MetaAnyParseInstructions& instructions)
	{
		using namespace entt::literals;

		bool resolve_symbol = instructions.resolve_symbol;

		if (instructions.strip_quotes)
		{
			if (util::is_quoted(value))
			{
				value = util::unquote(value);

				resolve_symbol = false;
			}
		}

		if (resolve_symbol)
		{
			auto execute_string_command = [](const auto command_id, const auto& content) -> entt::meta_any
			{
				switch (command_id)
				{
					case "id"_hs:
					case "Id"_hs:
					case "ID"_hs:
					case "name"_hs:
					case "hash"_hs:
						return hash(content).value();

					case "thread"_hs:
						return EntityThreadTarget(content);
				}

				return {};
			};

			auto execute_any_command = [&execute_string_command](const auto command_id, const entt::meta_any& content) -> entt::meta_any
			{
				//switch (command_id)
				{
					//case "..."_hs:
					// break;

					//default:
					{
						if (const auto* as_str_view = content.try_cast<std::string_view>())
						{
							return execute_string_command(command_id, *as_str_view);
						}
						else if (const auto* as_str = content.try_cast<std::string>())
						{
							return execute_string_command(command_id, *as_str);
						}

						//break;
					}
				}

				return {};
			};

			// Check for `hash` shorthand.
			if (value.starts_with('$') || value.starts_with('#'))
			{
				// Remove symbol prefix.
				value = value.substr(1);

				if (auto result = execute_string_command("hash"_hs, value))
				{
					return result;
				}
			}
			else
			{
				auto
				[
					command_name, command_content,
					command_trailing_expr, command_content_is_string
				] = util::parse_single_argument_command(value, true); // false

				if (!command_name.empty() && !command_content.empty())
				{
					const auto command_id = hash(command_name).value();

					if (!command_content_is_string)
					{
						if (auto resolved = meta_any_from_string_resolve_impl(command_content, instructions))
						{
							if (auto result = execute_any_command(command_id, resolved))
							{
								return result;
							}

							// Fallback to resolved value.
							return resolved;
						}
					}

					if (auto result = execute_string_command(command_id, command_content))
					{
						return result;
					}

					// If nothing else, forward the inner content
					// of the command as the intended value.
					value = command_content;
				}
				else
				{
					if (auto resolved = meta_any_from_string_resolve_impl(value, instructions))
					{
						return resolved;
					}
				}
			}
		}

		if (instructions.fallback_to_string)
		{
			return { std::string(value) };
		}

		return {};
	}

	entt::meta_any resolve_meta_any(const util::json& value, MetaTypeID type_id, const MetaAnyParseInstructions& instructions)
	{
		return resolve_meta_any(value, entt::resolve(type_id), instructions);
	}

	entt::meta_any resolve_meta_any(const util::json& value, MetaType type, const MetaAnyParseInstructions& instructions)
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
				return MetaTypeDescriptor(type, value, instructions);
		}

		return resolve_meta_any(value, instructions);
	}

	entt::meta_any resolve_meta_any(const util::json& value, const MetaAnyParseInstructions& instructions)
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
				return impl::resolve_array(value, instructions);
			case jtype::string:
				return meta_any_from_string(value, instructions);
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

	std::optional<MetaDataMember> meta_data_member_from_string(const std::string& value) // std::string_view
	{
		const auto [type_name, data_member_name] = util::parse_data_member_reference(value);

		return process_meta_data_member(type_name, data_member_name);
	}

	std::optional<MetaDataMember> process_meta_data_member(std::string_view type_name, std::string_view data_member_name)
	{
		if (type_name.empty())
		{
			return std::nullopt;
		}

		const auto type_id = hash(type_name);
		const auto type = resolve(type_id);
		
		if (!type)
		{
			return std::nullopt;
		}

		const auto data_member_id = hash(data_member_name);
		const auto data_member = resolve_data_member_by_id(type, true, data_member_id); // type.data(data_member_id);

		if (!data_member)
		{
			return std::nullopt;
		}

		return MetaDataMember { type_id, data_member_id };
	}

	// Overload added due to limitations of `std::regex`. (Shouldn't matter for most cases, thanks to SSO)
	std::optional<MetaDataMember> meta_data_member_from_string(std::string_view value)
	{
		return meta_data_member_from_string(std::string(value));
	}

	std::optional<IndirectMetaDataMember> indirect_meta_data_member_from_string(const std::string& value, EntityTarget target)
	{
		const auto [type_name, data_member_name] = util::parse_data_member_reference(value);

		if (type_name.empty())
		{
			return std::nullopt;
		}

		if ((type_name == "self") || (type_name == "this"))
		{
			auto as_self = meta_data_member_from_string(value);

			if (!as_self)
			{
				return std::nullopt;
			}

			return IndirectMetaDataMember
			{
				std::move(target), // EntityTarget { EntityTarget::SelfTarget{} },
				std::move(*as_self)
			};
		}

		//auto data_member_reference = util::parse_data_member_reference(...);

		std::size_t target_parse_offset = 0;

		//EntityTarget::TargetType target = EntityTarget::SelfTarget{};

		if (auto target_parse_result = EntityTarget::parse_type(value))
		{
			target = { std::move(std::get<0>(*target_parse_result)) };
			target_parse_offset = std::get<1>(*target_parse_result);
		}

		auto member_reference_data = std::string_view { (value.data() + target_parse_offset), (value.size() - target_parse_offset) };

		if (auto as_target = meta_data_member_from_string(member_reference_data))
		{
			return IndirectMetaDataMember
			{
				std::move(target),
				std::move(*as_target)
			};
		}

		return std::nullopt;
	}

	// Overload added due to limitations of `std::regex`. (Shouldn't matter for most cases, thanks to SSO)
	std::optional<IndirectMetaDataMember> indirect_meta_data_member_from_string(std::string_view value, EntityTarget target)
	{
		return indirect_meta_data_member_from_string(std::string(value), std::move(target));
	}

	EntityTarget::TargetType parse_target_type(const util::json& target_data)
	{
		using namespace entt::literals;

		using Target = EntityTarget;

		switch (target_data.type())
		{
			case util::json::value_t::string:
			{
				auto raw_value = target_data.get<std::string>();
				
				if (auto result = EntityTarget::parse_type(raw_value))
				{
					return std::get<0>(*result);
				}
				
				break;
			}

			case util::json::value_t::number_integer:
			case util::json::value_t::number_unsigned:
				return Target::ExactEntityTarget { static_cast<Entity>(target_data.get<entt::id_type>()) };
		}

		return Target::SelfTarget {};
	}

	void read_command_parsing_context(CommandParsingContext& context, const util::json& data)
	{
		if (auto aliases = data.find("aliases"); aliases != data.end())
		{
			for (const auto& proxy : aliases->items())
			{
				const auto& raw_value = proxy.value();

				if (!raw_value.is_string())
				{
					continue;
				}

				const auto& alias = proxy.key();

				auto command_name = raw_value.get<std::string>();

				const auto command_id = hash(command_name);

				auto command_type = resolve(command_id);

				if (!command_type)
				{
					continue;
				}

				context.command_aliases[alias] = command_type.info().name(); // std::string(...);
			}
		}
	}
}