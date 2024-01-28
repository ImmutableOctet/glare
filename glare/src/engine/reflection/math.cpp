#include "math.hpp"

#include "math_extensions.hpp"
#include "common_extensions.hpp"
#include "tuple_extensions.hpp"
#include "string_conversion.hpp"

#include <engine/meta/hash.hpp>

#include <math/math.hpp>
#include <math/reflection.hpp>
#include <math/lerp.hpp>
#include <math/surface.hpp>
#include <math/format.hpp>
#include <math/types.hpp>
#include <math/joyhat.hpp>
#include <math/oscillate.hpp>

#include <util/lambda.hpp>

#include <type_traits>
#include <cmath>

namespace engine
{
    namespace impl
    {
        static math::TransformVectors transform_vectors_from_position_and_rotation(const math::Vector3D& position={}, const math::Vector3D& rotation = {})
        {
            return { position, rotation, { 1.0f, 1.0f, 1.0f } };
        }

        static math::TransformVectors transform_vectors_from_position(const math::Vector3D& position={})
        {
            return transform_vectors_from_position_and_rotation(position);
        }

	    static math::Vector3D transform_vectors_get_by_index(const math::TransformVectors& instance, std::size_t index) // -> decltype(auto)
	    {
            switch (index)
            {
                case 0:
                    return std::get<0>(instance);
                case 1:
                    return std::get<1>(instance);
                case 2:
                    return std::get<2>(instance);
            }
		    
            return {};
	    }
    }

	template
    <
        typename T,
        
        bool generate_optional_type=true,
        bool generate_operators=true,
        bool generate_standard_methods=true
    >
    auto reflect_math_type(auto type_name, bool sync_context=true)
    {
        if (sync_context)
        {
            // Ensure that we're using the correct context.
            sync_reflection_context();
        }

        auto type = math::reflect<T>(hash(type_name));

        if constexpr (generate_operators)
        {
            type = type
                .func<&impl::add_impl<T>>("operator+"_hs)
                .func<&impl::subtract_impl<T>>("operator-"_hs)
                .func<&impl::multiply_impl<T>>("operator*"_hs)
            ;
        }

        // NOTE: This check doesn't work due to non-type template parameters for `glm::vec`.
        // TODO: Revisit the idea of checking for vector types.
        //constexpr auto is_vector_type = (util::is_specialization_v<T, glm::vec>)
        constexpr auto is_vector_type = (std::is_same_v<T, math::Vector2D> || std::is_same_v<T, math::Vector3D> || std::is_same_v<T, math::Vector4D>); // std::is_same_v<T, math::vec2i>
        
        if constexpr (is_vector_type)
        {
            if constexpr (generate_operators)
            {
                type = type
                    .func<&impl::vector_operator_unary_plus_impl<T>>("+operator"_hs)
                    .func<&impl::vector_operator_unary_minus_impl<T>>("-operator"_hs)

                    // NOTE: Same-type division is not supported by quaternions, only vectors.
                    .func<&impl::divide_impl<T>>("operator/"_hs)

                    .func<&impl::vector_operator_index_subscript_impl<T>>("operator[]"_hs)
                    .func<&impl::vector_operator_string_view_subscript_impl<T>>("operator[]"_hs)
                    .func<&impl::vector_operator_string_subscript_impl<T>>("operator[]"_hs)
                ;
            }
        }

        if constexpr (is_vector_type || std::is_same_v<T, math::Quaternion>)
        {
            if constexpr (generate_operators)
            {
                type = type
                    .func<&impl::multiply_impl<T, float>>("operator*"_hs)
                ;
            }

            type = type.conv<&impl::format_string_conv<T>>();

            if constexpr (generate_standard_methods)
            {
                using value_type = typename T::value_type;

                if constexpr (std::is_same_v<value_type, float>)
                {
                    type = type
                        .func<&impl::normalize_impl<T>>("normalize"_hs)
                    ;
                }
            }
        }

        if constexpr (generate_optional_type)
        {
            auto opt_type = optional_custom_meta_type<T>(type_name);
        }

        return type;
    }

    // Reflects `math::Vector2D` with the generalized name of `Vector2D`.
    // 
    // TODO: Look into migrating this to another file/header.
    template <>
    void reflect<math::Vector2D>()
    {
        reflect_math_type<math::Vector2D>("Vector2D");
    }

    // Reflects `math::Vector3D` with the generalized name of `Vector`.
    // 
    // TODO: Look into migrating this to another file/header.
    template <>
    void reflect<math::Vector3D>()
    {
        reflect_math_type<math::Vector3D>("Vector") // "Vector3D"
            .ctor<&impl::vec3_from_vec2>()
        ;
    }

    // Reflects `math::Vector4D` with the generalized name of `Vector4D`.
    // 
    // TODO: Look into migrating this to another file/header.
    template <>
    void reflect<math::Vector4D>()
    {
        reflect_math_type<math::Vector4D>("Vector4D")
            .ctor<&impl::vec4_from_vec2>()
            .ctor<&impl::vec4_from_vec3>()
        ;
    }

    // Reflects `math::vec2i` with the generalized name of `vec2i`.
    // 
    // TODO: Look into migrating this to another file/header.
    template <>
    void reflect<math::vec2i>()
    {
        reflect_math_type<math::vec2i>("vec2i");
    }

    template <>
    void reflect<math::Matrix4x4>()
    {
        reflect_math_type<math::Matrix4x4>("Matrix") // "Matrix4x4"
            .func<&impl::mat4_vec4_multiply_impl>("operator*"_hs)
            .func<&impl::mat4_vec3_multiply_impl>("operator*"_hs)
            .func<&impl::mat_quat_multiply_impl<math::Matrix4x4>>("operator*"_hs)
            .func<&math::identity<math::Matrix4x4>>("identity"_hs)
            .func<&impl::inverse_impl<math::Matrix4x4>>("inverse"_hs)
            .func<&impl::transpose_impl<math::Matrix4x4>>("transpose"_hs)
            .func<&impl::mat4_translate_impl>("translate"_hs)
        ;
    }

    template <>
    void reflect<math::Matrix3x3>()
    {
        reflect_math_type<math::Matrix3x3>("Matrix3x3")
            .func<&impl::multiply_impl<math::Matrix3x3, math::Vector3D>>("operator*"_hs)
            .func<&impl::mat_quat_multiply_impl<math::Matrix3x3>>("operator*"_hs)
            .func<&math::identity<math::Matrix3x3>>("identity"_hs)
            .func<&impl::inverse_impl<math::Matrix3x3>>("inverse"_hs)
            .func<&impl::transpose_impl<math::Matrix3x3>>("transpose"_hs)
        ;
    }

    template <>
    void reflect<math::Quaternion>()
    {
        reflect_math_type<math::Quaternion>("Quaternion")
            .func<&impl::quat_vec3_multiply_impl>("operator*"_hs)
            .func<&impl::quat_vec4_multiply_impl>("operator*"_hs)
            .func<&impl::inverse_impl<math::Quaternion>>("inverse"_hs)
        ;
    }

    // NOTE: `math::TransformVectors` is currently an alias to an `std::tuple` specialization.
    template <>
    void reflect<math::TransformVectors>()
    {
        reflect_math_type<math::TransformVectors, true, false, false>("TransformVectors")
            .ctor
            <
                std::tuple_element_t<0, math::TransformVectors>,
                std::tuple_element_t<1, math::TransformVectors>,
                std::tuple_element_t<2, math::TransformVectors>
            >()

            .ctor<&impl::transform_vectors_from_position>()
            .ctor<&impl::transform_vectors_from_position_and_rotation>()

            .data<&impl::set_tuple_element<0, math::TransformVectors>, &impl::get_tuple_element<0, math::TransformVectors>>("position"_hs)
            .data<&impl::set_tuple_element<1, math::TransformVectors>, &impl::get_tuple_element<1, math::TransformVectors>>("rotation"_hs)
            .data<&impl::set_tuple_element<2, math::TransformVectors>, &impl::get_tuple_element<2, math::TransformVectors>>("scale"_hs)

            .func<&impl::transform_vectors_get_by_index>("operator[]"_hs)
        ;
    }

    // Wraps math free-functions from the C/C++ standard library.
    template <typename T, bool special_functions=true>
    static auto reflect_cmath(auto math_type)
    {
        math_type = math_type
            .func<+[](T value) { return std::abs(value); }>("abs"_hs)
            .func<+[](T x, T y) { return std::remainder(x, y); }>("remainder"_hs)

            // Disabled in favor of `math` module's implementation.
            //.func<+[](T a, T b, T t) { return std::lerp(a, b, t); }>("lerp"_hs)

            .func<+[](T value) { return std::exp(value); }>("exp"_hs)
            .func<+[](T value) { return std::exp2(value); }>("exp2"_hs)
            .func<+[](T value) { return std::expm1(value); }>("expm1"_hs)
            .func<+[](T value) { return std::log(value); }>("log"_hs)
            .func<+[](T value) { return std::log10(value); }>("log10"_hs)
            .func<+[](T value) { return std::log2(value); }>("log2"_hs)
            .func<+[](T value) { return std::log1p(value); }>("log1p"_hs)

            .func<+[](T x, T y) { return std::pow(x, y); }>("pow"_hs)
            .func<+[](T value) { return std::sqrt(value); }>("sqrt"_hs)
            .func<+[](T value) { return std::cbrt(value); }>("cbrt"_hs)
            .func<+[](T x, T y) { return std::hypot(x, y); }>("hypot"_hs)
            .func<+[](T x, T y, T z) { return std::hypot(x, y, z); }>("hypot"_hs)

            .func<+[](T value) { return std::sin(value); }>("sin"_hs)
            .func<+[](T value) { return std::cos(value); }>("cos"_hs)
            .func<+[](T value) { return std::tan(value); }>("tan"_hs)
            .func<+[](T value) { return std::asin(value); }>("asin"_hs)
            .func<+[](T value) { return std::acos(value); }>("acos"_hs)
            .func<+[](T value) { return std::atan(value); }>("atan"_hs)
            .func<+[](T x, T y) { return std::atan2(x, y); }>("atan2"_hs)

            .func<+[](T value) { return std::sinh(value); }>("sinh"_hs)
            .func<+[](T value) { return std::cosh(value); }>("cosh"_hs)
            .func<+[](T value) { return std::tanh(value); }>("tanh"_hs)
            .func<+[](T value) { return std::asinh(value); }>("asinh"_hs)
            .func<+[](T value) { return std::acosh(value); }>("acosh"_hs)
            .func<+[](T value) { return std::atanh(value); }>("atanh"_hs)

            .func<+[](T value) { return std::erf(value); }>("erf"_hs)
            .func<+[](T value) { return std::erfc(value); }>("erfc"_hs)
            .func<+[](T value) { return std::tgamma(value); }>("tgamma"_hs)
            .func<+[](T value) { return std::lgamma(value); }>("lgamma"_hs)

            .func<+[](T value) { return std::ceil(value); }>("ceil"_hs)
            .func<+[](T value) { return std::floor(value); }>("floor"_hs)
            .func<+[](T value) { return std::trunc(value); }>("trunc"_hs)
            .func<+[](T value) { return std::round(value); }>("round"_hs)
            .func<+[](T value) { return std::nearbyint(value); }>("nearby_int"_hs)
            .func<+[](T value) { return std::rint(value); }>("rint"_hs)

            .func<+[](T value, int exp) { return std::ldexp(value, exp); }>("ldexp"_hs)
            .func<+[](T value, int exp) { return std::scalbn(value, exp); }>("scalbn"_hs)
            .func<+[](T value) { return std::ilogb(value); }>("ilogb"_hs)
            .func<+[](T value) { return std::logb(value); }>("logb"_hs)
            .func<+[](T from, T to) { return std::nextafter(from, to); }>("next_after"_hs)
            .func<+[](T mag, T sgn) { return std::copysign(mag, sgn); }>("copysign"_hs)

            //.func<+[](T value) { return std::signbit(value); }>("signbit"_hs)

            // Workaround for missing integral overloads for `std::signbit`. (MSVC issue?)
            .func
            <
                util::lambda_as_function_ptr
                <
                    [](T value)
                    {
                        if constexpr (std::is_floating_point_v<T>)
                        {
                            return std::signbit(value);
                        }
                        else if constexpr (std::is_same_v<T, std::int64_t>)
                        {
                            return std::signbit(static_cast<double>(value));
                        }
                        else
                        {
                            return std::signbit(static_cast<float>(value));
                        }

                        //return math::sign(value);
                    }
                >()
            >("signbit"_hs)

            .func<+[](T x, T y) { return std::isunordered(x, y); }>("is_unordered"_hs)
        ;

        if constexpr (special_functions)
        {
            math_type = math_type
                .func<+[](unsigned int n, unsigned int m, T x) { return std::assoc_laguerre(n, m, x); }>("assoc_laguerre"_hs)
                .func<+[](unsigned int n, unsigned int m, T x) { return std::assoc_legendre(n, m, x); }>("assoc_legendre"_hs)
                .func<+[](T x, T y) { return std::beta(x, y); }>("beta"_hs)
                .func<+[](T value) { return std::comp_ellint_1(value); }>("comp_ellint_1"_hs)
                .func<+[](T value) { return std::comp_ellint_2(value); }>("comp_ellint_2"_hs)
                .func<+[](T k, T nu) { return std::comp_ellint_3(k, nu); }>("comp_ellint_3"_hs)
                .func<+[](T nu, T x) { return std::cyl_bessel_i(nu, x); }>("cyl_bessel_i"_hs)
                .func<+[](T nu, T x) { return std::cyl_bessel_j(nu, x); }>("cyl_bessel_j"_hs)
                .func<+[](T nu, T x) { return std::cyl_bessel_k(nu, x); }>("cyl_bessel_k"_hs)
                .func<+[](T nu, T x) { return std::cyl_neumann(nu, x); }>("cyl_neumann"_hs)
                .func<+[](T k, T phi) { return std::ellint_1(k, phi); }>("ellint_1"_hs)
                .func<+[](T k, T phi) { return std::ellint_2(k, phi); }>("ellint_2"_hs)
                .func<+[](T k, T nu, T phi) { return std::ellint_3(k, nu, phi); }>("ellint_3"_hs)
                .func<+[](T value) { return std::expint(value); }>("expint"_hs)
                .func<+[](unsigned n, T x) { return std::hermite(n, x); }>("hermite"_hs)
                .func<+[](unsigned n, T x) { return std::legendre(n, x); }>("legendre"_hs)
                .func<+[](unsigned int n, T x) { return std::laguerre(n, x); }>("laguerre"_hs)
                .func<+[](T value) { return std::riemann_zeta(value); }>("riemann_zeta"_hs)
                .func<+[](unsigned int n, T x) { return std::sph_bessel(n, x); }>("sph_bessel"_hs)
                .func<+[](unsigned l, unsigned m, T theta) { return std::sph_legendre(l, m, theta); }>("sph_legendre"_hs)
                .func<+[](unsigned n, T x) { return std::sph_neumann(n, x); }>("sph_neumann"_hs)
            ;
        }

        if constexpr (std::is_floating_point_v<T>)
        {
            // Floating-point specific functions.
            math_type = math_type
                .func<+[](T value) { return std::isfinite(value); }>("is_finite"_hs)
                .func<+[](T value) { return std::isinf(value); }>("is_infinite"_hs)
                .func<+[](T value) { return std::isnormal(value); }>("is_normal"_hs)
                .func<+[](T value) { return std::isnan(value); }>("is_nan"_hs)
            ;
        }

        return math_type;
    }

    auto reflect_engine_math_functions(auto math_type)
    {
        // `math:constants`:
        math_type = math_type
            .prop("Pi"_hs, math::Pi)
            .prop("Tau"_hs, math::Tau)
            
            .func<&math::_Pi<float>>("Pi"_hs)
            .func<&math::_Tau<float>>("Tau"_hs)
        ;

        // `math:common`:
        math_type = math_type
            /*
            // Disabled for now. (Due to ICE on MSVC compiler)
            .func<static_cast<int(*)(float)>(&math::sign<float>)>("sign"_hs)
            .func<static_cast<int(*)(double)>(&math::sign<double>)>("sign"_hs)
            .func<static_cast<int(*)(std::int32_t)>(&math::sign<std::int32_t>)>("sign"_hs)
            .func<static_cast<int(*)(std::int64_t)>(&math::sign<std::int64_t>)>("sign"_hs)

            .func<static_cast<int(*)(const float&, const float&)>(&math::sign<float>)>("sign"_hs)
            .func<static_cast<int(*)(const double&, const double&)>(&math::sign<double>)>("sign"_hs)
            .func<static_cast<int(*)(const std::int32_t&, const std::int32_t&)>(&math::sign<std::int32_t>)>("sign"_hs)
            .func<static_cast<int(*)(const std::int64_t&, const std::int64_t&)>(&math::sign<std::int64_t>)>("sign"_hs)
            */

            // GLM:

            // Dot product:
            .func<+[](float x, float y) { return math::dot(x, y); }>("dot"_hs)
            .func<+[](const math::Quaternion& x, const math::Quaternion& y) { return math::dot(x, y); }>("dot"_hs)
            .func<+[](const math::Vector4D& x, const math::Vector4D& y) { return math::dot(x, y); }>("dot"_hs)
            .func<+[](const math::Vector2D& x, const math::Vector2D& y) { return math::dot(x, y); }>("dot"_hs)
            .func<+[](const math::Vector3D& x, const math::Vector3D& y) { return math::dot(x, y); }>("dot"_hs)

            // Cross product:
            .func<+[](const math::Vector3D& x, const math::Vector3D& y) { return math::cross(x, y); }>("cross"_hs)
            .func<+[](const math::Quaternion& q, const math::Vector3D& v) { return math::cross(q, v); }>("cross"_hs)
            .func<+[](const math::Vector3D& v, const math::Quaternion& q) { return math::cross(v, q); }>("cross"_hs)
            .func<+[](const math::Quaternion& q1, const math::Quaternion& q2) { return math::cross(q1, q2); }>("cross"_hs)

            // Normalize:
            .func<+[](const math::Vector3D& v) { return math::normalize(v); }>("normalize"_hs)
            .func<+[](const math::Quaternion& q) { return math::normalize(q); }>("normalize"_hs)

            // Length:
            .func<+[](float value) { return math::length(value); }>("length"_hs)
            .func<+[](const math::Quaternion& q) { return math::length(q); }>("length"_hs)
            .func<+[](const math::Vector4D& v) { return math::length(v); }>("length"_hs)
            .func<+[](const math::Vector2D& v) { return math::length(v); }>("length"_hs)
            .func<+[](const math::Vector3D& v) { return math::length(v); }>("length"_hs)

            // Inverse:
            .func<+[](const math::Quaternion& q) { return math::inverse(q); }>("inverse"_hs)
            .func<+[](const math::Matrix4x4& m) { return math::inverse(m); }>("inverse"_hs)
            .func<+[](const math::Matrix3x3& m) { return math::inverse(m); }>("inverse"_hs)

            // Transpose:
            .func<+[](const math::Matrix4x4& m) { return math::transpose(m); }>("transpose"_hs)
            .func<+[](const math::Matrix3x3& m) { return math::transpose(m); }>("transpose"_hs)

            // Translate:
            .func<+[](const math::Matrix4x4& m, const math::Vector3D& v) { return math::translate(m, v); }>("translate"_hs)

            // Rotate:
            .func<+[](const math::Matrix4x4& m, float angle, const math::Vector3D& v) { return math::rotate(m, angle, v); }>("rotate"_hs)
            .func<+[](const math::Quaternion& q, float angle, const math::Vector3D& v) { return math::rotate(q, angle, v); }>("rotate"_hs)
            .func<+[](const math::Quaternion& q, const math::Vector3D& v) { return math::rotate(q, v); }>("rotate"_hs)
            .func<+[](const math::Quaternion& q, const math::Vector4D& v) { return math::rotate(q, v); }>("rotate"_hs)

            // Scale:
            .func<+[](const math::Matrix4x4& m, const math::Vector3D& v) { return math::scale(m, v); }>("scale"_hs)

            // Mix:
            .func<+[](const math::Quaternion& x, const math::Quaternion& y, float a) { return math::mix(x, y, a); }>("mix"_hs)
            .func<+[](const math::Vector3D& x, const math::Vector3D& y, const math::Vector3D& a) { return math::mix(x, y, a); }>("mix"_hs)
            .func<+[](const math::Vector3D& x, const math::Vector3D& y, float a) { return math::mix(x, y, a); }>("mix"_hs)

            // General:
            .func<&math::positive<std::int64_t>>("positive"_hs)
            .func<&math::positive<std::int32_t>>("positive"_hs)
            .func<&math::positive<double>>("positive"_hs)
            .func<&math::positive<float>>("positive"_hs)

            .func<&math::negative<std::int64_t>>("negative"_hs)
            .func<&math::negative<std::int32_t>>("negative"_hs)
            .func<&math::negative<double>>("negative"_hs)
            .func<&math::negative<float>>("negative"_hs)

            .func<&math::negate<std::int64_t>>("negate"_hs)
            .func<&math::negate<std::int32_t>>("negate"_hs)
            .func<&math::negate<double>>("negate"_hs)
            .func<&math::negate<float>>("negate"_hs)

            .func<&math::sq<std::int64_t>>("square"_hs) // "sq"_hs
            .func<&math::sq<std::int32_t>>("square"_hs) // "sq"_hs
            .func<&math::sq<double>>("square"_hs) // "sq"_hs
            .func<&math::sq<float>>("square"_hs) // "sq"_hs

            .func<&math::identity_matrix>("identity_matrix"_hs)

            .func<&math::clamp<std::int64_t>>("clamp"_hs)
            .func<&math::clamp<std::int32_t>>("clamp"_hs)
            .func<&math::clamp<double>>("clamp"_hs)
            .func<&math::clamp<float>>("clamp"_hs)

            // Vector extension for `abs`.
            .func<static_cast<math::Vector4D(*)(const math::Vector4D&)>(&math::abs)>("abs"_hs)
            .func<static_cast<math::Vector2D(*)(const math::Vector2D&)>(&math::abs)>("abs"_hs)
            .func<static_cast<math::Vector3D(*)(const math::Vector3D&)>(&math::abs)>("abs"_hs)
        ;

        // `math:decompose`:
        math_type = math_type
            .func<&math::get_translation<math::Matrix4x4>>("get_translation"_hs)
            .func<&math::get_scaling<math::Matrix4x4>>("get_scaling"_hs)
        ;

        // `math:rotation`:
        math_type = math_type
            .func<&math::get_vector_pitch>("get_vector_pitch"_hs)
            .func<&math::get_vector_yaw>("get_vector_yaw"_hs)

            .func<&math::get_matrix_pitch>("get_matrix_pitch"_hs)
            .func<&math::get_matrix_yaw>("get_matrix_yaw"_hs)
            .func<&math::get_matrix_roll>("get_matrix_roll"_hs)

            .func<&math::get_rotation>("get_rotation"_hs)
            .func<&math::rotation_from_vector>("rotation_from_vector"_hs)

            .func<&math::rotation_pitch>("rotation_pitch"_hs)
            .func<&math::rotation_yaw>("rotation_yaw"_hs)
            .func<&math::rotation_roll>("rotation_roll"_hs)

            .func<&math::rotation_pitch_q>("rotation_pitch_q"_hs)
            .func<&math::rotation_yaw_q>("rotation_yaw_q"_hs)
            .func<&math::rotation_roll_q>("rotation_roll_q"_hs)

            .func<static_cast<math::Quaternion(*)(const math::Vector&, const math::Vector&, const math::Vector&)>(&math::quaternion_from_orthogonal)>("quaternion_from_orthogonal"_hs)
            .func<static_cast<math::Quaternion(*)(const math::Vector&, const math::Vector&)>(&math::quaternion_from_orthogonal)>("quaternion_from_orthogonal"_hs)
            .func<static_cast<math::Quaternion(*)(const math::OrthogonalVectors&)>(&math::quaternion_from_orthogonal)>("quaternion_from_orthogonal"_hs)

            .func<static_cast<math::RotationMatrix(*)(const math::Vector&, const math::Vector&, const math::Vector&)>(&math::rotation_from_orthogonal)>("rotation_from_orthogonal"_hs)
            .func<static_cast<math::RotationMatrix(*)(const math::Vector&, const math::Vector&)>(&math::rotation_from_orthogonal)>("rotation_from_orthogonal"_hs)
            .func<static_cast<math::RotationMatrix(*)(const math::OrthogonalVectors&)>(&math::rotation_from_orthogonal)>("rotation_from_orthogonal"_hs)

            .func<&math::direction_to_angle>("direction_to_angle"_hs)
            .func<&math::direction_to_angle_90_degrees>("direction_to_angle_90_degrees"_hs)
            .func<&math::direction_to_yaw>("direction_to_yaw"_hs)

            .func<&math::wrap_angle<double>>("wrap_angle"_hs)
            .func<&math::wrap_angle<float>>("wrap_angle"_hs)

            .func<&math::lerp<double, double, double>>("lerp"_hs)
            .func<&math::lerp<float, float, float>>("lerp"_hs)
            .func<&math::lerp<math::Vector2D, math::Vector2D, float>>("lerp"_hs)
            .func<&math::lerp<math::Vector3D, math::Vector3D, float>>("lerp"_hs)
        ;

        // `math:lerp`:
        math_type = math_type
            .func<&math::nlerp<math::Vector2D, math::Vector2D, float>>("nlerp"_hs)
            .func<&math::nlerp<math::Vector3D, math::Vector3D, float>>("nlerp"_hs)

            .func<static_cast<float(*)(const math::Vector&, const math::Vector&, float)>(&math::nlerp_radians)>("nlerp_radians"_hs)
            .func<static_cast<float(*)(float, float, float)>(&math::nlerp_radians)>("nlerp_radians"_hs)

            .func<static_cast<float(*)(const math::Vector&, const math::Vector&, float)>(&math::nlerp_degrees)>("nlerp_degrees"_hs)
            .func<static_cast<float(*)(float, float, float)>(&math::nlerp_degrees)>("nlerp_degrees"_hs)

            .func<&math::slerp>("slerp"_hs)
            .func<&math::slerp_unnormalized>("slerp_unnormalized"_hs)
        ;

        // `math:oscillate`:
        /*
        // Disabled for now. (Due to ICE on MSVC compiler)
        math_type = math_type
            .func<static_cast<float(*)(float, float, float)>(&math::oscillate<float>)>("oscillate"_hs)
            .func<static_cast<float(*)(float)>(&math::oscillate<float>)>("oscillate"_hs)

            .func<static_cast<float(*)(float)>(&math::oscillate_normalized<float>)>("oscillate_normalized"_hs)
        ;
        */

        // `math:surface`:
        math_type = make_overloads
		<
			&math::get_surface_forward,
			[](auto&&... args) { return math::get_surface_forward(std::forward<decltype(args)>(args)...); },
			1
		>(math_type, "get_surface_forward"_hs);

        math_type = make_overloads
		<
			&math::get_surface_slope,
			[](auto&&... args) { return math::get_surface_slope(std::forward<decltype(args)>(args)...); },
			2
		>(math_type, "get_surface_slope"_hs);

        // `math:joyhat`:
        math_type = math_type
            .func<&math::joyhat>("joyhat"_hs)
            .func<&math::joyhat_angle>("joyhat_angle"_hs)
        ;

        // `math:conversion`:
        math_type = math_type
            .func<&math::degrees<double>>("degrees"_hs)
            .func<&math::degrees<float>>("degrees"_hs)

            .func<static_cast<math::Vector(*)(const math::Vector&)>(&math::degrees)>("degrees"_hs)

            .func<&math::radians<double>>("radians"_hs)
            .func<&math::radians<float>>("radians"_hs)

            .func<static_cast<math::Vector(*)(const math::Vector&)>(&math::radians)>("radians"_hs)

            .func<&math::to_normalized_device_coordinates_ex>("to_normalized_device_coordinates_ex"_hs)
            .func<&math::to_normalized_device_coordinates>("to_normalized_device_coordinates"_hs)

            .func<&math::from_normalized_device_coordinates_ex>("from_normalized_device_coordinates_ex"_hs)
            .func<&math::from_normalized_device_coordinates>("from_normalized_device_coordinates"_hs)

            .func<static_cast<math::vec2f(*)(const math::vec2f&)>(&math::normalized_device_coordinates_to_screen_space)>("normalized_device_coordinates_to_screen_space"_hs)
            .func<static_cast<math::vec2f(*)(const math::vec2f&, const math::vec2f&)>(&math::normalized_device_coordinates_to_screen_space)>("normalized_device_coordinates_to_screen_space"_hs)
        ;

        return math_type;
    }

    // TODO: Implement reflection for matrix types.
    template <>
    void reflect<Math>()
    {
        auto math_type = engine_empty_meta_type<Math>()
            .prop("global namespace"_hs)
        ;
        
        //math_type = reflect_cmath<std::int64_t>(math_type);
        math_type = reflect_cmath<std::int32_t>(math_type);
        //math_type = reflect_cmath<double>(math_type);
        math_type = reflect_cmath<float>(math_type);

        math_type = reflect_engine_math_functions(math_type);
        
        reflect<math::Vector2D>();
        reflect<math::Vector3D>();
        reflect<math::Vector4D>();
        reflect<math::Quaternion>();

        reflect<math::vec2i>();

        reflect<math::Matrix4x4>();
        reflect<math::Matrix3x3>();

        reflect<math::TransformVectors>();

        // ...
    }
}