#include "name_component.hpp"

#include <engine/meta/hash.hpp>

#include <utility>

namespace engine
{
	NameComponent::NameComponent(const std::string& name)
		: name_hash(engine::hash(name)), name(name) {}

	NameComponent::NameComponent(std::string&& name, StringHash name_hash)
		: name_hash(name_hash), name(std::move(name)) {}

	NameComponent::NameComponent(std::string&& name)
		: name_hash(engine::hash(name)), name(std::move(name)) {}

	StringHash NameComponent::hash() const
	{
		return name_hash;
	}

	const std::string& NameComponent::get_name() const
	{
		return name;
	}

	void NameComponent::set_name(const std::string& name)
	{
		this->name = name;
		this->name_hash = engine::hash(name);
	}
}