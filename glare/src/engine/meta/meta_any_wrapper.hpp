#pragma once

#include "types.hpp"
#include "meta_any.hpp"

namespace engine
{
	struct MetaAnyWrapper : public MetaAny
	{
		MetaAnyWrapper() = default;
		MetaAnyWrapper(MetaAnyWrapper&& wrapper) noexcept = default;

		MetaAnyWrapper(MetaAny&& underlying_in);

		//MetaAnyWrapper(MetaAnyWrapper&& wrapper) noexcept;

		MetaAnyWrapper(const MetaAnyWrapper& wrapper);

		MetaAnyWrapper& operator=(MetaAnyWrapper&&) noexcept = default;

		MetaAnyWrapper& operator=(MetaAny&& new_underlying) noexcept;
		MetaAnyWrapper& operator=(const MetaAnyWrapper& wrapper);

		bool operator==(const MetaAnyWrapper&) const noexcept = default;
		bool operator!=(const MetaAnyWrapper&) const noexcept = default;

		[[nodiscard]] explicit operator bool() const noexcept;

		operator const MetaAny*() const;
		operator const MetaAny&() const;

		operator MetaAny() const;
		operator MetaAny&() const;
		operator MetaAny*() const;
	};
}