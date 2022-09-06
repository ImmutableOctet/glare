#pragma once

#include <util/static_member_function.hpp>
#include <engine/service_events.hpp>
#include <engine/input_events.hpp>

// TODO: Remove direct usage of `world` header. (currently required for `delta`)
#include <engine/world/world.hpp>

//#define BEHAVIOR_ASSUME_SERVICE_IS_ALWAYS_WORLD 1

namespace app::input
{
    struct MouseState;
    struct KeyboardState;
}

namespace engine
{
    class World;
    class Service;

    //struct OnServiceUpdate;
    //struct OnMouseState;
    //struct OnKeyboardState;

    // Possible behavior event triggers:
    GENERATE_STATIC_MEMBER_FUNCTION_CHECK(on_update, void (*)(engine::World&, float));
    GENERATE_STATIC_MEMBER_FUNCTION_CHECK(on_mouse, void(*)(World&, float, const app::input::MouseState&));
    GENERATE_STATIC_MEMBER_FUNCTION_CHECK(on_keyboard, void(*)(World&, float, const app::input::KeyboardState&));

    namespace behavior_impl
    {
        World* resolve_world_from_service(Service* service);
        const World* resolve_world_from_service(const Service* service);

        template <typename ServiceEventType>
	    inline World* resolve_world(const ServiceEventType& event_obj)
	    {
            return resolve_world_from_service(event_obj.service);
	    }

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

            BehaviorType::on_mouse(*world, world->delta(), *mouse_event.mouse_state);
        }

        template <typename BehaviorType>
        void bridge_on_keyboard(const OnKeyboardState& keyboard_event)
        {
            auto* world = resolve_world(keyboard_event);

            if (!world)
            {
                return;
            }

            BehaviorType::on_keyboard(*world, world->delta(), *keyboard_event.keyboard_state);
        }
    }
}