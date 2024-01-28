#include "controls.hpp"

#include "controls_impl.hpp"

#include <util/log.hpp>
#include <util/format.hpp>

#include <engine/components/relationship_component.hpp>
#include <engine/transform.hpp>

#include <engine/entity/entity_descriptor.hpp>
#include <engine/entity/components/instance_component.hpp>
#include <engine/entity/components/static_mutation_component.hpp>

#include <engine/meta/component.hpp>
#include <engine/meta/hash.hpp>

#include <engine/world/world.hpp>

#include <engine/world/animation/components/animation_component.hpp>
#include <engine/world/animation/components/bone_component.hpp>

//#include <engine/world/animation/XXX.hpp>

#include <math/math.hpp>

#include <util/imgui.hpp>

#include <tuple>
#include <optional>
#include <string_view>
#include <utility>

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	namespace display
	{
		bool vector(std::string_view display_text, math::Vector4D& v, bool allow_modification)
		{
			if (!allow_modification)
			{
				vector(display_text, std::as_const(v));

				return false;
			}

			return vector_ex(display_text, v);
		}

		bool vector(std::string_view display_text, math::Vector3D& v, bool allow_modification)
		{
			if (!allow_modification)
			{
				vector(display_text, std::as_const(v));

				return false;
			}

			return vector_ex(display_text, v);
		}

		bool vector(std::string_view display_text, math::Vector2D& v, bool allow_modification)
		{
			if (!allow_modification)
			{
				vector(display_text, std::as_const(v));

				return false;
			}

			return vector_ex(display_text, v);
		}

		void vector(std::string_view display_text, const math::Vector4D& v)
		{
			vector_ex(display_text, math::Vector4D { v }, false);
		}

		void vector(std::string_view display_text, const math::Vector3D& v)
		{
			vector_ex(display_text, math::Vector3D { v }, false);
		}

		void vector(std::string_view display_text, const math::Vector2D& v)
		{
			vector_ex(display_text, math::Vector2D { v }, false);
		}

		void transform(const math::Vector& position, const math::Vector& rotation, const math::Vector& scale)
		{
			ImGui::FormatText("x: {}, y: {}, z: {}", position.x, position.y, position.z);
			ImGui::FormatText("pitch: {}, yaw: {}, roll: {}", rotation.x, rotation.y, rotation.z);
			ImGui::FormatText("sx: {}, sy: {}, sz: {}", scale.x, scale.y, scale.z);
		}

		void transform(const math::TransformVectors& transform_vectors)
		{
			transform(std::get<0>(transform_vectors), std::get<1>(transform_vectors), std::get<2>(transform_vectors));
		}

		void transform(const Transform& tform)
		{
			if (ImGui::TreeNode("Local"))
			{
				transform
				(
					tform.get_local_position(),
					math::degrees(tform.get_local_rotation()),
					tform.get_local_scale()
				);

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Global"))
			{
				transform
				(
					tform.get_position(),
					math::degrees(tform.get_rotation()),
					tform.get_scale()
				);

				ImGui::TreePop();
			}
		}

		void transform(Registry& registry, Entity entity)
		{
			auto* tform_component = registry.try_get<TransformComponent>(entity);

			if (!tform_component)
			{
				return;
			}

			auto tform = Transform(registry, entity, *tform_component);

			transform(tform);
		}

		void transform(World& world, Entity entity)
		{
			auto& registry = world.get_registry();
			
			transform(registry, entity);
		}

		/*
		// TODO: Reimplement
		void animation(const graphics::AnimationData& a, std::string_view display_name)
		{
			if (ImGui::TreeNode(display_name.data()))
			{
				ImGui::FormatText("ID: #{}", a.id);
				ImGui::FormatText("Duation: {} frames", a.duration);
				ImGui::FormatText("Rate: {} fps", a.rate);

				ImGui::TreePop();
			}
		}
		*/

		void animator(AnimationComponent& animator)
		{
			// TODO: Reimplement
			/*
			if (ImGui::TreeNode("AnimationComponent"))
			{
				const AnimationData* cur_anim = animator.get_current_animation();

				if (cur_anim)
				{
					ImGui::SliderFloat("Time", &animator.time, 0.0f, cur_anim->duration, "%.3f");
				}

				//ImGui::DragFloat("Rate", &animator.rate, 0.0001f, 0.0f, 1.0f, "%.06f");
				ImGui::SliderFloat("Rate", &animator.rate, 0.0f, 1.0f, "%.06f");

				if (animator.playing())
				{
					if (ImGui::Button("Pause", {54, 24}))
						animator.pause();
				}
				else
				{
					if (ImGui::Button("Play", {54, 24}))
						animator.play();
				}

				ImGui::SameLine();

				ImGui::FormatText("State: {}", static_cast<int>(animator.get_state()));

				ImGui::TreePop();
			}
			*/
		}

		void child_tree(World& world, Entity entity)
		{
			child_tree<>(world, entity, {});
		}

		void transform_tree(World& world, Entity entity)
		{
			transform_tree<>(world, entity, {}, {});
		}

		void skeletal_tree(World& world, Entity entity)
		{
			skeletal_tree<>(world, entity, {});
		}

		void opaque_value(const MetaAny& value)
		{
			opaque_value_ex({}, value, false);
		}

		bool opaque_value(MetaAny& value, bool allow_modification)
		{
			return opaque_value_ex({}, value, allow_modification);
		}

		void opaque_value(std::string_view display_text, const MetaAny& value)
		{
			opaque_value_ex(display_text, value, false);
		}

		bool opaque_value(std::string_view display_text, MetaAny& value, bool allow_modification)
		{
			return opaque_value_ex(display_text, value, allow_modification);
		}

		bool component_content(Registry& registry, Entity entity, const MetaType& component_type, bool allow_modification, bool* component_modified_out)
		{
			auto component = get_component_ref(registry, entity, component_type);

			if (!component)
			{
				return false;
			}

			bool component_updated = false;

			opaque_value_ex({}, component, allow_modification, &component_updated);

			if (component_updated)
			{
				// Check for disallowed modification of component instances.
				assert(allow_modification);

				if (allow_modification)
				{
					const auto patch_result = mark_component_as_patched(registry, entity, component_type);

					assert(patch_result);

					if (!patch_result)
					{
						const auto component_type_id = component_type.id();
						const auto component_type_name = get_known_string_from_hash(component_type_id);

						print_warn("Failed to properly mark component as patched: {} (#{})", component_type_name, component_type_id);
					}
				}
			}

			if (component_modified_out)
			{
				*component_modified_out = component_updated;
			}

			return true;
		}

		bool component(Registry& registry, Entity entity, MetaTypeID component_type_id, bool allow_modification, bool* component_modified_out, bool highlight)
		{
			if (!component_type_id)
			{
				return false;
			}

			const auto component_type = resolve(component_type_id);

			return component(registry, entity, component_type, allow_modification, component_modified_out, highlight);
		}

		bool component(Registry& registry, Entity entity, const MetaType& component_type, bool allow_modification, bool* component_modified_out, bool highlight)
		{
			if (!component_type)
			{
				return false;
			}

			const auto component_type_id = component_type.id();
			const auto component_type_name = get_known_string_from_hash(component_type_id);

			if (component_type_name.empty())
			{
				return false;
			}

			const auto component_has_members = meta_type_has_data_member(component_type, true);

			if (!component_has_members)
			{
				ImGui::BeginDisabled();
			}

			bool component_node_opened = (highlight)
				? ImGui::TreeNodeEx(component_type_name.data(), ImGuiTreeNodeFlags_Framed) // | ImGuiTreeNodeFlags_Bullet
				: ImGui::TreeNode(component_type_name.data())
			;

			bool component_displayed = false;
			bool component_modified = false;

			if (component_node_opened)
			{
				if (const auto component_type = resolve(component_type_id))
				{
					component_displayed = component_content(registry, entity, component_type, allow_modification, &component_modified);

					if (!component_displayed)
					{
						ImGui::BeginDisabled();
						ImGui::FormatText("Component unavailable");
						ImGui::EndDisabled();
					}
				}
				else
				{
					ImGui::BeginDisabled();
					ImGui::FormatText("{}: Type information unavailable", component_type_name);
					ImGui::EndDisabled();
				}

				ImGui::TreePop();
				ImGui::Spacing();
			}

			if (component_modified_out)
			{
				*component_modified_out = component_modified;
			}

			if (!component_has_members)
			{
				ImGui::EndDisabled();
			}

			return component_displayed;
		}

		std::size_t components
		(
			Registry& registry, Entity entity,

			bool allow_modification,

			bool display_from_mutations,
			bool display_from_descriptor,
			bool check_mutation_status
		)
		{
			std::size_t components_displayed = 0;

			StaticMutationComponent* mutation_comp = nullptr;
						
			if ((check_mutation_status || display_from_mutations))
			{
				mutation_comp = registry.try_get<StaticMutationComponent>(entity);
			}

			bool mutations_found = false;

			if ((display_from_mutations) && (mutation_comp))
			{
				if (!mutation_comp->mutated_components.empty())
				{
					if (ImGui::TreeNode("Scene")) // Modified
					{
						for (const auto& component_entry : mutation_comp->mutated_components)
						{
							const auto& component_type_id = component_entry;

							if (display::component(registry, entity, component_type_id, allow_modification, nullptr, true))
							{
								components_displayed++;
							}
						}

						ImGui::TreePop();
						ImGui::Spacing();
					}

					mutations_found = true;
				}
			}

			if (display_from_descriptor)
			{
				if (const auto* instance_comp = registry.try_get<InstanceComponent>(entity))
				{
					const auto& descriptor = instance_comp->get_descriptor();

					if (!descriptor.components.type_definitions.empty())
					{
						if (mutations_found)
						{
							ImGui::Separator();
						}

						if (ImGui::TreeNode("Archetype")) // "Unmodified"
						{
							for (const auto& component_entry : descriptor.components.type_definitions)
							{
								const auto& type_descriptor = component_entry.get(descriptor);

								const auto component_type_id = type_descriptor.get_type_id();

								bool is_modified_component = false;

								if ((mutation_comp) && (check_mutation_status || mutations_found))
								{
									if (mutation_comp->contains_component(component_type_id))
									{
										// Mark this component as modified.
										is_modified_component = true;
									}
								}

								bool component_updated = false;

								if (display::component(registry, entity, component_type_id, allow_modification, &component_updated, is_modified_component))
								{
									components_displayed++;
								}

								if (component_updated)
								{
									if (!mutation_comp)
									{
										mutation_comp = &(registry.emplace<StaticMutationComponent>(entity));
									}

									if (mutation_comp)
									{
										mutation_comp->add_component(component_type_id);
									}
								}
							}

							ImGui::TreePop();
							ImGui::Spacing();
						}
					}
				}
			}

			return components_displayed;
		}
	}

	void animation_control(World& world, Entity entity)
	{
		auto& registry = world.get_registry();

		display::named_window
		(
			world, entity,
			
			[&world, &registry](Entity entity)
			{
				auto* animator = registry.try_get<AnimationComponent>(entity);

				if (!animator)
				{
					return;
				}

				const auto& rel = registry.get<RelationshipComponent>(entity);

				ImGui::FormatText("Number of Children: {} ({} locally)", rel.total_children(registry), rel.children());

				ImGui::Separator();

				display::animator(*animator);

				/*
				// TODO: Reimplement
				const AnimationData* cur_anim = animator->get_current_animation();
				const AnimationData* prev_anim = animator->get_prev_animation();

				if (cur_anim)
				{
					display::animation(*cur_anim, "Current AnimationData");
				}

				if ((prev_anim) && (cur_anim != prev_anim))
				{
					display::animation(*prev_anim, "Previous AnimationData");
				}
				*/

				display::skeletal_tree(world, entity, {});
			},

			"AnimationData Controls"
		);
	}

	void hierarchy_control(World& world, Entity entity)
	{
		display::named_window
		(
			world, entity, [&world](Entity entity)
			{
				display::transform_tree(world, entity);
			}
		);
	}
}