#pragma once

#include <engine/types.hpp>
#include <engine/meta/types.hpp>

namespace engine
{
	namespace impl
	{
		template <typename SamplerType>
		MetaType sampler_get_wrapped_container_type()
		{
			return resolve<typename SamplerType::Container>();
		}

		template <typename SamplerType>
		MetaTypeID sampler_get_wrapped_container_type_id()
		{
			if (auto type = sampler_get_wrapped_container_type<SamplerType>())
			{
				return type.id();
			}

			return {};
		}

		template <typename SamplerType>
		MetaType sampler_get_value_type()
		{
			return resolve<typename SamplerType::value_type>();
		}

		template <typename SamplerType>
		MetaTypeID sampler_get_value_type_id()
		{
			if (auto type = sampler_get_value_type<SamplerType>())
			{
				return type.id();
			}

			return {};
		}

		template <typename SamplerType>
		MetaType sampler_get_key_type()
		{
			return resolve<typename SamplerType::Entry>();
		}

		template <typename SamplerType>
		MetaTypeID sampler_get_key_type_id()
		{
			if (auto type = sampler_get_key_type<SamplerType>())
			{
				return type.id();
			}

			return {};
		}

		template <typename SamplerType>
		MetaType sampler_get_pair_type()
		{
			return resolve<typename SamplerType::Entry>();
		}

		template <typename SamplerType>
		MetaTypeID sampler_get_pair_type_id()
		{
			if (auto type = sampler_get_pair_type<SamplerType>())
			{
				return type.id();
			}

			return {};
		}
	}
}