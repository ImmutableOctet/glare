#pragma once

#include "controls.hpp"

#include <engine/types.hpp>
#include <engine/transform.hpp>

#include <engine/components/relationship_component.hpp>
#include <engine/components/transform_component.hpp>

#include <engine/meta/types.hpp>
#include <engine/meta/data_member.hpp>
#include <engine/meta/container.hpp>
#include <engine/meta/runtime_traits.hpp>

#include <engine/world/world.hpp>
#include <engine/world/animation/components/bone_component.hpp>

#include <util/imgui.hpp>
#include <util/format.hpp>

#include <string>
#include <string_view>
#include <type_traits>
#include <algorithm>
//#include <cmath>
#include <cstring>
#include <cassert>
#include <optional>

#include <glm/gtc/type_ptr.hpp>

namespace engine
{
	namespace display
	{
		namespace impl
		{
			template <typename PrimitiveType>
			std::optional<PrimitiveType> get_default_step_value()
			{
				if constexpr (std::is_arithmetic_v<PrimitiveType>)
				{
					if constexpr (std::is_floating_point_v<PrimitiveType>)
					{
						return static_cast<PrimitiveType>(0.01);
					}
					else
					{
						return static_cast<PrimitiveType>(1);
					}
				}

				return std::nullopt;
			}
		}

		template <typename VectorType>
		bool vector_ex(std::string_view display_text, VectorType&& vector_instance, bool allow_modification=false) // true
		{
			// TODO: Update implementation to allow for empty display text.
			if (display_text.empty())
			{
				return false;
			}

			bool vector_modified = false;

			if (auto raw_data = glm::value_ptr(vector_instance))
			{
				using value_type = typename std::remove_cvref_t<VectorType>::value_type;

				constexpr auto is_const = std::is_const_v<std::remove_reference_t<VectorType>>;
				constexpr auto vector_size = std::remove_cvref_t<VectorType>::length();

				if constexpr (is_const)
				{
					allow_modification = false;
				}

				if (!allow_modification)
				{
					ImGui::BeginDisabled();
				}

				if constexpr (std::is_same_v<value_type, float>)
				{
					switch (vector_size)
					{
						case 2:
							vector_modified = ImGui::DragFloat2(display_text.data(), raw_data);

							break;
						case 3:
							vector_modified = ImGui::DragFloat3(display_text.data(), raw_data);

							break;
						case 4:
							vector_modified = ImGui::DragFloat4(display_text.data(), raw_data);

							break;
					}
				}

				if (!allow_modification)
				{
					ImGui::EndDisabled();
				}
			}

			return vector_modified;
		}

		template <typename window_fn>
		void named_window(World& world, Entity entity, window_fn wnd, std::string_view subtitle={}, bool display_entity_name = true)
		{
			auto name = world.label(entity);

			std::string wnd_name;

			if (!subtitle.empty())
			{
				wnd_name = util::format("{} - {}", name, subtitle);
			}
			else
			{
				wnd_name = name;
			}

			ImGui::Begin(wnd_name.c_str());

			if (display_entity_name)
			{
				ImGui::FormatText("Name: {}", name);
			}

			wnd(entity);

			ImGui::End();
		}

		template <typename on_display_fn=empty_child_display, typename on_child_fn=on_child_show_all>
		void child_tree_ex(World& world, Entity entity, on_display_fn on_display, on_child_fn on_child, std::optional<std::string_view> node_name=std::nullopt)
		{
			auto& registry = world.get_registry();
			const auto& rel = registry.get<RelationshipComponent>(entity);

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
					[&world, &on_display, &on_child](Entity child, const RelationshipComponent& relationship, Entity next_child, const std::tuple<bool, std::optional<bool>>* parent_response) -> std::tuple<bool, std::optional<bool>>
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

						if (auto child_label = world.label(child); ImGui::TreeNode(child_label.c_str()))
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
					[](Entity child, const RelationshipComponent& relationship, Entity next_child, const auto& response)
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

		template <typename on_display_fn=empty_child_display>
		void child_tree(World& world, Entity entity, on_display_fn on_display, std::optional<std::string_view> node_name=std::nullopt)
		{
			child_tree_ex(world, entity, on_display, [](Entity child) { return true; }, node_name);
		}

		template <typename on_display_fn=empty_child_display, typename on_child_fn=on_child_show_all>
		void transform_tree(World& world, Entity entity, on_display_fn on_display, on_child_fn on_child)
		{
			child_tree_ex
			(
				world, entity,

				[&on_display](World& world, Entity child, const RelationshipComponent& relationship)
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

				[&registry, &on_display](World& world, Entity child, const RelationshipComponent& relationship)
				{
					/*
					auto& bone = registry.get<BoneComponent>(child);

					ImGui::FormatText("Bone ID: {}", bone.ID);
					*/

					on_display(world, child, relationship);
				},

				[&registry](Entity child) -> bool
				{
					return (registry.try_get<BoneComponent>(child));
				}
			);
		}

		template <typename PrimitiveType, typename PrimitiveValueType=std::decay_t<PrimitiveType>>
		bool primitive_value_ex
		(
			std::string_view display_text,
			PrimitiveType&& primitive_value,

			bool* value_modified_out=nullptr,
			
			std::optional<PrimitiveValueType> step=std::nullopt,

			// NOTE: Experimental parameter. (May change later)
			bool _read_only=false
		)
		{
			auto set_updated = [value_modified_out](bool value=true)
			{
				if (value_modified_out)
				{
					*value_modified_out = value;
				}

				return value;
			};

			const auto imgui_input_flags = (_read_only)
				? ImGuiInputTextFlags_ReadOnly
				: ImGuiInputTextFlags_None
			;

			if constexpr (std::is_same_v<PrimitiveValueType, bool>)
			{
				if (ImGui::Checkbox(display_text.data(), &primitive_value))
				{
					return set_updated();
				}
			}
			else if constexpr (std::is_same_v<PrimitiveValueType, float>)
			{
				/*
				// Alternative implementation:
				if (ImGui::InputFloat(display_text.data(), &primitive_value))
				{
					return set_updated();
				}
				*/

				if (ImGui::InputScalar(display_text.data(), ImGuiDataType_Float, &primitive_value, ((step && !_read_only) ? &(step.value()) : nullptr), nullptr, nullptr, imgui_input_flags))
				{
					return set_updated();
				}
			}
			else if constexpr (std::is_same_v<PrimitiveValueType, double>)
			{
				/*
				// Alternative implementation:
				if (ImGui::InputDouble(display_text.data(), &primitive_value))
				{
					return set_updated();
				}
				*/

				if (ImGui::InputScalar(display_text.data(), ImGuiDataType_Double, &primitive_value, ((step && !_read_only) ? &(step.value()) : nullptr), nullptr, nullptr, imgui_input_flags))
				{
					return set_updated();
				}
			}
			else if constexpr (std::is_same_v<PrimitiveValueType, std::int8_t>)
			{
				if (ImGui::InputScalar(display_text.data(), ImGuiDataType_S8, &primitive_value, ((step && !_read_only) ? &(step.value()) : nullptr), nullptr, nullptr, imgui_input_flags))
				{
					return set_updated();
				}
			}
			else if constexpr (std::is_same_v<PrimitiveValueType, std::int16_t>)
			{
				if (ImGui::InputScalar(display_text.data(), ImGuiDataType_S16, &primitive_value, ((step && !_read_only) ? &(step.value()) : nullptr), nullptr, nullptr, imgui_input_flags))
				{
					return set_updated();
				}
			}
			else if constexpr (std::is_same_v<PrimitiveValueType, std::int32_t>)
			{
				/*
				// Alternative implementation:
				if (ImGui::InputInt(display_text.data(), &primitive_value))
				{
					return set_updated();
				}
				*/

				if (ImGui::InputScalar(display_text.data(), ImGuiDataType_S32, &primitive_value, ((step && !_read_only) ? &(step.value()) : nullptr), nullptr, nullptr, imgui_input_flags))
				{
					return set_updated();
				}
			}
			else if constexpr (std::is_same_v<PrimitiveValueType, std::int64_t>)
			{
				if (ImGui::InputScalar(display_text.data(), ImGuiDataType_S64, &primitive_value, ((step && !_read_only) ? &(step.value()) : nullptr), nullptr, nullptr, imgui_input_flags))
				{
					return set_updated();
				}
			}
			else if constexpr (std::is_same_v<PrimitiveValueType, std::uint8_t>)
			{
				if (ImGui::InputScalar(display_text.data(), ImGuiDataType_U8, &primitive_value, ((step && !_read_only) ? &(step.value()) : nullptr), nullptr, nullptr, imgui_input_flags))
				{
					return set_updated();
				}
			}
			else if constexpr (std::is_same_v<PrimitiveValueType, std::uint16_t>)
			{
				if (ImGui::InputScalar(display_text.data(), ImGuiDataType_U16, &primitive_value, ((step && !_read_only) ? &(step.value()) : nullptr), nullptr, nullptr, imgui_input_flags))
				{
					return set_updated();
				}
			}
			else if constexpr (std::is_same_v<PrimitiveValueType, std::uint32_t>)
			{
				if (ImGui::InputScalar(display_text.data(), ImGuiDataType_U32, &primitive_value, ((step && !_read_only) ? &(step.value()) : nullptr), nullptr, nullptr, imgui_input_flags))
				{
					return set_updated();
				}
			}
			else if constexpr (std::is_same_v<PrimitiveValueType, std::uint64_t>)
			{
				if (ImGui::InputScalar(display_text.data(), ImGuiDataType_U64, &primitive_value, ((step && !_read_only) ? &(step.value()) : nullptr), nullptr, nullptr, imgui_input_flags))
				{
					return set_updated();
				}
			}
			else if constexpr (std::is_same_v<PrimitiveValueType, std::string>)
			{
				// Ensure that at least 512 characters (including the NULL-terminator) are available to be used for string input.
				// NOTE: This is an arbitrary limit that may change in the future.
				primitive_value.resize(std::max(primitive_value.capacity(), static_cast<std::size_t>(511))); // 1023

				if (ImGui::InputText(display_text.data(), primitive_value.data(), (primitive_value.size()), (imgui_input_flags | ImGuiInputTextFlags_EnterReturnsTrue)))
				{
					primitive_value.resize(std::strlen(primitive_value.data()));

					return set_updated();
				}
			}
			else if constexpr (std::is_same_v<PrimitiveValueType, std::string_view>)
			{
				ImGui::FormatText("{}: {}", display_text, primitive_value);
			}
			else
			{
				if (!display_text.empty())
				{
					ImGui::BeginDisabled();
					ImGui::FormatText("{}: Unavailable", display_text);
					ImGui::EndDisabled();
				}

				set_updated(false);

				return false; // return set_updated(false);
			}

			set_updated(false);

			return true;
		}

		template <typename ValueType>
		bool opaque_primitive_value_ex(std::string_view display_text, ValueType&& value, bool allow_modification=false, bool* value_modified_out=nullptr)
		{
			constexpr bool is_const_value = std::is_const_v<std::remove_reference_t<ValueType>>;

			bool value_displayed = false;
			bool value_modified = false;

			if constexpr (is_const_value)
			{
				assert(!allow_modification);

				allow_modification = false;
			}

			try_get_primitive_value
			(
				value,
				
				[&](auto&& primitive_value)
				{
					//constexpr auto is_const_primitive_value = is_const_value;
					constexpr bool is_const_primitive_value = std::is_const_v<std::remove_reference_t<decltype(primitive_value)>>;

					using primitive_t = std::decay_t<decltype(primitive_value)>;

					if constexpr (is_const_primitive_value)
					{
						auto copy_of_primitive_value = primitive_value;

						ImGui::BeginDisabled();

						value_displayed = primitive_value_ex
						(
							display_text,
							copy_of_primitive_value,
							nullptr,
							std::optional<primitive_t>(std::nullopt),
							true
						);

						ImGui::EndDisabled();
					}
					else
					{
						const auto can_modify_field_value = (allow_modification);

						if (!can_modify_field_value)
						{
							ImGui::BeginDisabled();
						}

						value_displayed = primitive_value_ex
						(
							display_text, primitive_value,
							
							&value_modified,

							(can_modify_field_value)
								? impl::get_default_step_value<primitive_t>()
								: std::optional<primitive_t>(std::nullopt),

							(!can_modify_field_value)
						);

						if (!can_modify_field_value)
						{
							ImGui::EndDisabled();
						}
					}
				}
			);

			if constexpr (is_const_value)
			{
				//assert(!value_updated);

				if (value_modified_out)
				{
					*value_modified_out = false;
				}
			}
			else
			{
				if (value_modified_out)
				{
					*value_modified_out = value_modified;
				}
			}

			return value_displayed;
		}

		template <typename ValueType>
		bool opaque_value_ex(std::string_view display_text, ValueType&& value, bool allow_modification=false, bool* value_modified_out=nullptr)
		{
			using namespace engine::literals;

			if (!value)
			{
				return false;
			}

			constexpr bool is_const_value = std::is_const_v<std::remove_reference_t<ValueType>>;

			if constexpr (is_const_value)
			{
				assert(!allow_modification);

				allow_modification = false;
			}
			
			if (value_is_primitive(value))
			{
				return opaque_primitive_value_ex(display_text, value, allow_modification);
			}

			if (!display_text.empty())
			{
				ImGui::Text(display_text.data());
				ImGui::Spacing();
			}

			auto close_tree_node = []()
			{
				ImGui::TreePop();
				ImGui::Spacing();

				return true;
			};

			auto open_tree_node = [](std::string_view node_display_text, bool highlight=false)
			{
				return ImGui::TreeNodeEx
				(
					node_display_text.data(),

					(highlight)
						? (ImGuiTreeNodeFlags_Bullet) // | ImGuiTreeNodeFlags_OpenOnDoubleClick
						: ImGuiTreeNodeFlags_None
				);
			};

			bool value_displayed = false;
			bool value_updated = false;

			enumerate_primitive_member_values
			(
				value,

				[allow_modification, &value_displayed](auto&& instance, MetaSymbolID field_id, const entt::meta_data& field, auto& field_value, bool& field_updated_out)
				{
					using field_native_t = std::remove_cvref_t<decltype(field_value)>; // std::decay_t
					
					value_displayed = true;

					const auto field_name = get_known_string_from_hash(field_id);

					if (field_name.empty())
					{
						return;
					}

					bool field_updated = false;

					if constexpr (is_const_value)
					{
						ImGui::BeginDisabled();

						primitive_value_ex
						(
							field_name,
							field_value,
							nullptr,
							std::optional<field_native_t>(std::nullopt),
							true
						);

						ImGui::EndDisabled();
					}
					else
					{
						const auto can_modify_field_value = ((allow_modification) && (!field.is_const()));

						if (!can_modify_field_value)
						{
							ImGui::BeginDisabled();
						}

						primitive_value_ex
						(
							field_name,
							field_value, // std::forward<decltype(field_value)>(field_value),
							
							&field_updated,

							(can_modify_field_value)
								? impl::get_default_step_value<field_native_t>()
								: std::optional<field_native_t>(std::nullopt),

							(!can_modify_field_value)
						);

						if (!can_modify_field_value)
						{
							ImGui::EndDisabled();
						}
					}

					field_updated_out = field_updated;
				},

				[allow_modification, &open_tree_node, &close_tree_node, &value_displayed](auto&& parent_instance, MetaSymbolID nested_field_id, const entt::meta_data& nested_field, auto&& nested_value, bool& nested_value_updated) -> bool
				{
					const auto nested_field_name = get_known_string_from_hash(nested_field_id);

					if (nested_field_name.empty())
					{
						return false;
					}

					const auto nested_type = nested_value.type();
					const auto nested_type_id = nested_type.id();

					const auto nested_type_name = get_known_string_from_hash(nested_type_id);

					const auto can_modify_nested_field_value = ((allow_modification) && (!nested_field.is_const()));

					bool tree_node_opened = false;

					bool nested_type_has_members = true;

					if (!meta_type_is_container(nested_type, false)) // true
					{
						if (!meta_type_has_data_member(nested_type, true))
						{
							nested_type_has_members = false;
						}
					}

					if (!nested_type_has_members)
					{
						ImGui::BeginDisabled();
					}

					if (!nested_type_name.empty())
					{
						const auto field_and_type = util::format("{}:{}", nested_field_name, nested_type_name);

						tree_node_opened = open_tree_node(field_and_type, nested_field.is_const());
					}
					else
					{
						tree_node_opened = open_tree_node(nested_field_name, nested_field.is_const());
					}

					if (tree_node_opened)
					{
						/*
						// Disabled for now:
						if (!can_modify_nested_field_value)
						{
							ImGui::BeginDisabled();
						}
						*/

						auto vector_impl = [can_modify_nested_field_value, &nested_value, &nested_value_updated]<typename VectorType>(std::string_view display_text)
						{
							if (auto as_vector = nested_value.try_cast<VectorType>())
							{
								if (vector(display_text, *as_vector, can_modify_nested_field_value))
								{
									nested_value_updated = true;

									return true;
								}
							}

							return false;
						};

						bool specialization_found = true;

						// Non-primitive type specializations:
						switch (nested_type_id)
						{
							case "Vector4D"_hs: // entt::type_hash<math::Vector4D>::value():
								vector_impl.operator()<math::Vector4D>(nested_type_name);

								break;
							case "Vector"_hs: // "Vector3D"_hs // entt::type_hash<math::Vector>::value():
								vector_impl.operator()<math::Vector3D>(nested_type_name);
								
								break;
							case "Vector2D"_hs: // entt::type_hash<math::Vector2D>::value():
								vector_impl.operator()<math::Vector2D>(nested_type_name);

								break;

							case "TransformVectors"_hs: // entt::type_hash<math::TransformVectors>::value():
								if (auto as_tform = nested_value.try_cast<math::TransformVectors>())
								{
									if (vector("position", std::get<0>(*as_tform), can_modify_nested_field_value))
									{
										nested_value_updated = true;
									}

									if (vector("rotation", std::get<1>(*as_tform), can_modify_nested_field_value))
									{
										nested_value_updated = true;
									}

									if (vector("scale", std::get<2>(*as_tform), can_modify_nested_field_value))
									{
										nested_value_updated = true;
									}
								}

								break;

							default:
							{
								specialization_found = false;

								if (auto sequence_container = try_get_sequence_container(nested_value))
								{
									if (sequence_container.size())
									{
										value_displayed = true;

										std::size_t index = 0;

										for (auto entry : sequence_container)
										{
											const auto entry_name = util::format("[{}]", index);

											assert(!entry_name.empty());

											/*
											// Alternative implementation:
											if (open_tree_node(entry_name))
											{
												const auto entry_type = entry.type();
												const auto entry_type_id = entry_type.id();
												const auto entry_type_name = get_known_string_from_hash(entry_type_id);

												opaque_value_ex(entry_type_name, entry, allow_modification);

												close_tree_node();
											}
											*/

											opaque_value_ex(entry_name, entry, allow_modification);

											index++;
										}

										// Disabled for now; although this is a container,
										// it may also have members we want to display.
										//specialization_found = true;
									}
								}
								else if (auto associative_container = try_get_associative_container(nested_value))
								{
									if (associative_container.size())
									{
										value_displayed = true;

										for (auto [container_key, container_value] : associative_container)
										{
											if (!container_key.allow_cast<std::string>())
											{
												break;
											}

											const auto container_key_as_string = container_key.cast<std::string>();

											if (container_key_as_string.empty())
											{
												continue;
											}

											const auto entry_name = util::format("[{}]", container_key_as_string);

											/*
											// Alternative implementation:
											if (open_tree_node(entry_name))
											{
												const auto container_value_type = container_value.type();
												const auto container_value_type_id = container_value_type.id();
												const auto container_value_type_name = get_known_string_from_hash(container_value_type_id);

												opaque_value_ex(container_value_type_name, container_value, allow_modification);

												close_tree_node();
											}
											*/

											opaque_value_ex(entry_name, container_value, allow_modification);
										}

										// Disabled for now; although this is a container,
										// it may also have members we want to display.
										//specialization_found = true;
									}
								}

								break;
							}
						}

						/*
						// Disabled for now:
						if (!can_modify_nested_field_value)
						{
							ImGui::EndDisabled();
						}
						*/

						if (specialization_found)
						{
							close_tree_node();

							value_displayed = true;
							tree_node_opened = false;
						}
					}

					if (!nested_type_has_members)
					{
						ImGui::EndDisabled();
					}

					return tree_node_opened;
				},

				[&close_tree_node](auto&&...)
				{
					close_tree_node();
				},

				true, true, true, false,
				&value_updated
			);

			if constexpr (is_const_value)
			{
				//assert(!value_updated);

				if (value_modified_out)
				{
					*value_modified_out = false;
				}
			}
			else
			{
				assert((!value_updated) || (allow_modification));

				if (value_modified_out)
				{
					*value_modified_out = value_updated;
				}
			}

			return value_displayed;
		}
	}
}