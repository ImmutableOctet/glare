#include "entity_target.hpp"

#include "components/instance_component.hpp"

#include <engine/meta/meta.hpp>
#include <engine/meta/serial.hpp>

#include <engine/components/relationship_component.hpp>
#include <engine/components/name_component.hpp>
#include <engine/components/player_component.hpp>

#include <util/string.hpp>
#include <util/parse.hpp>
#include <util/variant.hpp>

namespace engine
{
	// EntityTarget:
	static Entity find_child_impl(Registry& registry, Entity source, StringHash child_name, bool recursive=true)
	{
		if (!child_name)
		{
			return null;
		}

		const RelationshipComponent* relationship = registry.try_get<RelationshipComponent>(source);

		if (!relationship)
		{
			return null;
		}

		Entity target_entity = null;

		relationship->enumerate_children
		(
			registry,
									
			[&registry, child_name, &target_entity](Entity child, const RelationshipComponent& child_relationship, Entity next_child) -> bool
			{
				const auto* child_name_comp = registry.try_get<NameComponent>(child);

				if (!child_name_comp)
				{
					return true;
				}

				if (child_name_comp->hash() == child_name)
				{
					target_entity = child;

					return false;
				}
										
				return true;
			},

			recursive
		);

		return target_entity;
	}

	Entity EntityTarget::resolve(Registry& registry, Entity source) const
	{
		return get(registry, source);
	}

	Entity EntityTarget::get(Registry& registry, Entity source) const
	{
		return get_impl(registry, source);
	}

	Entity EntityTarget::get(Registry& registry, Entity source, const MetaEvaluationContext& context) const
	{
		return get_impl(registry, source, context);
	}

	template <typename ...Args>
	Entity EntityTarget::get_impl(Registry& registry, Entity source, Args&&... args) const
	{
		using namespace engine::literals;

		Entity target_entity = null;

		util::visit
		(
			type,

			[source, &target_entity](const SelfTarget&)
			{
				target_entity = source;
			},

			[&registry, source, &target_entity](const ParentTarget&)
			{
				if (const RelationshipComponent* relationship = registry.try_get<RelationshipComponent>(source))
				{
					target_entity = relationship->get_parent();
				}
			},

			[&target_entity](const ExactEntityTarget& exact_entity)
			{
				target_entity = exact_entity.entity;
			},

			[&registry, source, &target_entity](const EntityNameTarget& named_target)
			{
				auto named_range = registry.view<NameComponent>().each();

				if (named_target.search_children_first)
				{
					if (auto child = find_child_impl(registry, source, named_target.entity_name, false); (child != null)) // true
					{
						target_entity = child;

						return;
					}
				}

				for (auto it = named_range.begin(); it != named_range.end(); it++)
				{
					const auto& [e_out, name] = *it;

					// First available match.
					if (name.hash() == named_target.entity_name)
					{
						target_entity = e_out;

						return;
					}
				}
			},

			[&registry, source, &target_entity](const ChildTarget& child_target)
			{
				//assert(child_target.child_name);

				target_entity = find_child_impl
				(
					registry, source,
					child_target.child_name,
					child_target.recursive
				);
			},
			
			[&registry, &target_entity](const PlayerTarget& player_target)
			{
				auto player_range = registry.view<PlayerComponent>().each();

				for (auto it = player_range.begin(); it != player_range.end(); it++)
				{
					const auto& [e_out, player_comp] = *it;

					// First available match.
					if (player_comp.player_index == player_target.player_index)
					{
						target_entity = e_out;

						return;
					}
				}
			},

			[&](const IndirectTarget& indirect_target)
			{
				auto* instance_comp = registry.try_get<InstanceComponent>(source);

				if (!instance_comp)
				{
					return;
				}

				auto deferred_expr = indirect_target.target_value_expr.get(instance_comp->get_storage());

				if (!deferred_expr)
				{
					return;
				}

				if (auto result = get_indirect_value_or_ref(deferred_expr, registry, source, args...))
				{
					if (auto result_as_entity = result.try_cast<EntityTarget>())
					{
						target_entity = result_as_entity->get(registry, source, args...);
					}
				}
			},

			[](const NullTarget&) // [&target_entity]
			{
				// Not actually required; defaults to `null` anyway.
				//target_entity = null;
			}
		);

		return target_entity;
	}

	std::optional<EntityTarget::ParseResult> EntityTarget::parse_type(std::string_view raw_value, const MetaParsingInstructions* opt_parsing_instructions)
	{
		using namespace engine::literals;

		if (raw_value.empty())
		{
			return std::nullopt;
		}

		if (raw_value.starts_with("self") || raw_value.starts_with("this"))
		{
			return ParseResult { SelfTarget {}, (sizeof("self")-1), true }; // 4
		}

		if (raw_value.starts_with("this"))
		{
			return ParseResult { SelfTarget {}, (sizeof("this")-1), true }; // 4
		}

		if (raw_value.starts_with("parent"))
		{
			return ParseResult { ParentTarget {}, (sizeof("parent")-1), true }; // 6
		}
		
		if (raw_value.starts_with("null"))
		{
			return ParseResult { NullTarget {}, (sizeof("null")-1), true };
		}

		if (auto [command_name, command_content, trailing_expr, is_string_content, command_parsed_length] = util::parse_command(raw_value); !command_name.empty())
		{
			// NOTE: Workaround to avoid additional temporary string allocation. (`std::regex` limitation)
			const auto parse_offset = (trailing_expr.empty())
				? raw_value.length()
				: static_cast<std::size_t>(trailing_expr.data() - raw_value.data())
			;

			// The above line would normally would look like this:
			//const auto parse_offset = static_cast<std::size_t>((command_content.end() + sizeof(")")) - raw_value.begin()));

			auto command_content_as_indirect = [&opt_parsing_instructions, &command_content, parse_offset]() -> std::optional<ParseResult>
			{
				if (command_content.empty())
				{
					return std::nullopt;
				}

				if (opt_parsing_instructions && opt_parsing_instructions->storage)
				{
					auto indirect_value = meta_any_from_string
					(
						command_content,
						*opt_parsing_instructions,
						engine::resolve<EntityTarget>(),
						false, false, false
					);

					if (indirect_value)
					{
						const auto indirect_value_type = indirect_value.type();

						if (auto as_indirect_meta_any = indirect_value.try_cast<IndirectMetaAny>())
						{
							return ParseResult { IndirectTarget { std::move(*as_indirect_meta_any) }, parse_offset, false };
						}
					}
				}

				return std::nullopt;
			};

			switch (hash(command_name))
			{
				case "entity"_hs:
					if (!is_string_content)
					{
						if (auto entity_id_raw = util::from_string<entt::id_type>(command_content))
						{
							return ParseResult { ExactEntityTarget { static_cast<Entity>(*entity_id_raw) }, parse_offset, false };
						}
						else if (auto as_indirect = command_content_as_indirect())
						{
							return *as_indirect;
						}
					}

					if (!command_content.empty())
					{
						return ParseResult { EntityNameTarget { hash(command_content) }, parse_offset, false };
					}

					break;

				case "child"_hs:
					if (!command_content.empty())
					{
						if (auto as_indirect = command_content_as_indirect())
						{
							return *as_indirect;
						}
						else if (command_content.contains(','))
						{
							auto parameter_idx = 0;

							std::string_view child_name;
							bool recursive = true;

							util::split(command_content, ",", [&parameter_idx, &child_name, &recursive](std::string_view argument, bool is_last_argument=false) -> bool
							{
								argument = util::trim(argument);

								switch (parameter_idx++)
								{
									// First argument: `child_name`
									case 0:
										child_name = argument;

										break;

									// Second argument: `recursive`
									case 1:
									{
										switch (hash(argument))
										{
											// Recursion is enabled by default; no
											// need to check for positive values:
											/*
											case "true"_hs:
											case "1"_hs:
												recursive = true;

												break;
											*/

											//case "f"_hs:
											case "0"_hs:
											case "false"_hs:
												recursive = false;

												break;
										}

										return false;
									}
								}

								return true;
							});

							return ParseResult { ChildTarget { hash(child_name), recursive }, parse_offset, false };
						}
						else
						{
							return ParseResult { ChildTarget { hash(command_content) }, parse_offset, false };
						}
					}

					break;

				case "player"_hs:
					if (!command_content.empty())
					{
						if (auto player_index = util::from_string<PlayerIndex>(command_content))
						{
							return ParseResult { PlayerTarget { *player_index }, parse_offset, false };
						}
						else if (auto as_indirect = command_content_as_indirect())
						{
							return *as_indirect;
						}
					}

					break;

				case "parent"_hs:
					return ParseResult { ParentTarget {}, parse_offset, false };

				case "self"_hs:
					return ParseResult { SelfTarget {}, parse_offset, false };

				case "name"_hs:
					if (!command_content.empty())
					{
						if (!is_string_content)
						{
							if (auto as_indirect = command_content_as_indirect())
							{
								return *as_indirect;
							}
						}
					
						return ParseResult { EntityNameTarget { hash(command_content) }, parse_offset, false };
					}

					break;
			}
		}

		//return ParseResult { SelfTarget {}, 0, false };

		return std::nullopt;
	}

	std::optional<EntityTarget> EntityTarget::parse(std::string_view raw_value, const MetaParsingInstructions* opt_parsing_instructions)
	{
		return from_parse_result(parse_type(raw_value, opt_parsing_instructions));
	}

	EntityTarget EntityTarget::from_parse_result(const ParseResult& result)
	{
		return EntityTarget { std::get<0>(result) };
	}

	std::optional<EntityTarget> EntityTarget::from_parse_result(const std::optional<ParseResult>& result)
	{
		if (!result)
		{
			return std::nullopt;
		}

		return from_parse_result(*result);
	}

	EntityTarget EntityTarget::from_entity(Entity entity)
	{
		return from_target_type(EntityTarget::ExactEntityTarget { entity });
	}

	EntityTarget EntityTarget::from_string(std::string_view raw_value)
	{
		if (auto result = parse(raw_value))
		{
			return std::move(*result);
		}

		return { EntityNameTarget { hash(raw_value) } };
	}

	EntityTarget EntityTarget::from_string(const std::string& raw_value)
	{
		return from_string(std::string_view { raw_value });
	}

	// EntityTarget::ChildTarget:
	EntityTarget::ChildTarget EntityTarget::ChildTarget::from_string(std::string_view child_name, bool recursive)
	{
		return { hash(child_name).value(), recursive };
	}

	EntityTarget::ChildTarget EntityTarget::ChildTarget::from_string(std::string_view child_name)
	{
		return from_string(child_name, true);
	}

	EntityTarget::ChildTarget EntityTarget::ChildTarget::from_string(const std::string& child_name, bool recursive)
	{
		return from_string(std::string_view { child_name }, recursive);
	}

	EntityTarget::ChildTarget EntityTarget::ChildTarget::from_string(const std::string& child_name)
	{
		return from_string(child_name, true);
	}

	// EntityTarget::EntityNameTarget:
	EntityTarget::EntityNameTarget EntityTarget::EntityNameTarget::from_string(std::string_view entity_name)
	{
		return { hash(entity_name).value() };
	}

	EntityTarget::EntityNameTarget EntityTarget::EntityNameTarget::from_string(const std::string& entity_name)
	{
		return from_string(std::string_view { entity_name });
	}
}