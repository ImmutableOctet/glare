//#include <util/io.hpp>

#include "context.hpp"
#include "texture.hpp"
#include "pixelmap.hpp"

namespace graphics
{
	// Texture:
	Texture::Texture(weak_ref<Context> ctx, ContextHandle&& handle)
		: Resource(ctx, std::move(handle)) {}

	Texture::Texture(pass_ref<Context> ctx, const PixelMap& data, Flags flags)
		: Texture(ctx, ctx->generate_texture(data, ElementType::UByte, flags)) {}

	Texture::Texture(pass_ref<Context> ctx, int width, int height, TextureFormat format, ElementType element_type, TextureFlags flags)
		: Texture(ctx, ctx->generate_texture(width, height, format, element_type, flags)) {}

	Texture::Texture(pass_ref<Context> ctx, const std::string& path, Flags flags)
		: Texture(ctx, PixelMap::Load(path), flags) {}

	Texture::~Texture()
	{
		get_context()->release_texture(std::move(handle));
	}
}