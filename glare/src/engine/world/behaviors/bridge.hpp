#pragma once

#include <util/member_traits.hpp>
#include <engine/service_events.hpp>
#include <engine/input/raw_input_events.hpp>

// TODO: Remove direct usage of `world` header. (currently required for `delta`)
#include <engine/world/world.hpp>

//#define BEHAVIOR_ASSUME_SERVICE_IS_ALWAYS_WORLD 1

namespace app::input
{
    // Input states:
    struct MouseState;
    struct KeyboardState;
}

// Generates a simple function that resolves a `World` object from an event type.
#define GENERATE_BEHAVIOR_BRIDGE(behavior_function_name, behavior_event_type)    \
namespace behavior_impl                                                          \
{                                                                                \
    template <typename BehaviorType>                                             \
    void bridge_##behavior_function_name(const behavior_event_type& event_obj)   \
    {                                                                            \
        auto* world = resolve_world(event_obj);                                  \
                                                                                 \
        if (!world)                                                              \
        {                                                                        \
            return;                                                              \
        }                                                                        \
                                                                                 \
        BehaviorType::behavior_function_name(*world, world->get_delta(), event_obj); \
    }                                                                            \
}

// Generates a function trait for `behavior_function_name` that handles `behavior_event_type`,
// taking in a `World` instance and a delta-time value as the only other parameters.
#define GENERATE_SIMPLE_BEHAVIOR_BRIDGE(behavior_function_name, behavior_event_type)                                   \
    GENERATE_EXACT_FUNCTION_TRAIT_SIMPLIFIED(behavior_function_name, void, World&, float, const behavior_event_type&); \
    GENERATE_BEHAVIOR_BRIDGE(behavior_function_name, behavior_event_type);

namespace engine
{
    class World;
    class Service;

    // Event types:
    struct OnInput;
    struct OnButtonDown;
    struct OnButtonPressed;
    struct OnButtonReleased;
    struct OnAnalogInput;

    //struct OnServiceUpdate;
    //struct OnMouseState;
    //struct OnKeyboardState;

    //void reflect_behaviors();

    namespace behavior_impl
    {
        World* resolve_world_from_service(Service* service);
        const World* resolve_world_from_service(const Service* service);

        template <typename ServiceEventType>
        inline World* resolve_world(const ServiceEventType& event_obj)
        {
            return resolve_world_from_service(event_obj.service);
        }
    }

    // Possible event triggers for behaviors:

    // See also: `SystemManager::register_behavior` and `SystemManager::unregister_behavior`

    // General purpose:
    GENERATE_EXACT_FUNCTION_TRAIT_SIMPLIFIED(on_update,   void, engine::World&, float);

    // Raw input:
    GENERATE_EXACT_FUNCTION_TRAIT_SIMPLIFIED(on_mouse,    void, World&, float, const app::input::MouseState&);
    GENERATE_EXACT_FUNCTION_TRAIT_SIMPLIFIED(on_keyboard, void, World&, float, const app::input::KeyboardState&);

    // High-level input:

    // Triggered any time the input state changes.
    GENERATE_SIMPLE_BEHAVIOR_BRIDGE(on_input_update, OnInput);

    // Triggered continuously while a button is held down.
    GENERATE_SIMPLE_BEHAVIOR_BRIDGE(on_button_down, OnButtonDown);

    // Triggered once after a button has been released. (Following after one or more calls to `on_button_down`)
    GENERATE_SIMPLE_BEHAVIOR_BRIDGE(on_button_up, OnButtonReleased);

    // Triggered once when a button is initially pressed.
    GENERATE_SIMPLE_BEHAVIOR_BRIDGE(on_button_pressed, OnButtonPressed);

    // TODO: Look into automating entity enumeration for behavior-bridge functions.

    namespace behavior_impl
    {
        // Event-type bridge functions and templates:
        template <typename BehaviorType>
        void bridge_on_update(const OnServiceUpdate& update_event)
        {
            auto* world = resolve_world(update_event);

            if (!world)
            {
                return;
            }

            BehaviorType::on_update(*world, update_event.delta);
        }

        template <typename BehaviorType>
        void bridge_on_mouse(const OnMouseState& mouse_event)
        {
            auto* world = resolve_world(mouse_event);

            if (!world)
            {
                return;
            }

            BehaviorType::on_mouse(*world, world->get_delta(), *mouse_event.mouse_state);
        }

        template <typename BehaviorType>
        void bridge_on_keyboard(const OnKeyboardState& keyboard_event)
        {
            auto* world = resolve_world(keyboard_event);

            if (!world)
            {
                return;
            }

            BehaviorType::on_keyboard(*world, world->get_delta(), *keyboard_event.keyboard_state);
        }
    }
}