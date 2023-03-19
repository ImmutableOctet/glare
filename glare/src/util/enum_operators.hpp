#pragma once

#define FLAG_ENUM(BaseType, EnumType)                                                                          \
	inline BaseType operator& (EnumType a, EnumType b) { return (BaseType)((BaseType)a & (BaseType)b); }       \
	inline EnumType operator~ (EnumType a) { return (EnumType)~(BaseType)a; }                                  \
	inline EnumType operator| (EnumType a, EnumType b) { return (EnumType)((BaseType)a | (BaseType)b); }       \
	inline EnumType operator^ (EnumType a, EnumType b) { return (EnumType)((BaseType)a ^ (BaseType)b); }       \
	inline EnumType& operator|= (EnumType& a, EnumType b) { return (EnumType&)((BaseType&)a |= (BaseType)b); } \
	inline EnumType& operator&= (EnumType& a, EnumType b) { return (EnumType&)((BaseType&)a &= (BaseType)b); } \
	inline EnumType& operator^= (EnumType& a, EnumType b) { return (EnumType&)((BaseType&)a ^= (BaseType)b); }