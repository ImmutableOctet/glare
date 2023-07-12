#pragma once

#include "string_binary_format.hpp"

#include <limits>
#include <bit>

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
			None                 = static_cast<FormatRaw>(0),

			// Initial bit is reserved.
			Reserved             = static_cast<FormatRaw>(1 << 0),

			// This flag controls whether big-endian byte order is used to store integral and floating-point types.
			BigEndian            = static_cast<FormatRaw>(1 << 1),

			/*
				This flag indicates if trivial copying of data is allowed.
				(Requires unannotated data to be successful)
				
				NOTES:

				* This flag may only be enabled if the memory layout of
				serialized objects matches the intended in-memory representation.
				(i.e. padding/alignment matches 1:1)

				* Always take note of possible side-effects when targeting multiple platforms.
				
				Whether trivial copying is performed is implementation-defined. (see `can_trivially_copy`)
			*/
			AllowTrivialCopy     = static_cast<FormatRaw>(1 << 2),

			// This flag controls whether objects have a standard format header prior to their value segment.
			// This is useful for control-flow when processing data members.
			StandardHeader       = static_cast<FormatRaw>(1 << 3),

			// The type ID header specifies the identifier of the encoded object's intended type.
			TypeIDHeader         = static_cast<FormatRaw>(1 << 4),

			// This flag controls whether the type identifiers of member-values are encoded prior to the value itself.
			// e.g. '[Count: 2][Member][Type][Value][Member][Type][Value]' vs. '[Count: 2][Member][Value][Member][Value]', etc.
			MemberTypes          = static_cast<FormatRaw>(1 << 5),

			// The length header is used to specify the length of the encoded object's segment.
			LengthHeader         = static_cast<FormatRaw>(1 << 6),

			// This flag controls whether a length is encoded for each member entry.
			MemberLengths        = static_cast<FormatRaw>(1 << 7),

			// This flag controls whether a count is encoded that
			// indicates the number of members in the following sequence.
			CountMembers         = static_cast<FormatRaw>(1 << 8),

			// This flag controls whether the identifiers of members are encoded prior to their value.
			MemberNames          = static_cast<FormatRaw>(1 << 9),

			// This flag controls whether the sequence of members includes read-only values. (Useful for debugging)
			ReadOnlyMembers      = static_cast<FormatRaw>(1 << 10),

			// All bits enabled.
			// 
			// NOTE: Exact behavior of this format may differ
			// between engine versions; use at your own risk.
			All = (std::numeric_limits<FormatRaw>::max() & (~(Reserved|BigEndian|AllowTrivialCopy)))
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

		// A format filter where the only enabled flags are those that indicate nested annotations.
		inline static constexpr Format annotation_filter()
		{
			return static_cast<Format>(Format::MemberTypes | Format::MemberLengths | Format::MemberNames | Format::CountMembers); // | Format::ReadOnlyMembers
		}

		// Binary format version indicator.
		FormatVersion format_version = 1;

		// Bitfield describing the binary format.
		Format format = default_format;

		// String encoding used by the binary format.
		StringBinaryFormat string_format = StringBinaryFormat::Default;

		inline bool big_endian() const
		{
			return get_flag(Format::BigEndian);
		}

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

		// See notes for `is_unannotated`.
		inline bool is_annotated() const
		{
			return ((format & annotation_filter()) > 0);
		}

		// An object is unannotated when there are no other formatting elements in the binary stream that would prevent a 1:1 copy of the data.
		// Typically, these are elements like type and member identifiers, which make it impossible to load the data in a single operation.
		inline bool is_unannotated() const
		{
			return (!is_annotated());
		}

		// This member-function indicates if trivial copying of objects is allowed.
		// 
		// NOTE: Only types that support trivial copying may benefit from this optimization.
		// 
		// Regardless of this indicator, implementations are free to forgo
		// trivial copying for any reason, including safety and security concerns.
		inline bool can_trivially_copy() const
		{
			return
			(
				((std::endian::native == std::endian::big) || (!big_endian()))
				&&
				(get_flag(Format::AllowTrivialCopy)) // (allow_trivial_copy())
				&&
				(is_unannotated())
			);
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