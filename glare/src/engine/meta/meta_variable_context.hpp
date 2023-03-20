#pragma once

#include "types.hpp"
#include "meta_variable_scope.hpp"
#include "meta_variable_description.hpp"

//#include <engine/types.hpp>

#include <util/small_vector.hpp>

#include <tuple>
#include <string_view>

namespace engine
{
	class MetaParsingContext;

	class MetaVariableContext
	{
		public:
			using VariableScope = MetaVariableScope;

			using VariableDescription = MetaVariableDescription;

			static MetaSymbolID resolve_path(MetaSymbolID prefix, std::string_view scope_local_name, MetaVariableScope scope=MetaVariableScope::Local);
			static MetaSymbolID resolve_path(MetaSymbolID prefix, MetaSymbolID scope_local_id, MetaVariableScope scope=MetaVariableScope::Local);

			std::tuple
			<
				const MetaVariableDescription*, // Description of the variable.
				bool                            // Whether a variable was defined by this call.
			>
			define_or_retrieve_variable(MetaVariableScope scope, std::string_view scope_local_name, MetaTypeID type_id={}, bool forward_to_parent=false, bool fail_on_different_type=true);

			const MetaVariableDescription* define_variable(MetaVariableScope scope, std::string_view scope_local_name, MetaTypeID type_id={}, bool forward_to_parent=false);

			const MetaVariableDescription* retrieve_variable(std::string_view scope_local_name, bool fallback_to_parent=true) const;
			const MetaVariableDescription* retrieve_variable(MetaSymbolID scope_local_id, bool fallback_to_parent=true) const;

			const MetaVariableDescription* retrieve_variable_by_resolved_id(MetaSymbolID resolved_id, bool fallback_to_parent=true) const;

			MetaVariableContext(MetaVariableContext* parent_context={}, MetaSymbolID name={});
			MetaVariableContext(const MetaParsingContext& parent_parsing_context);

			MetaVariableContext(const MetaVariableContext&) = default; // delete;
			MetaVariableContext(MetaVariableContext&&) noexcept = default;

			MetaSymbolID get_resolved_id(MetaVariableScope scope, std::string_view scope_local_name) const;
			MetaSymbolID get_resolved_id(MetaVariableScope scope, MetaSymbolID scope_local_id) const;

			MetaVariableContext* get_parent();
			const MetaVariableContext* get_parent() const;

			MetaVariableContext* get_root();
			const MetaVariableContext* get_root() const;

			bool contains(MetaSymbolID scope_local_id) const;
			bool contains(std::string_view scope_local_name) const;
			bool contains(MetaVariableScope scope, MetaSymbolID scope_local_id) const;
			bool contains_resolved(MetaSymbolID resolved_id) const;
			bool contains_resolved(MetaVariableScope scope, MetaSymbolID resolved_id) const;

			bool has_parent() const;
			bool parent_is_root() const;

			bool has_name() const;

			bool set_name(MetaSymbolID name, bool force=false);

			MetaSymbolID get_name() const;
			MetaSymbolID get_parent_name(bool recursive=true) const;
			MetaSymbolID get_path(bool recursive=true) const;

			// Applies the prefix specified to the contents of `variables`.
			std::size_t apply_prefix(std::string_view prefix, std::optional<MetaVariableScope> scope=std::nullopt);

			// Applies the prefix specified to the contents of `variables`.
			std::size_t apply_prefix(MetaSymbolID prefix, std::optional<MetaVariableScope> scope=std::nullopt);

			// Vector used here over `std::unordered_map` (or similar)
			// due to small number of elements.
			util::small_vector<VariableDescription, 16> variables;
		protected:
			MetaVariableContext* parent_context = {};
			MetaSymbolID name = {};
	};
}