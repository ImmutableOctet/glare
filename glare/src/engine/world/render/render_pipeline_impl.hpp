#pragma once

#include "render_pipeline.hpp"

#include <util/pipeline.hpp>

#include <utility>
#include <memory>

namespace engine
{
	/*
		This produces a `util::inject_self` object for use in a pipeline, which in turn
		allows you to inspect the specified `phase` object in the following execution step via `util::inject_self::Manifold`.
		
		If you don't want to fully specify the `Manifold` type in the next execution step (i.e. you want to use [](auto&& manifold){...}),
		it's recommended that you use the callback-based overload of `InspectRenderPhase` instead.
	*/
	template <typename RenderPhaseType, typename InputType=const RenderParameters&>
	auto InspectRenderPhase(RenderPhaseType&& phase)
	{
		return util::inject_self<RenderPhaseType, InputType>(std::forward<RenderPhaseType>(phase));
	}

	/*
		This version of `InspectRenderPhase` acts as a workaround for having the user specify the
		`Manifold` type in their following pipline step. (This is specified instead using `callback_fn`)
		
		Basically, by move-capturing the `phase` and `callback_fn` objects in another (mutable) lambda,
		we're able to explicitly utilize the `inject_self` type (and by extension, its `Manifold` type) when calling `callback_fn`,
		allowing its input argument to be auto&&, but still qualify for the `operator()` lookup needed for the pipeline templates.
		
		This boils down to the compiler being unable to introspect template/auto-deduced inputs, since its not an instantiation.
		
		So, to make a long story short, you can send [](auto&& manifold) {...} into the `callback_fn` argument,
		instead of [](const util::inject_self<RenderPhaseType, const engine::RenderParameters&>::Manifold& manifold) {...}
		-- this isn't possible with another pipeline step by itself.
	*/
	template <typename RenderPhaseType, typename Callback, typename InputType=const RenderParameters&>
	auto InspectRenderPhase(RenderPhaseType&& phase, Callback&& callback_fn)
	{
		using inject_t = util::inject_self<RenderPhaseType, InputType>;

		return [phase = std::move(phase), callback_fn = std::move(callback_fn)](InputType&& input) mutable
		{
			return callback_fn(inject_t(std::move(phase))(input));
		};
	}

	template <typename... RenderPhases>
	class RenderPipelineImpl : public RenderPipeline
	{
		protected:
			util::pipeline<RenderPhases...> phases;

		public:
			RenderPipelineImpl(RenderPhases&&... phases)
				: phases(std::forward<RenderPhases>(phases)...) {}

			virtual void render(const RenderParameters& parameters) override
			{
				phases(parameters);
			}
	};

	template <typename ...Phases>
	auto make_render_pipeline(Phases&&... phases)
	{
		return std::make_unique<engine::RenderPipelineImpl<Phases...>>(std::forward<Phases>(phases)...);
	}
}