#include "meta_any_wrapper.hpp"

namespace engine
{
	MetaAnyWrapper::MetaAnyWrapper(MetaAny&& underlying_in)
		: MetaAny(std::move(underlying_in)) // underlying_in.as_ref()
	{
		//assert(!underlying_in.owner());
	}

	//MetaAnyWrapper::MetaAnyWrapper(MetaAnyWrapper&& wrapper) noexcept
	//	: MetaAny(wrapper.underlying.as_ref()) {} // std::move(wrapper.underlying)

	MetaAnyWrapper::MetaAnyWrapper(const MetaAnyWrapper& wrapper)
		: MetaAny(wrapper.as_ref()) {}

	MetaAnyWrapper& MetaAnyWrapper::operator=(MetaAny&& new_underlying) noexcept
	{
		static_cast<MetaAny&>(*this) = std::move(new_underlying); // new_underlying.as_ref();

		return *this;
	}

	MetaAnyWrapper& MetaAnyWrapper::operator=(const MetaAnyWrapper& wrapper)
	{
		static_cast<MetaAny&>(*this) = wrapper.as_ref();

		return *this;
	}

	MetaAnyWrapper::operator bool() const noexcept
	{
		return MetaAny::operator bool();
	}

	MetaAnyWrapper::operator const MetaAny*() const
	{
		//return reinterpret_cast<const MetaAny*>(this);
		return static_cast<const MetaAny*>(this);
	}

	MetaAnyWrapper::operator const MetaAny&() const
	{
		return *(static_cast<const MetaAny*>(this));
	}

	MetaAnyWrapper::operator MetaAny() const
	{
		//return underlying.as_ref();

		// NOTE: Const-cast used here to avoid transfer of
		// const qualification to underlying reference.
		return const_cast<MetaAnyWrapper*>(this)->as_ref();
	}

	MetaAnyWrapper::operator MetaAny&() const // const MetaAny&
	{
		// NOTE: Const-cast used here to avoid transfer of
		// const qualification to underlying reference.
		return *const_cast<MetaAnyWrapper*>(this);
	}

	MetaAnyWrapper::operator MetaAny*() const
	{
		// NOTE: Const-cast used here to avoid transfer of
		// const qualification to underlying reference.
		return const_cast<MetaAny*>(static_cast<const MetaAny*>(this));
	}
}