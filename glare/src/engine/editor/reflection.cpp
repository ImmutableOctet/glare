#pragma once

#include "reflection.hpp"

#include "editor.hpp"

#include "components/editor_properties_component.hpp"

namespace engine
{
    template <>
    void reflect<EditorPropertiesComponent>()
    {
        engine_meta_type<EditorPropertiesComponent>()
            .data<&EditorPropertiesComponent::set_is_selected, &EditorPropertiesComponent::get_is_selected>("is_selected"_hs)
        ;
    }

    template <>
    void reflect<Editor>()
    {
        auto type = engine_system_type<Editor>()
            .data<nullptr, &Editor::get_root>("root"_hs)
        ;

        reflect<EditorPropertiesComponent>();
    }
}