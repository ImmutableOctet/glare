#pragma once

#include <engine/meta/types.hpp>
#include <engine/meta/meta_variable.hpp>
#include <engine/meta/meta_variable_storage_interface.hpp>

#include <util/small_vector.hpp>

#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>

namespace engine
{
	struct MetaVariable;

	template <std::size_t preallocated=8>
	struct EntityVariables : MetaVariableStorageInterface
	{
		public:
			using Names  = util::small_vector<MetaSymbolID, preallocated>;
			using Values = util::small_vector<MetaAny, preallocated>;

			std::optional<std::size_t> get_index(MetaSymbolID name) const override
			{
				if (!name)
				{
					return std::nullopt;
				}

				for (std::size_t i = 0; i < names.size(); i++)
				{
					if (names[i] == name)
					{
						assert(values.size() > i);

						return i;
					}
				}

				return std::nullopt;
			}

			bool contains(MetaSymbolID name) const override
			{
				return (get_index(name) != std::nullopt);
			}

			const MetaAny* get(MetaSymbolID name) const override
			{
				if (auto index = get_index(name))
				{
					return &(values[*index]);
				}

				return {};
			}

			MetaAny* get(MetaSymbolID name) override
			{
				return const_cast<MetaAny*>
				(
					const_cast<const EntityVariables*>(this)->get(name)
				);
			}

			MetaAny* set(MetaVariable&& variable) override
			{
				if (!variable.has_name())
				{
					return nullptr;
				}

				return set(variable.name, std::move(variable.value));
			}

			MetaAny* set(MetaSymbolID name, MetaAny&& value) override
			{
				if (auto* existing = get(name))
				{
					*existing = std::move(value);

					return existing;
				}
				
				return emplace(name, std::move(value));
			}

			bool set_existing(MetaSymbolID name, MetaAny&& value) override
			{
				auto* existing = get(name);

				if (!existing)
				{
					return false;
				}

				*existing = std::move(value);

				return true;
			}

			bool set_missing(MetaSymbolID name, MetaAny&& value) override
			{
				if (get(name))
				{
					return false;
				}

				return emplace(name, std::move(value));
			}

			const MetaAny* data() const // override
			{
				return values.data();
			}

			std::size_t size() const override
			{
				return values.size();
			}

			Names& get_names() { return names; }
			const Names& get_names() const { return names; }

			Values& get_values() { return values; }
			const Values& get_values() const { return values; }

			explicit operator bool() const
			{
				return (size() > 0);
			}

			EntityVariables& operator<<(MetaVariable&& variable)
			{
				set(std::move(variable));

				return *this;
			}

			template <typename Callback>
			std::size_t enumerate_variables(Callback&& callback, std::size_t count, std::size_t offset=0)
			{
				const auto end_point = (offset + count);

				std::size_t variables_enumerated = 0;

				for (std::size_t i = offset; i < end_point; i++)
				{
					const auto& variable_name = names[i];
					auto& variable_value = values[i];

					if constexpr (std::is_invocable_r_v<bool, Callback, MetaSymbolID, MetaAny&> || std::is_invocable_r_v<bool, Callback, MetaSymbolID, const MetaAny&>)
					{
						if (!callback(variable_name, variable_value))
						{
							break;
						}
					}
					else if constexpr (std::is_invocable_r_v<bool, Callback, const MetaVariable&> || std::is_invocable_r_v<bool, Callback, MetaVariable&&>)
					{
						if (!callback(MetaVariable { variable_name, variable_value.as_ref() }))
						{
							break;
						}
					}
					else if constexpr (std::is_invocable_v<bool, Callback, const MetaVariable&> || std::is_invocable_v<bool, Callback, MetaVariable&&>)
					{
						callback(MetaVariable { variable_name, variable_value.as_ref() });
					}
					else
					{
						callback(variable_name, variable_value);
					}

					variables_enumerated++;
				}

				return variables_enumerated;
			}

			template <typename Callback>
			std::size_t enumerate_variables(Callback&& callback)
			{
				return enumerate_variables(std::forward<Callback>(callback), size(), 0);
			}

			template <typename Callback>
			std::size_t enumerate_variables(Callback&& callback, std::size_t count, std::size_t offset=0) const
			{
				return const_cast<EntityVariables*>(this)->enumerate_variables
				(
					std::forward<Callback>(callback),
					count,
					offset
				);
			}

			template <typename Callback>
			std::size_t enumerate_variables(Callback&& callback) const
			{
				return enumerate_variables(std::forward<Callback>(callback), size(), 0);
			}
		protected:
			MetaAny* emplace(MetaSymbolID name, MetaAny&& value)
			{
				names.emplace_back(name);

				auto& value_out = values.emplace_back(std::move(value));

				assert(names.size() == values.size());

				return &value_out;
			}

			Names names;
			Values values;
	};
}