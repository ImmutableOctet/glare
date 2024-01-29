#include "util.hpp"

#include "function.hpp"
#include "sampler_extensions.hpp"

#include <engine/meta/hash.hpp>

#include <util/reflection.hpp>
#include <util/lambda.hpp>
#include <util/sampler.hpp>

#include <type_traits>

namespace engine
{
	template
    <
        typename T,
        
        bool generate_optional_type=true//,
        //bool generate_operators=true,
        //bool generate_standard_methods=true
    >
    auto reflect_util_type(auto type_name, bool sync_context=true)
    {
        if (sync_context)
        {
            // Ensure that we're using the correct context.
            sync_reflection_context();
        }

        auto type = entt::meta<T>().type(hash(type_name));

        if constexpr (generate_optional_type)
        {
            auto opt_type = custom_optional_type<T>(type_name);
        }

        return type;
    }

    template <typename SamplerType, bool generate_optional_types=true, bool generate_operators=true>
    static auto reflect_sampler_type(auto type_name, auto entry_type_name, bool sync_context=true)
    {
        using ContainerType = typename SamplerType::Container;
        using EntryType     = typename SamplerType::Entry;
        using KeyType       = typename SamplerType::key_type;   // typename EntryType::first_type;
        using ValueType     = typename SamplerType::value_type; // typename EntryType::second_type;

        using STLVector      = std::vector<EntryType>;
        using STLDynamicSpan = std::span<EntryType, std::dynamic_extent>;

        auto entry_type = reflect_util_type<EntryType, generate_optional_types>(entry_type_name)
            .ctor<KeyType, ValueType>()

            .data<&EntryType::first>("x"_hs)
            .data<&EntryType::second>("y"_hs)

            .data<&EntryType::first>("key"_hs)
            .data<&EntryType::second>("value"_hs)

            .data<&EntryType::first>("first"_hs)
            .data<&EntryType::second>("second"_hs)
        ;

        auto sampler_type = reflect_util_type<SamplerType, generate_optional_types>(type_name)
            .prop("container wrapper"_hs)

            .ctor<const STLVector&, bool>()
            .ctor<const STLVector&>()
            
            .ctor<const STLDynamicSpan&, bool>()
            .ctor<const STLDynamicSpan&>()
            
            .ctor<const ContainerType&, bool>()
            .ctor<const ContainerType&>()
            
            //.ctor<ContainerType&&, bool>()
            //.ctor<ContainerType&&>()

            .func<&impl::sampler_get_wrapped_container_type<SamplerType>>("get_wrapped_container_type"_hs)
            .func<&impl::sampler_get_wrapped_container_type_id<SamplerType>>("get_wrapped_container_type_id"_hs)

            .func<&impl::sampler_get_key_type<SamplerType>>("get_key_type"_hs)
            .func<&impl::sampler_get_key_type_id<SamplerType>>("get_key_type_id"_hs)

            .func<&impl::sampler_get_value_type<SamplerType>>("get_value_type"_hs)
            .func<&impl::sampler_get_value_type_id<SamplerType>>("get_value_type_id"_hs)

            .func<&impl::sampler_get_pair_type<SamplerType>>("get_pair_type"_hs)
            .func<&impl::sampler_get_pair_type_id<SamplerType>>("get_pair_type_id"_hs)

            // Disabled for now to avoid confusion.
            //.func<&SamplerType::get_value_by_index>("operator[]"_hs)

            .func<&SamplerType::get_value>("operator[]"_hs)

            .func<&SamplerType::get_entry_by_index>("get_entry_by_index"_hs)
            .func<&SamplerType::get_key_by_index>("get_key_by_index"_hs)
            .func<&SamplerType::get_value_by_index>("get_value_by_index"_hs)
            
            .func<&SamplerType::get_next_entry>("get_next_entry"_hs)
            .func<&SamplerType::get_prev_entry>("get_prev_entry"_hs)
            //.func<&SamplerType::get_nearest_pair>("get_nearest_pair"_hs)

            .func<&SamplerType::get_next_value>("get_next_value"_hs)

            .func<&SamplerType::get_value>("get_value"_hs)
            .func<&SamplerType::get_value>("get"_hs)

            .func<&SamplerType::get_sample_data, entt::as_cref_t>("data"_hs)

            .data<nullptr, &SamplerType::get_sample_data, entt::as_cref_t>("data"_hs)

            .data<nullptr, &SamplerType::size>("size"_hs)
            .data<nullptr, &SamplerType::empty>("empty"_hs)

            .conv<bool>()
        ;

        sampler_type = make_overloads
		<
			&SamplerType::get_cumulative_value,
			[](const auto& sampler, auto&&... args) { return sampler.get_cumulative_value(std::forward<decltype(args)>(args)...); },
			1
		>(sampler_type, "get_cumulative_value"_hs);

        return sampler_type;
    }

    // Reflects `util::Sampler1D` with the generalized name of `Sampler1D`.
    template <>
    void reflect<util::Sampler1D>()
    {
        reflect_sampler_type<util::Sampler1D>("Sampler1D", "SampleEntry1D"); // "FloatSampler"
    }

    // Reflects `util::Sampler2D` with the generalized name of `Sampler2D`.
    template <>
    void reflect<util::Sampler2D>()
    {
        reflect_sampler_type<util::Sampler2D>("Sampler2D", "SampleEntry2D");
    }

    // Reflects `util::Sampler3D` with the generalized name of `Sampler3D`.
    template <>
    void reflect<util::Sampler3D>()
    {
        reflect_sampler_type<util::Sampler3D>("Sampler3D", "SampleEntry3D");
    }

    // Reflects `util::Sampler4D` with the generalized name of `Sampler4D`.
    template <>
    void reflect<util::Sampler4D>()
    {
        reflect_sampler_type<util::Sampler4D>("Sampler4D", "SampleEntry4D");
    }

    // Reflects `util::MatrixSampler` with the generalized name of `MatrixSampler`.
    template <>
    void reflect<util::MatrixSampler>()
    {
        reflect_sampler_type<util::MatrixSampler>("MatrixSampler", "MatrixSampleEntry");
    }

    // Reflects `util::RotationMatrixSampler` with the generalized name of `RotationMatrixSampler`.
    template <>
    void reflect<util::RotationMatrixSampler>()
    {
        reflect_sampler_type<util::RotationMatrixSampler>("RotationMatrixSampler", "RotationMatrixSampleEntry");
    }

    // Reflects `util::QuaternionSampler` with the generalized name of `QuaternionSampler`.
    template <>
    void reflect<util::QuaternionSampler>()
    {
        reflect_sampler_type<util::QuaternionSampler>("QuaternionSampler", "QuaternionSampleEntry");
    }

    auto reflect_engine_util_functions(auto util_type)
    {
        // `util:...`:
        //util_type = util_type

        return util_type;
    }

    // TODO: Implement reflection for matrix types.
    template <>
    void reflect<Util>()
    {
        auto util_type = engine_global_static_type<Util>()
            .prop("global namespace"_hs)
        ;

        util_type = reflect_engine_util_functions(util_type);
        
        reflect<util::Sampler1D>();
        reflect<util::Sampler2D>();
        reflect<util::Sampler3D>();
        reflect<util::Sampler4D>();
        reflect<util::MatrixSampler>();
        reflect<util::RotationMatrixSampler>();
        reflect<util::QuaternionSampler>();

        // ...
    }
}