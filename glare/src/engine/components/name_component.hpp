#pragma once

#include <string>

#include <engine/types.hpp>

namespace engine
{
	struct NameComponent
	{
		public:
			NameComponent(std::string&& name, StringHash name_hash);
			NameComponent(const std::string& name="Unknown");
			NameComponent(std::string&& name);

			NameComponent(const NameComponent&) = default;
			NameComponent(NameComponent&&) noexcept = default;

			NameComponent& operator=(const NameComponent&) = default;
			NameComponent& operator=(NameComponent&&) noexcept = default;

			StringHash hash() const;

			const std::string& get_name() const;
			void set_name(const std::string& name);

			inline std::size_t size() const
			{
				return name.size();
			}

			inline std::size_t length() const
			{
				return size();
			}

			inline explicit operator std::string() const
			{
				return name; // get_name();
			}
		protected:
			StringHash name_hash;

			std::string name;
	};
}