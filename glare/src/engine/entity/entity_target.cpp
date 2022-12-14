#include "entity_target.hpp"

#include <engine/meta/meta.hpp>

#include <engine/components/relationship_component.hpp>
#include <engine/components/name_component.hpp>
#include <engine/components/player_component.hpp>

#include <util/string.hpp>
#include <util/parse.hpp>

namespace engine
{
	Entity EntityTarget::resolve(Registry& registry, Entity source) const
	{
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

			[&registry, &target_entity](const EntityNameTarget& named_target)
			{
				auto named_range = registry.view<NameComponent>().each();

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

				const RelationshipComponent* relationship = registry.try_get<RelationshipComponent>(source);

				if (!relationship)
				{
					return;
				}

				relationship->enumerate_children
				(
					registry,
									
					[&registry, &child_target, &target_entity](Entity child, const RelationshipComponent& child_relationship, Entity next_child) -> bool
					{
						const auto* child_name_comp = registry.try_get<NameComponent>(child);

						if (!child_name_comp)
						{
							return true;
						}

						if (child_name_comp->hash() == child_target.child_name)
						{
							target_entity = child;

							return false;
						}
										
						return true;
					},

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

			[](const NullTarget&) // [&target_entity]
			{
				// Not actually required; defaults to `null` anyway.
				//target_entity = null;
			}
		);

		return target_entity;
	}

	// TODO: Remove. (Workaround for `std::regex`)
	std::optional<EntityTarget::ParseResult> EntityTarget::parse_type(std::string_view raw_value)
	{
		return parse_type(std::string(raw_value));
	}

	std::optional<EntityTarget::ParseResult> EntityTarget::parse_type(const std::string& raw_value)
	{
		using namespace entt::literals;

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
			return ParseResult{ NullTarget {}, (sizeof("null")-1), true };
		}

		if (auto [command_name, command_content, is_string_content] = util::parse_single_argument_command(raw_value); !command_name.empty())
		{
			// NOTE: Workaround to avoid additional temporary string allocation. (`std::regex` limitation)
			const auto parse_offset = static_cast<std::size_t>((command_content.data() + command_content.size() + (sizeof(")")-1)) - raw_value.data());

			// The above line would normally would look like this:
			//const auto parse_offset = static_cast<std::size_t>((command_content.end() + sizeof(")")) - raw_value.begin()));

			switch (hash(command_name))
			{
				case "entity"_hs:
					if (!is_string_content)
					{
						if (auto entity_id_raw = util::from_string<entt::id_type>(raw_value))
						{
							return ParseResult { ExactEntityTarget { static_cast<Entity>(*entity_id_raw) }, parse_offset, false };
						}
					}

					break;

				case "child"_hs:
				{
					if (command_content.contains(','))
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

				case "player"_hs:
					if (auto player_index = util::from_string<PlayerIndex>(command_content))
					{
						return ParseResult { PlayerTarget { *player_index }, parse_offset, false };
					}

					break;

				case "parent"_hs:
					return ParseResult { ParentTarget {}, parse_offset, false };

				case "self"_hs:
					return ParseResult { SelfTarget {}, parse_offset, false };
			}

			return ParseResult { EntityNameTarget { hash(command_content) }, parse_offset, false };
		}

		//return ParseResult { SelfTarget {}, 0, false };

		return std::nullopt;
	}
}