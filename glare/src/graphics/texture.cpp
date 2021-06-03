//#include <util/io.hpp>
#include <algorithm>

#include "context.hpp"
#include "texture.hpp"
#include "pixelmap.hpp"

namespace graphics
{
	// Texture:
	Texture::Texture(weak_ref<Context> ctx, ContextHandle&& handle, int width, int height, TextureFormat format, ElementType element_type, TextureFlags flags, TextureType type)
		: Resource(ctx, std::move(handle)), width(width), height(height), format(format), element_type(element_type), flags(flags), type(type) {}

	Texture::Texture(pass_ref<Context> ctx, const PixelMap& data, Flags flags, TextureType type)
		: Texture(ctx, ctx->generate_texture(data, ElementType::UByte, flags, type), data.width(), data.height(), data.format(), ElementType::UByte, flags, type) {}

	Texture::Texture
	(
		pass_ref<Context> ctx,
		int width, int height,
		TextureFormat format, ElementType element_type,
		TextureFlags flags, TextureType type,
		std::optional<ColorRGBA> _border_color,
		bool _loose_internal_format
	)
		: Texture(ctx, ctx->generate_texture(width, height, format, element_type, (flags | TextureFlags::Dynamic), type, _border_color, true, _loose_internal_format), width, height, format, element_type, (flags | TextureFlags::Dynamic), type) {}

	Texture::Texture(pass_ref<Context> ctx, raw_string path, Flags flags, TextureType type)
		: Texture(ctx, PixelMap::Load(path), flags, type) {}

	Texture::Texture(pass_ref<Context> ctx, const std::string& path, Flags flags, TextureType type)
		: Texture(ctx, PixelMap::Load(path), flags, type)
	{
		#ifdef _DEBUG
			_path = path;
		#endif
	}

	Texture::~Texture()
	{
		get_context()->release_texture(std::move(handle));
	}

	void Texture::resize(int width, int height)
	{
		get_context()->resize_texture(*this, width, height); // , {}, false
	}

	void swap(Texture& x, Texture& y)
	{
		using std::swap;

		swap(static_cast<Resource&>(x), static_cast<Resource&>(y));

		swap(x.width, y.width);
		swap(x.height, y.height);

		swap(x.format, y.format);
		swap(x.element_type, y.element_type);
		swap(x.flags, y.flags);
	}
}