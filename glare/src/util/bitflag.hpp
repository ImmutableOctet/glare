#pragma once

#include <type_traits>
#include <bitset>

namespace util
{
	template<typename Enum, bool IsEnum = std::is_enum<Enum>::value>
    class bitflag;

    template<typename Enum>
    class bitflag<Enum, true>
    {
        public:
          constexpr const static int number_of_bits = std::numeric_limits<typename std::underlying_type<Enum>::type>::digits;

          constexpr bitflag() = default;
          constexpr bitflag(Enum value) : bits(1 << static_cast<std::size_t>(value)) {}
          constexpr bitflag(const bitflag& other) : bits(other.bits) {}

          constexpr operator Enum() const
          {
              /*
              auto* value = reinterpret_cast<Enum*>(bits);

              return *value;
              */

              auto value = bits.to_ulong();

              return static_cast<Enum>(value);
          }

          constexpr bitflag operator|(Enum value) const { bitflag result = *this; result.bits |= 1 << static_cast<std::size_t>(value); return result; }
          constexpr bitflag operator&(Enum value) const { bitflag result = *this; result.bits &= 1 << static_cast<std::size_t>(value); return result; }
          constexpr bitflag operator^(Enum value) const { bitflag result = *this; result.bits ^= 1 << static_cast<std::size_t>(value); return result; }
          constexpr bitflag operator~() const { bitflag result = *this; result.bits.flip(); return result; }

          constexpr bitflag& operator|=(Enum value) { bits |= 1 << static_cast<std::size_t>(value); return *this; }
          constexpr bitflag& operator&=(Enum value) { bits &= 1 << static_cast<std::size_t>(value); return *this; }
          constexpr bitflag& operator^=(Enum value) { bits ^= 1 << static_cast<std::size_t>(value); return *this; }

          constexpr bool any() const { return bits.any(); }
          constexpr bool all() const { return bits.all(); }
          constexpr bool none() const { return bits.none(); }
          constexpr explicit operator bool() { return any(); }

          constexpr bool test(Enum value) const { return bits.test(1 << static_cast<std::size_t>(value)); }
          constexpr void set(Enum value) { bits.set(1 << static_cast<std::size_t>(value)); }
          constexpr void unset(Enum value) { bits.reset(1 << static_cast<std::size_t>(value)); }

        private:
          std::bitset<number_of_bits> bits;
    };

    /*
    template<typename Enum>
    constexpr typename std::enable_if<std::is_enum<Enum>::value, bitflag<Enum>>::type operator|(Enum left, Enum right)
    {
      return bitflag<Enum>(left) | right;
    }
    template<typename Enum>
    constexpr typename std::enable_if<std::is_enum<Enum>::value, bitflag<Enum>>::type operator&(Enum left, Enum right)
    {
      return bitflag<Enum>(left) & right;
    }
    template<typename Enum>
    constexpr typename std::enable_if_t<std::is_enum<Enum>::value, bitflag<Enum>>::type operator^(Enum left, Enum right)
    {
      return bitflag<Enum>(left) ^ right;
    }
    */
}