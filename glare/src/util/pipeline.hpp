#pragma once

#include "function_traits.hpp"

#include <utility>
#include <type_traits>

namespace util
{
    namespace impl
    {
        template <typename in_type, typename out_type>
        inline constexpr bool pipeline_output_same_type_as_input =
        (
            (std::is_same_v<std::decay_t<in_type>, std::decay_t<out_type>>)
            &&
            (std::is_const_v<std::remove_reference_t<in_type>>)
            &&
            (std::is_const_v<std::remove_reference_t<out_type>>)
        );
    }

	// Retrieves details on the input type and the phase that accepts it.
    template<typename... Phases>
    struct pipeline_input
    {
        using first_phase = typename std::tuple_element<0, std::tuple<Phases...>>::type;
        using type = typename first_argument_type<first_phase>;
    };

    // Retrieves details on the output type and the phase that generated it.
    template<typename... Phases>
    struct pipeline_output
    {
        using final_phase = typename std::tuple_element<(sizeof...(Phases)-1), std::tuple<Phases...>>::type;
        using first_arg_type = first_argument_type<final_phase>;

        // Final output type.
        using type = std::invoke_result_t<final_phase, first_arg_type>;
    };

    // Execute each phase in the pipeline, forwarding the remaining phases to each iteration.
    // The last recursive execution will simply execute the last phase. (see above)
    template <typename CurrentPhase, typename... NextPhases>
    constexpr decltype(auto) execute_phases
    (
        typename pipeline_input<CurrentPhase, NextPhases...>::type input_value,
        CurrentPhase& current_phase,
        NextPhases&... next_phases
    )
    {
        using ReturnType = std::conditional_t
        <
            impl::pipeline_output_same_type_as_input
            <
                typename pipeline_input<CurrentPhase, NextPhases...>::type,
                typename pipeline_output<CurrentPhase, NextPhases...>::type
            >,

            void,

            typename pipeline_output<CurrentPhase, NextPhases...>::type
        >;

        if constexpr (std::is_void_v<decltype(current_phase(input_value))>)
        {
            current_phase(input_value);

            if constexpr (sizeof...(next_phases) > 0)
            {
                if constexpr (std::is_void_v<ReturnType>)
                {
                    execute_phases(input_value, next_phases...);
                }
                else
                {
                    return execute_phases(input_value, next_phases...);
                }
            }
        }
        else
        {
            if constexpr (sizeof...(next_phases) > 0)
            {
                return execute_phases(current_phase(std::forward<decltype(input_value)>(input_value)), next_phases...);
            }
            else
            {
                if constexpr (std::is_void_v<ReturnType>)
                {
                    current_phase(std::forward<decltype(input_value)>(input_value));
                }
                else
                {
                    return current_phase(std::forward<decltype(input_value)>(input_value));
                }
            }
        }
    }

    // Expand a tuple into variadic arguments, and apply `execute_phases`.
    template <typename InputValue, typename Phases, size_t... PhaseIndices>
    constexpr decltype(auto) execute_phases_expansion(InputValue&& input_value, Phases& phases, std::index_sequence<PhaseIndices...>)
    {
        if constexpr (std::is_void_v<decltype(execute_phases(std::forward<InputValue>(input_value), std::get<PhaseIndices>(phases)...))>)
        {
            execute_phases(std::forward<InputValue>(input_value), std::get<PhaseIndices>(phases)...);
        }
        else
        {
            return execute_phases(std::forward<InputValue>(input_value), std::get<PhaseIndices>(phases)...);
        }
    }

    template <typename InputValue, typename Phases>
    constexpr decltype(auto) execute_phases_tuple(InputValue&& input_value, Phases& phases)
    {
        if constexpr (std::is_void_v<decltype(execute_phases_expansion(std::forward<InputValue>(input_value), phases, std::make_index_sequence<std::tuple_size<Phases>::value> {}))>)
        {
            execute_phases_expansion(std::forward<InputValue>(input_value), phases, std::make_index_sequence<std::tuple_size<Phases>::value> {});
        }
        else
        {
            return execute_phases_expansion(std::forward<InputValue>(input_value), phases, std::make_index_sequence<std::tuple_size<Phases>::value> {});
        }
    }

    // Creates a pipeline from multiple phases, supplied as template parameters. (see also: `make_pipeline`)
    template<typename... Phases>
    struct pipeline
    {
        // T-in/T-out types:
        using in_type = typename pipeline_input<Phases...>::type;
        using out_type = typename pipeline_output<Phases...>::type;

        using in_value_type = std::remove_const_t<std::remove_reference_t<in_type>>;
        using in_rvalue_type = in_value_type&&;

        // Tuple holding the current state of each phase.
        std::tuple<Phases...> phases;
        
        pipeline(std::tuple<Phases...>&& phases) : phases(std::forward<std::tuple<Phases...>>(phases)) {};
        pipeline(Phases&&... phases) : phases(std::tuple<Phases...>(std::forward<Phases>(phases)...)) {};

        // Call operator; `pipeline` objects are lazy and will not execute until this is invoked.
        template
        <
            typename InputArgType,

            bool bypass_return_value =
            (
                (std::is_rvalue_reference_v<InputArgType>)
                ||
                (
                    (impl::pipeline_output_same_type_as_input<in_type, out_type>)
                    &&
                    (
                        (std::is_same_v<std::decay_t<InputArgType>, std::decay_t<in_type>>)
                        ||
                        (std::is_constructible_v<std::decay_t<in_type>, InputArgType>)
                    )
                )
            )
        >
        decltype(auto) operator()(InputArgType&& input_value) // -> out_type
        {
            if constexpr (bypass_return_value || std::is_void_v<decltype(execute_phases_tuple(input_value, phases))>)
            {
                execute_phases_tuple(input_value, phases);
            }
            else
            {
                //static_assert(std::is_same_v<decltype(execute_phases_tuple(input_value, phases)), out_type>); // std::is_convertible_v<...>

                return execute_phases_tuple(input_value, phases);
            }
        }
    };

    // Utility function that generates a pipeline object.
    template<typename... Phases>
    auto make_pipeline(Phases... phases) { return pipeline<Phases...>(std::forward<Phases>(phases)...); };

    /*
        Simple type used to store a reference to a `phase` object in a pipeline as well as its produced value.

        This is useful when you want to reference the object being called within a constructed pipeline,
        but have no way of obtaining a reference to it, outside of the pipeline itself.

        e.g. You want to access members from a `GeometryRenderPhase` object before forwarding its `operator()` result to `DeferredShadingPhase`.
    */
    template <typename phase_type, typename input_type>
    struct inject_self
    {
        using PhaseType = phase_type;
        using InputType = input_type;

        // Result of calling `operator()` on `phase`.
        using OutputType = std::invoke_result<PhaseType, InputType>::type; // std::decay<InputType>::type
        //using OutputType = decltype(phase(std::decay<InputType>::type()));
        //using OutputType = std::result_of<PhaseType(InputType)>::type;

        PhaseType phase;

        struct Manifold
        {
            PhaseType& phase;
            OutputType output;
        };

        inline Manifold operator()(InputType&& input)
        {
            return { phase, phase(std::forward<InputType>(input)) };
        }
    };

    /*
    template <typename phase_type, typename input_type>
    struct store_and_inspect
    {
        using PhaseType = phase_type;
        using InputType = input_type;

        // Result of calling `operator()` on `phase`.
        using OutputType = std::invoke_result<PhaseType, InputType>::type; // std::decay<InputType>::type

        template <typename Callback>
        store_and_inspect(PhaseType&& phase, Callback&& callback)
            phase(std::move(phase))
        {
            callback(phase);
        }

        PhaseType phase;

        inline OutputType operator()(InputType&& input)
        {
            return phase(input);
        }
    };
    */

    // Calls `callback_fn` with `phase` (as an lvalue reference), then returns `phase` back to the caller.
    template <typename PhaseType, typename Callback>
    PhaseType inspect_and_store(PhaseType&& phase, Callback&& callback_fn)
    {
        callback_fn(phase);

        return phase;
    }
}