#pragma once

namespace engine
{
	// Simple system base-class to handle setup boilerplate.
	// 
	// NOTE: Systems do not have to inherit from this type,
	// and are free to define variations of this interface.
	template <typename ServiceType>
	class BasicSystemImpl
	{
		public:
			// The `ServiceType` this system was designed for.
			using ConnectedServiceType = ServiceType;

			// This acts as the default constructor.
			// Systems are free to be constructed multiple ways.
			BasicSystemImpl(bool allow_multiple_subscriptions=false) :
				allow_multiple_subscriptions(allow_multiple_subscriptions)
			{}

			// This constructs this system with an intended `service`.
			// Intended services have the benefit of being unsubscribed-from automatically.
			// NOTE:
			// This does not immediately call `subscribe` by default. The reasoning
			// behind this is that this system is not fully constructed during this step.
			BasicSystemImpl(ServiceType& service, bool subscribe_immediately=false, bool allow_multiple_subscriptions=false) :
				service(&service),
				allow_multiple_subscriptions(allow_multiple_subscriptions)
			{}

			// NOTE:
			// This only automatically unsubscribes from the intended service on destruction.
			// If you have subscribed to multiple services with this system, you will need to unsubscribe manually.
			virtual ~BasicSystemImpl()
			{
				// Execute the destructor-safe version of `unsubscribe`.
				// NOTE: Does not trigger `on_unsubscribe`.
				if (this->service)
				{
					unsubscribe_impl<decltype(*this)>(*this->service, false);
				}
			}

			// NOTE:
			// If subscribing with a service other than the intended `service`,
			// the caller is responsible for unsubscribing later.
			bool subscribe(ServiceType& service)
			{
				if (!subscription_allowed(service))
				{
					return false;
				}

				bool result = on_subscribe(service);

				if (result)
				{
					if (is_intended_service(service))
					{
						subscribed_to_intended = true;
					}

					subscribed = true;
				}

				return result;
			}

			/*
				NOTES:
				* This is called automatically on destruction if an intended service is set.
				
				* Although `unsubscribe` itself is called automatically during destruction,
				the `on_unsubscribe` event is NOT triggered during destruction for safety reasons.
			*/
			bool unsubscribe(ServiceType& service)
			{
				return unsubscribe_as<decltype(*this)>(service);
			}

			// Allows you to specify the type of `*this` when calling `unregister` on `ServiceType`.
			// NOTE: RTTI is not utilized by this implementation; use at your own risk.
			template <typename SelfType> // <-- Deducing this unsupported until C++23.
			bool unsubscribe_as(ServiceType& service) // unsafe
			{
				return unsubscribe_impl<SelfType>(service, true);
			}

			// This indicates if this system is currently subscribed to its intended service.
			// Unlike `is_subscribed`, this is guaranteed to reflect the most up-to-date subscription status.
			// 
			// NOTE:
			// If `allow_multiple_subscriptions` is false, this
			// and `is_subscribed` should behave similarly.
			bool is_subscribed_to_intended() const
			{
				return subscribed_to_intended;
			}

			// This indicates if this system has subscribed to a service.
			// NOTE: This does not change after at least one subscription has taken place.
			bool is_subscribed() const
			{
				return subscribed;
			}

			bool allows_multiple_subscriptions() const
			{
				return allow_multiple_subscriptions;
			}

			// Retrieves a reference to the intended `service` this system was constructed with.
			// 
			// NOTE:
			// This service may or may not currently be subscribed;
			// use `is_subscribed_to_intended` to determine this, if needed.
			ServiceType& get_service() const
			{
				return service;
			}
		private:
			// Implementation of `unsubscribe`. (Used internally)
			// `_dispatch` determines if `on_unsubscribe` is called.
			// NOTE: `_dispatch` must be false during destruction.
			template <typename SelfType>
			bool unsubscribe_impl(ServiceType& service, bool _dispatch=true)
			{
				if (!subscribed)
				{
					return false;
				}

				if (!allowed_service(service))
				{
					return false;
				}

				if (!subscribed_to_intended && is_intended_service(service))
				{
					return false;
				}

				bool result = ((_dispatch) ? on_unsubscribe(service) : true);

				if (result)
				{
					service.unregister(reinterpret_cast<SelfType&>(*this));

					if (is_intended_service(service))
					{
						subscribed_to_intended = false;
					}
				}

				return result;
			}
		protected:
			// Called during `subscribe`; return true on success, false on failure.
			// NOTE: If no subscription/registration is performed by your `on_subscribe` implementation, do not return true.
			virtual bool on_subscribe(ServiceType& service) = 0;

			// Called during normal usage of `unsubscribe`; return true on success, false on failure.
			// NOTE: This is not called during destruction due to virtual calls being prohibited.
			virtual bool on_unsubscribe(ServiceType& service)
			{
				// Empty implementation.
				return true;
			}

			// Default implementation; allows all services. (Override this if needed)
			virtual bool is_allowed_service(const ServiceType& service) const
			{
				return true;
			}

			// Returns true if `service` is the same as the intended
			// service this system was constructed with.
			bool is_intended_service(const ServiceType& service) const
			{
				return (&service == this->service);
			}

			// Similar to `subscription_allowed`, but does not check against subscription status.
			bool allowed_service(const ServiceType& service) const
			{
				if (!allows_multiple_subscriptions())
				{
					return is_intended_service(service);
				}

				return is_allowed_service(service);
			}

			// Returns true if this system is allowed to subscribe/unsubscribe to/from `service`.
			bool subscription_allowed(const ServiceType& service) const
			{
				if (!allowed_service(service))
				{
					return false;
				}

				if (is_intended_service(service))
				{
					return (!subscribed_to_intended);
				}

				return true;
			}

			// For internal use by this system; subscribes to the intended service.
			bool subscribe()
			{
				if (!this->service)
				{
					return false;
				}

				return subscribe(*this->service);
			}

			// For internal use by this system; unsubscribes from the intended service.
			bool unsubscribe()
			{
				if (!this->service)
				{
					return false;
				}

				return unsubscribe(*this->service);
			}

			// The intended service for this system, if applicable.
			ServiceType* service = nullptr;

			// Flags:
			bool allow_multiple_subscriptions : 1 = false;
		private:
			bool subscribed                   : 1 = false;
			bool subscribed_to_intended       : 1 = false;
	};

	class Service;

	using BasicSystem = BasicSystemImpl<Service>;
}