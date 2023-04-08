#pragma once

#include "types.hpp"

#include <util/variant.hpp>

#include <variant>
#include <type_traits>
#include <utility>

namespace engine
{
	template <typename VariantType>
    struct WrappedVariant
    {
        using type = VariantType;

        static auto from_meta_any(MetaAny opaque_value, bool allow_conversion=true)
            -> std::conditional_t
            <
                util::variant_contains_v<VariantType, std::monostate>,
                WrappedVariant,
                std::optional<WrappedVariant>
            >
        {
            if constexpr (util::variant_contains_v<VariantType, std::monostate>) // (std::is_default_constructible_v<VariantType>)
            {
                return WrappedVariant { std::move(opaque_value), allow_conversion };
            }
            else
            {
                std::optional<WrappedVariant> output = std::nullopt;

                if (opaque_value)
                {
                    util::for_each_variant_type<VariantType>
			        (
				        [&output, &opaque_value]<typename T>()
				        {
					        if (output)
					        {
						        return;
					        }

					        if (auto exact_type = opaque_value.try_cast<T>())
					        {
						        output = WrappedVariant { VariantType { std::move(*exact_type) } }; // *exact_type;
					        }
				        }
			        );

                    if (allow_conversion && (!output))
                    {
                        util::for_each_variant_type<VariantType>
                        (
                            [&output, &opaque_value]<typename T>()
                            {
                                if (output)
                                {
                                    return;
                                }

                                if (opaque_value.allow_cast<T>())
                                {
                                    output = WrappedVariant { VariantType { opaque_value.cast<T>() } };
                                }
                            }
                        );
                    }
                }

                return output;
            }
        }

        template <typename T>
        WrappedVariant from_type(T&& value)
        {
            static_assert(util::variant_contains_v<VariantType, std::decay_t<T>>, "The type specified must be a valid variant option");
            //static_assert(util::variant_contains_v<VariantType, T>, "The type specified must be a valid variant option");

            return WrappedVariant { std::forward<T>(value) };
        }

        //template <typename=std::enable_if_t<std::is_default_constructible_v<VariantType>>>
        WrappedVariant() = default;

        //template <typename=std::enable_if_t<std::is_copy_constructible_v<VariantType>>>
        WrappedVariant(const WrappedVariant&) = default;

        //template <typename=std::enable_if_t<std::is_move_constructible_v<VariantType>>>
        WrappedVariant(WrappedVariant&&) noexcept = default;

        template
        <
            typename RawValueType,

            typename=std::enable_if_t
            <
                (
                    (!std::is_same_v<std::decay_t<RawValueType>, MetaAny>)
                    &&
                    (
                        (std::is_same_v<VariantType, std::decay_t<RawValueType>>)
                        ||
                        //(util::variant_contains_v<VariantType, std::remove_reference<RawValueType>>) // std::decay_t<RawValueType> // RawValueType
                        std::is_constructible_v<VariantType, RawValueType>
                    )
                )
            >
        >
        WrappedVariant(RawValueType&& raw_value)
            : value(std::forward<RawValueType>(raw_value)) {}
        
        //WrappedVariant(std::enable_if_t<util::variant_contains_v<VariantType, std::monostate>, std::monostate> raw_value)
        //    : value(std::move(raw_value)) {}

        template
        <
            typename OpaqueValueType,
            
            typename=std::enable_if_t
            <
                (
                    (std::is_same_v<std::decay_t<OpaqueValueType>, MetaAny>)
                    &&
                    (util::variant_contains_v<VariantType, std::monostate>) // std::is_default_constructible_v<VariantType>,
                )
            >
        >
        WrappedVariant
        (
            OpaqueValueType opaque_value,
            bool allow_conversion=true
        )
            : value(std::monostate {})
		{
			if (!opaque_value)
			{
				return;
			}

            bool match_found = false;

			util::for_each_variant_type<VariantType>
			(
				[this, &opaque_value, &match_found]<typename T>()
				{
					if (match_found) // (static_cast<bool>(*this))
					{
						return;
					}

					if (auto exact_type = opaque_value.try_cast<T>())
					{
						this->value = std::move(*exact_type); // *exact_type;

                        match_found = true;
					}
				}
			);

            if ((allow_conversion) && (!match_found))// (!static_cast<bool>(*this))
            {
                util::for_each_variant_type<VariantType>
                (
                    [this, &opaque_value, &match_found]<typename T>()
                    {
                        if (match_found) // (static_cast<bool>(*this))
                        {
                            return;
                        }

                        if (opaque_value.allow_cast<T>())
                        {
                            this->value = VariantType { opaque_value.cast<T>() };

                            match_found = true;
                        }
                    }
                );
            }
		}

        WrappedVariant& operator=(const WrappedVariant&) = default;
        WrappedVariant& operator=(WrappedVariant&&) noexcept = default;

        template
        <
            typename ValueType,

            typename=std::enable_if_t
            <
                (
                    (std::is_assignable_v<VariantType, ValueType>)
                    //(std::is_copy_assignable_v<> || std::is_move_assignable_v<VariantType>)
                    &&
                    (
                        (std::is_same_v<VariantType, std::decay_t<ValueType>>)
                        ||
                        (util::variant_contains_v<VariantType, std::decay_t<ValueType>>)
                    )
                )
            >
        >
        WrappedVariant& operator=(ValueType&& value) noexcept
        {
            this->value = std::forward<ValueType>(value);

            return *this;
        }

        constexpr explicit operator bool() const
        {
            return exists();
        }

        constexpr operator const VariantType&() const
        {
            return value;
        }

        constexpr operator VariantType&()
        {
            return value;
        }

        auto operator<=>(const WrappedVariant&) const = default;

        constexpr bool exists() const
        {
            if constexpr (util::variant_contains_v<VariantType, std::monostate>) // std::is_default_constructible_v<VariantType>
            {
                return ((!value.valueless_by_exception()) && (type_index() != util::variant_index<VariantType, std::monostate>()));
            }
            else
            {
                return (!value.valueless_by_exception()); // true;
            }
        }

        constexpr bool empty() const
        {
            return (!exists());
        }

        const VariantType& get_value() const
        {
            return value;
        }

        constexpr VariantType& get_value()
        {
            return value;
        }

        template <typename T>
        constexpr decltype(auto) get()
        {
            return std::get<T>(value);
        }

        template <typename T>
        constexpr decltype(auto) get() const
        {
            return std::get<T>(value);
        }

        constexpr std::size_t type_index() const
        {
            return value.index();
        }

        // TODO: Determine if this should be made private/protected.
        VariantType value;
    };

    template <typename... Ts>
    using VariantWrapper = WrappedVariant<std::variant<Ts...>>;
}