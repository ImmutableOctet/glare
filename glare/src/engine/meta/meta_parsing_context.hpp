#pragma once

namespace engine
{
	struct MetaTypeResolutionContext;
	struct MetaVariableContext;

	class MetaParsingContext
	{
		public:
			inline MetaParsingContext
			(
				const MetaTypeResolutionContext* type_context={},
				MetaVariableContext* variable_context={}
			) :
				type_context(type_context),
				variable_context(variable_context)
			{}

			inline MetaParsingContext(const MetaTypeResolutionContext* type_context)
				: MetaParsingContext(type_context, nullptr) {}

			inline MetaParsingContext(MetaVariableContext* variable_context)
				: MetaParsingContext(nullptr, variable_context) {}

			MetaParsingContext(const MetaParsingContext&) = default;
			MetaParsingContext(MetaParsingContext&&) noexcept = default;

			MetaParsingContext& operator=(const MetaParsingContext&) = default;
			MetaParsingContext& operator=(MetaParsingContext&&) noexcept = default;

			//explicit operator bool() const;

			inline const MetaTypeResolutionContext* get_type_context() const
			{
				return type_context;
			}

			inline MetaVariableContext* get_variable_context() const
			{
				return variable_context;
			}
		protected:
			const MetaTypeResolutionContext* type_context = nullptr;
			MetaVariableContext* variable_context = nullptr;
	};
}