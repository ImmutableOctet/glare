#pragma once

#include <util/static_member_function.hpp>
#include <engine/service_events.hpp>

//#define BEHAVIOR_ASSUME_SERVICE_IS_ALWAYS_WORLD 1

namespace engine
{
    class World;
    class Service;

    //struct OnServiceUpdate;

    // Possible behavior event triggers:
    GENERATE_STATIC_MEMBER_FUNCTION_CHECK(on_update, void (*)(engine::World&, float));
    //GENERATE_STATIC_MEMBER_FUNCTION_CHECK(on_mouse, );
    //GENERATE_STATIC_MEMBER_FUNCTION_CHECK(on_keyboard, );

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
            auto* world = resolve_world_from_service(update_event.service);

            if (!world)
            {
                return;
            }

            BehaviorType::on_update(*world, update_event.delta);
        }
    }
}