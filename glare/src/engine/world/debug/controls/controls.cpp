#include "controls.hpp"

#include <util/log.hpp>
#include <util/format.hpp>

#include <engine/world/world.hpp>
#include <engine/relationship.hpp>
#include <engine/bone_component.hpp>
#include <engine/transform.hpp>

#include <math/math.hpp>

#include <imgui/imgui.h>

#include <tuple>
#include <optional>
#include <string_view>

// imgui extensions:
namespace ImGui
{
	template <typename ...Args>
	void FormatText(fmt::format_string<Args...> fmt, Args&&... args)
	{
		Text(format(fmt, std::forward<Args>(args)...).c_str());
	}
}

namespace engine::debug
{
	namespace display
	{
		std::string name_and_id(World& world, Entity entity)
		{
			auto entity_name = world.get_name(entity);

			return format("{} ({})", entity_name, entity);
		}

		struct empty_child_display { void operator()(World& world, Entity child, const Relationship& relationship) {} };
		struct on_child_show_all   { auto operator()(Entity child) { return true; } };

		template <typename on_display_fn=empty_child_display, typename on_child_fn=on_child_show_all>
		void child_tree_ex(World& world, Entity entity, on_display_fn on_display, on_child_fn on_child, std::optional<std::string_view> node_name=std::nullopt)
		{
			auto& registry = world.get_registry();
			const auto& rel = registry.get<Relationship>(entity);

			bool node_open = false;
			bool render_tree = true;

			if (node_name.has_value())
			{
				assert(node_name->length());
				node_open = ImGui::TreeNode(node_name->data());
				render_tree = node_open;
			}

			if (render_tree)
			{
				rel.enumerate_children<std::tuple<bool, std::optional<bool>>>
				(
					registry,

					// Enter:
					[&world, &on_display, &on_child](Entity child, const Relationship& relationship, Entity next_child, const std::tuple<bool, std::optional<bool>>* parent_response) -> std::tuple<bool, std::optional<bool>>
					{
						bool continue_recursion = true;

						if (!on_child(child))
						{
							// Continue enumeration, no parent response known (skipped entity).
							return { true, std::nullopt }; // false
						}

						if (parent_response)
						{
							auto parent_value = std::get<1>(*parent_response);

							if (parent_value.has_value())
							{
								if (!parent_value.value())
								{
									// Continue enumeration, but parent declined tree node creation.
									return { true, false }; // { false, false };
								}
							}
						}

						if (ImGui::TreeNode(name_and_id(world, child).c_str()))
						{
							on_display(world, child, relationship);

							// Continue enumeration and indicate tree expansion.
							return { true, true };
						}

						return { true, false };
					},

					// Recurse.
					true,

					// Exit:
					[](Entity child, const Relationship& relationship, Entity next_child, const auto& response)
					{
						std::optional<bool> expanded = std::get<1>(response);

						// Check for open tree node.
						if (expanded.value_or(false))
						{
							ImGui::TreePop();
						}
					},

					[](const auto& response)
					{
						return std::get<0>(response);
					}
				);
			}

			if (node_open)
			{
				ImGui::TreePop();
			}
		}

		template <typename on_display_fn>
		void child_tree(World& world, Entity entity, on_display_fn on_display, std::optional<std::string_view> node_name=std::nullopt)
		{
			child_tree_ex(world, entity, on_display, [](Entity child) { return true; }, node_name);
		}

		void vectors(const math::Vector& pos, const math::Vector& rot, const math::Vector& scale)
		{
			ImGui::FormatText("x: {}, y: {}, z: {}", pos.x, pos.y, pos.z);
			ImGui::FormatText("pitch: {}, yaw: {}, roll: {}", rot.x, rot.y, rot.z);
			ImGui::FormatText("sx: {}, sy: {}, sz: {}", scale.x, scale.y, scale.z);
		}

		void transform(World& world, Entity entity, Transform& t)
		{
			if (ImGui::TreeNode("Local"))
			{
				vectors
				(
					t.get_local_position(),
					math::degrees(t.get_local_rotation()),
					t.get_local_scale()
				);

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Global"))
			{
				vectors
				(
					t.get_position(),
					math::degrees(t.get_rotation()),
					t.get_scale()
				);

				ImGui::TreePop();
			}
		}

		void transform(World& world, Entity entity)
		{
			auto t = world.get_transform(entity);

			transform(world, entity, t);
		}

		template <typename on_display_fn=empty_child_display, typename on_child_fn=on_child_show_all>
		void transform_tree(World& world, Entity entity, on_display_fn on_display, on_child_fn on_child)
		{
			child_tree_ex
			(
				world, entity,

				[&on_display](World& world, Entity child, const Relationship& relationship)
				{
					on_display(world, child, relationship);

					if (ImGui::TreeNode("Transform"))
					{
						transform(world, child);

						ImGui::TreePop();
					}

					ImGui::Separator();
				},

				on_child,

				"Children"
			);
		}

		template <typename on_display_fn=empty_child_display>
		void skeletal_tree(World& world, Entity entity, on_display_fn on_display)
		{
			auto& registry = world.get_registry();

			transform_tree
			(
				world, entity,

				[&registry, &on_display](World& world, Entity child, const Relationship& relationship)
				{
					auto& bone = registry.get<BoneComponent>(child);
					
					//ImGui::Separator();

					ImGui::FormatText("Bone ID: {}", bone.ID);

					on_display(world, child, relationship);
				},

				[&registry](Entity child) -> bool
				{
					return (registry.try_get<BoneComponent>(child));
				}
			);
		}
	}

	void animation_control(World& world, Entity entity)
	{
		using namespace display;

		auto& registry = world.get_registry();

		auto name = name_and_id(world, entity);

		auto wnd_name = format("{} - Animation Controls", name, entity);

		ImGui::Begin(wnd_name.c_str());

		ImGui::FormatText("Name: {}", name);

		const auto& rel = registry.get<Relationship>(entity);

		ImGui::FormatText("Number of Children: {}, {} locally", rel.total_children(registry), rel.children());

		skeletal_tree(world, entity, {});

		//ImGui::ShowDemoWindow();
		//ImGui::TreeNode

		/*
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);
		*/

		ImGui::End();
	}
}