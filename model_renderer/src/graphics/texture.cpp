//#include <util/io.hpp>

#include "context.hpp"
#include "texture.hpp"
#include "pixelmap.hpp"

namespace graphics
{
	// Texture:
	Texture::Texture(weak_ref<Context> ctx, Context::Handle&& handle)
		: Resource(ctx, std::move(handle)) {}

	Texture::Texture(pass_ref<Context> ctx, const PixelMap& data, Flags flags)
		: Texture(ctx, ctx->generate_texture(data, flags)) {}

	Texture::Texture(pass_ref<Context> ctx, const std::string& path, Flags flags)
		: Texture(ctx, PixelMap::Load(path), flags) {}

	Texture::~Texture()
	{
		get_context()->release_texture(std::move(handle));
	}
}