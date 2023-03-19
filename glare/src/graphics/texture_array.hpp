#pragma once

#include <memory>
#include <variant>
#include <tuple>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace graphics
{
	class Texture;

	using TextureArray = std::vector<std::shared_ptr<Texture>>;

	// Names are owning views.
	using NamedTextureArray = std::vector<std::tuple<std::string, std::shared_ptr<Texture>>>; // string_view

	using TextureArrayRaw = std::vector<const Texture*>;

	// Names are non-owning views.
	//using NamedTextureArrayRaw = std::vector<std::tuple<std::string_view, const Texture*>>;
	using NamedTextureArrayRaw = std::unordered_map<std::string, TextureArrayRaw>; // std::vector<const Texture*>

	using TextureGroup = std::variant<std::shared_ptr<Texture>, TextureArray>; // Used to represent a single texture object or vector of textures objects. (Represents a 'TextureClass')
	//using NamedTextureGroup = std::tuple<std::string, TextureGroup>;

	using TextureGroupRaw = std::variant<const Texture*, TextureArrayRaw*>;

	//using NamedTextureGroupRaw = std::tuple<std::string, TextureGroup>;
	using NamedTextureGroupRaw = std::variant<std::tuple<std::string_view, const Texture*>, const NamedTextureArrayRaw*>;
}