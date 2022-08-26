#pragma once

#include <utility>
#include <type_traits>

#include "function_traits.hpp"

namespace util
{
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

    // Last recursive pipeline phase. (1:1)
    template<typename CurrentPhase>
    auto execute_phases(typename first_argument_type<CurrentPhase> input_value, CurrentPhase last_phase)
    {
        return last_phase(input_value);
    }

    // Execute each phase in the pipeline, forwarding the remaining phases to each iteration.
    // The last recursive execution will simply execute the last phase. (see above)
    template<typename CurrentPhase, typename... NextPhases>
    typename std::enable_if
    <
        (sizeof...(NextPhases) > 0),
        typename pipeline_output<CurrentPhase, NextPhases...>::type
    >::type
    execute_phases(typename pipeline_input<CurrentPhase, NextPhases...>::type input_value, CurrentPhase current_phase, NextPhases... next_phases)
    {
        return execute_phases(current_phase(input_value), next_phases...);
    }

    // Expand a tuple into variadic arguments, and apply `execute_phases`.
    template <typename InputValue, typename Phases, size_t... PhaseIndices>
    constexpr auto execute_phases_expansion(InputValue& input_value, Phases& phases, std::index_sequence<PhaseIndices...>)
    {
        return execute_phases(input_value, std::get<PhaseIndices>(phases)...);
    }

    template <typename InputValue, typename Phases>
    constexpr auto execute_phases_tuple(InputValue& input_value, Phases& phases)
    {
        return execute_phases_expansion(input_value, phases, std::make_index_sequence<std::tuple_size<Phases>::value>{});
    }

    // Creates a pipeline from multiple phases, supplied as template parameters. (see also: `make_pipeline`)
    template<typename... Phases>
    struct pipeline
    {
        // T-in/T-out types:
        using in_type = typename pipeline_input<Phases...>::type;
        using out_type = typename pipeline_output<Phases...>::type;

        // Tuple holding the current state of each phase.
        std::tuple<Phases...> phases;
        
        pipeline(std::tuple<Phases...>&& phases) : phases(std::forward<std::tuple<Phases...>>(phases)) {};
        pipeline(Phases&&... phases) : phases(std::tuple<Phases...>(std::forward<Phases>(phases)...)) {};
        
        // Call operator; `pipeline` objects are lazy and will not execute until this is invoked.
        out_type operator()(in_type input_value)
        {
            return execute_phases_tuple(input_value, phases);
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