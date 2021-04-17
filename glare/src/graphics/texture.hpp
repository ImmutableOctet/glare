#pragma once

#include <string>
#include <type_traits>
#include <optional>

#include <debug.hpp>

#include "types.hpp"
#include "resource.hpp"

namespace graphics
{
	class Context;
	class ContextState;

	class PixelMap;

	class Texture : public Resource
	{
		private:
#if _DEBUG
			// Debugging related:
			std::string _path;
#endif
		protected:
			int width, height;

			TextureFormat format;
			ElementType element_type;
			TextureFlags flags;
			TextureType type;
		public:
			friend Context;
			friend ContextState;

			using Flags = TextureFlags;

			friend void swap(Texture& x, Texture& y);

			Texture(pass_ref<Context> ctx, raw_string path, Flags flags=Flags::Default, TextureType type=TextureType::Default);
			Texture(pass_ref<Context> ctx, const std::string& path, Flags flags=Flags::Default, TextureType type=TextureType::Default);
			Texture(pass_ref<Context> ctx, const PixelMap& data, Flags flags=Flags::Default, TextureType type=TextureType::Default);

			// TextureFlags::Dynamic is automatically applied internally.
			Texture(pass_ref<Context> ctx, int width, int height, TextureFormat format, ElementType element_type, TextureFlags flags=Flags::None, TextureType type=TextureType::Default, std::optional<ColorRGBA> _border_color=std::nullopt);
			
			Texture(Texture&& texture) : Texture() { swap(*this, texture); }

			inline Texture() : Texture({}, {}, 0, 0, TextureFormat::Unknown, ElementType::Unknown, TextureFlags::None, TextureType::Default) {}

			Texture(const Texture&) = delete;

			~Texture();

			inline Texture& operator=(Texture texture)
			{
				swap(*this, texture);

				return *this;
			}

			inline int get_width()  const { return width;  }
			inline int get_height() const { return height; }

			inline PointRect rect() const // Viewport
			{
				return PointRect{ {}, { width, height } };
			}

			void resize(int width, int height);

			inline TextureFormat get_format()       const { return format;       }
			inline ElementType   get_element_type() const { return element_type; }
			inline TextureFlags  get_flags()        const { return flags;        }
			inline TextureType   get_type()         const { return type;         }

			inline bool is_dynamic() const { return (flags & TextureFlags::Dynamic); }
		protected:
			Texture(weak_ref<Context> ctx, ContextHandle&& resource_handle, int width, int height, TextureFormat format, ElementType element_type, TextureFlags flags, TextureType type);
	};
}