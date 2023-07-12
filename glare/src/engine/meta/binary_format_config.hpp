#pragma once

#include "string_binary_format.hpp"

#include <limits>

#include <cstdint>

namespace engine
{
	// Used to control how a binary format is specified.
	// 
	// This structure acts as a literal representation of the object
	// description header used for binary object encoding.
	struct BinaryFormatConfig
	{
		using LengthType = std::uint32_t; // std::uint16_t;
		using SmallLengthType = std::uint16_t; // std::uint8_t;

		using FormatVersion = std::uint16_t; // std::uint8_t;
		using FormatRaw = std::uint16_t;

		// Bitfield enumeration indicating which elements are included in the binary sequence.
		enum Format : FormatRaw // enum class
		{
			// No format specification.
			None                 = 0,

			// Initial bit is reserved.
			Reserved             = static_cast<FormatRaw>(1 << 0),

			// This flag controls whether objects have a standard format header prior to their value segment.
			// This is useful for control-flow when processing data members.
			StandardHeader       = static_cast<FormatRaw>(1 << 1),

			// The type ID header specifies the identifier of the encoded object's intended type.
			TypeIDHeader         = static_cast<FormatRaw>(1 << 2),

			// This flag controls whether the type identifiers of member-values are encoded prior to the value itself.
			// e.g. '[Count: 2][Member][Type][Value][Member][Type][Value]' vs. '[Count: 2][Member][Value][Member][Value]', etc.
			MemberTypes          = static_cast<FormatRaw>(1 << 3),

			// The length header is used to specify the length of the encoded object's segment.
			LengthHeader         = static_cast<FormatRaw>(1 << 4),

			// This flag controls whether a length is encoded for each member entry.
			MemberLengths        = static_cast<FormatRaw>(1 << 5),

			// This flag controls whether a count is encoded that
			// indicates the number of members in the following sequence.
			CountMembers         = static_cast<FormatRaw>(1 << 6),

			// This flag controls whether the identifiers of members are encoded prior to their value.
			MemberNames          = static_cast<FormatRaw>(1 << 7),

			// This flag controls whether the sequence of members includes read-only values. (Useful for debugging)
			ReadOnlyMembers      = static_cast<FormatRaw>(1 << 8),

			// All bits enabled.
			// 
			// NOTE: Exact behavior of this format may differ
			// between engine versions; use at your own risk.
			All = std::numeric_limits<FormatRaw>::max()
		};

		// Default format specifications:
		inline static constexpr Format format_v1 = static_cast<Format>(Format::StandardHeader | Format::LengthHeader | Format::TypeIDHeader);

		inline static constexpr Format default_format = format_v1;

		// Used to indicate that any format version can be accepted.
		inline static constexpr FormatVersion any_format_version = {};

		inline static constexpr BinaryFormatConfig any_format()
		{
			return { any_format_version, default_format };
		}

		// Binary format version indicator.
		FormatVersion format_version = 1;

		// Bitfield describing the binary format.
		Format format = default_format;

		// String encoding used by the binary format.
		StringBinaryFormat string_format = StringBinaryFormat::Default;

		inline bool standard_header() const
		{
			return get_flag(Format::StandardHeader);
		}

		inline bool type_id_header() const
		{
			return get_flag(Format::TypeIDHeader);
		}

		inline bool member_types() const
		{
			return get_flag(Format::MemberTypes);
		}

		inline bool length_header() const
		{
			return get_flag(Format::LengthHeader);
		}

		inline bool member_lengths() const
		{
			return get_flag(Format::MemberLengths);
		}

		inline bool count_members() const
		{
			return get_flag(Format::CountMembers);
		}

		inline bool member_ids() const
		{
			return get_flag(Format::MemberNames);
		}

		inline bool read_only_members() const
		{
			return get_flag(Format::ReadOnlyMembers);
		}

		inline BinaryFormatConfig& set_flag(Format flag, bool value)
		{
			if (value)
			{
				format = static_cast<Format>(format | flag);
			}
			else
			{
				format = static_cast<Format>(format & ~flag);
			}

			return *this;
		}

		inline bool get_flag(Format flag) const
		{
			return ((format & flag) > 0);
		}

		inline BinaryFormatConfig decay() const
		{
			auto decayed_config = BinaryFormatConfig { *this };

			decayed_config.set_flag(Format::StandardHeader, false);

			decayed_config.set_flag(Format::TypeIDHeader, get_flag(Format::MemberTypes));
			decayed_config.set_flag(Format::LengthHeader, get_flag(Format::MemberLengths));

			return decayed_config;
		}
	};
}