#pragma once

//#include "types.hpp"

#include <util/small_vector.hpp>

#include <type_traits>
#include <algorithm>

namespace engine
{
	template <typename IDType, std::size_t local_buffer_size=6> // 4
	struct MetaIDFilter
	{
		public:
			using IDContainer = util::small_vector<IDType, local_buffer_size>;

			using id_type = IDType;
			using container_type = IDContainer;

			/*
				Executes `callback` with the elements in the sequence from `sequence_begin` until `sequence_end`,
				for any element allowed by this filter configuration. (i.e. `includes` reports true)
				
				The `callback` function specified may optionally return a
				boolean value indicating if execution shall continue.
				
				The return value of this function indicates the number of identifiers passed
				to `callback` where `callback` either returned nothing, or returned true.
			*/
			template <typename IteratorType, typename Callback>
			std::size_t filter(IteratorType sequence_begin, IteratorType sequence_end, Callback&& callback) const
			{
				std::size_t identifiers_handled = 0;

				for (auto it = sequence_begin; it != sequence_end; it++)
				{
					auto& id_to_compare = *it;

					if (includes(id_to_compare))
					{
						if constexpr (std::is_invocable_r_v<bool, Callback, decltype(id_to_compare)>)
						{
							if (!callback(id_to_compare))
							{
								break;
							}
						}
						else
						{
							callback(id_to_compare);
						}

						identifiers_handled++;
					}
				}

				return identifiers_handled;
			}

			// Returns true if the `id` specified is allowed by the configuration of this filter.
			bool includes(const IDType& id) const
			{
				if (is_inclusive)
				{
					return contains(id);
				}
				else
				{
					return (!contains(id));
				}
			}

			// Returns true if the `id` specified is not allowed by the configuration of this filter.
			bool excludes(const IDType& id) const
			{
				return (!includes(id));
			}

			// Adds `id_to_add` to the internal list of identifiers, regardless of intended configuration.
			bool add(const IDType& id_to_add)
			{
				if (contains(id_to_add))
				{
					return false;
				}

				identifiers.emplace_back(id_to_add);

				return true;
			}

			// Removes `id_to_remove` from the internal list of identifiers, regardless of intended configuration.
			bool remove(const IDType& id_to_remove)
			{
				if (auto existing_it = find(id_to_remove); existing_it != end())
				{
					identifiers.erase(existing_it);

					return true;
				}

				return false;
			}

			// Optionally adds or removes `id_to_include` based on the configuration of this filter.
			// 
			// i.e. If this filter is exclusive, this will attempt to remove `id_to_include`, if it is present.
			// If this filter is inclusive, this will add `id_to_include`, if it is not already present.
			bool include(const IDType& id_to_include)
			{
				if (is_inclusive)
				{
					return add(id_to_include);
				}
				else
				{
					return remove(id_to_include);
				}
			}

			// Optionally adds or removes `id_to_include` based on the configuration of this filter.
			// 
			// i.e. If this filter is exclusive, this will attempt to add `id_to_exclude`, if it is not already present.
			// If this filter is inclusive, this will remove `id_to_exclude`, if it is present.
			bool exclude(const IDType& id_to_exclude)
			{
				if (is_inclusive)
				{
					return remove(id_to_exclude);
				}
				else
				{
					return add(id_to_exclude);
				}
			}

			// Attempts to find `id_to_find` in the `identifiers` container, regardless of filter configuration.
			auto find(const IDType& id_to_find)
			{
				return std::find(begin(), end(), id_to_find);
			}

			// Attempts to find `id_to_find` in the `identifiers` container, regardless of filter configuration.
			auto find(const IDType& id_to_find) const
			{
				return std::find(begin(), end(), id_to_find);
			}

			// Returns true if `id_to_find` is found within `identifiers`, regardless of filter configuration.
			bool contains(const IDType& id_to_find) const
			{
				return (find(id_to_find) != end());
			}

			auto begin()
			{
				return identifiers.begin();
			}

			auto begin() const
			{
				return cbegin();
			}

			auto cbegin() const
			{
				return identifiers.cbegin();
			}

			auto end()
			{
				return identifiers.end();
			}

			auto end() const
			{
				return cend();
			}

			auto cend() const
			{
				return identifiers.cend();
			}

			bool filter_is_inclusive() const
			{
				return is_inclusive;
			}

			bool filter_is_exclusive() const
			{
				return (!is_inclusive);
			}

			// A collection of identifiers used for filtering.
			// See also: `is_inclusive`
			IDContainer identifiers;

			// Determines whether `identifiers` acts as a list
			// of included IDs (true) or exlcuded IDs (false).
			bool is_inclusive : 1 = true;
	};

	
}