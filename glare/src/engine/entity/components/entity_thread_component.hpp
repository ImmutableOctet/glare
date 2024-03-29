#pragma once

#include <engine/entity/entity_thread.hpp>
#include <engine/entity/entity_thread_range.hpp>
#include <engine/entity/entity_variables.hpp>

//#include <util/small_vector.hpp>

#include <utility>
#include <optional>
#include <type_traits>
#include <vector>
#include <optional>
#include <memory>

namespace engine
{
	class EntityDescriptor;
	class EntityState;

	struct EntityThreadComponent
	{
		public:
			using LocalThreadIndex = std::size_t; // EntityThreadIndex;
			using ThreadGlobalVariables = EntityVariables<8>;

			using EntityThreadContainer = std::vector<EntityThread>; // util::small_vector<EntityThread, 4>;
			using EntityThreadIterator = EntityThreadContainer::iterator; // EntityThread*;
			using EntityThreadConstIterator = EntityThreadContainer::const_iterator; // EntityThread*;

			EntityThreadComponent() = default;

			EntityThreadComponent(EntityThreadComponent&&) noexcept = default;
			EntityThreadComponent(const EntityThreadComponent&) = delete;

			EntityThreadComponent& operator=(EntityThreadComponent&&) noexcept = default;
			EntityThreadComponent& operator=(const EntityThreadComponent&) = delete;

			const EntityThreadContainer& get_threads() const
			{
				return threads;
			}

			EntityThreadContainer& get_threads()
			{
				return threads;
			}

			// Attempts to allocate a `ThreadGlobalVariables` object, managed internally.
			// If a `ThreadGlobalVariables` object has already been allocated, this will return a pointer to the existing instance.
			// The value returned is a non-owning pointer to the remote object. (see `global_variables`)
			ThreadGlobalVariables* get_global_variables(); // const

			EntityThread* start_thread
			(
				EntityThreadIndex thread_index,

				std::optional<EntityStateIndex> state_index=std::nullopt,
				bool check_existing=true,
				bool check_linked=true,
				bool restart_existing=false,
				const EntityThreadFlags& flags={}
			);

			EntityThread* start_thread
			(
				const EntityDescriptor& descriptor,
				EntityThreadID thread_id,

				std::optional<EntityStateIndex> state_index,
				bool check_existing,
				bool check_linked,
				bool restart_existing,
				const EntityThreadFlags& flags
			);

			// NOTE: This overload automatically generates an `EntityThreadFlags` object.
			// (e.g. handles the thread's `cadence` field automatically)
			EntityThread* start_thread
			(
				const EntityDescriptor& descriptor,
				EntityThreadID thread_id,

				std::optional<EntityStateIndex> state_index=std::nullopt,
				bool check_existing=true,
				bool check_linked=true,
				bool restart_existing=false
			);

			std::size_t start_threads
			(
				const EntityDescriptor& descriptor,
				const EntityThreadRange& thread_range,
				std::optional<EntityStateIndex> state_index=std::nullopt,
				bool restart_existing=false
			);

			std::size_t start_threads
			(
				const EntityDescriptor& descriptor,
				EntityStateIndex state_index,
				bool restart_existing=false
			);

			std::size_t start_threads
			(
				const EntityDescriptor& descriptor,
				const EntityState& state,
				EntityStateIndex state_index,
				bool restart_existing=false
			);

			// The return value of this method indicates if the specified
			// thread was terminated (true) or detached (false).
			// 
			// If no thread could be found, true will be returned.
			bool stop_thread
			(
				EntityThreadIndex thread_index,
				bool check_linked=true
			);

			bool stop_thread
			(
				const EntityDescriptor& descriptor,
				EntityThreadID thread_id,
				bool check_linked=true
			);

			// The return value of this method indicates how many threads were fully terminated.
			std::size_t stop_threads(const EntityThreadRange& thread_range, bool check_linked=true);

			// The return value of this method indicates how many threads were fully terminated.
			// NOTE: Link status is always ignored when stopping threads by state.
			std::size_t stop_threads(const EntityDescriptor& descriptor, EntityStateIndex state_index, bool limit_to_static_range=false);

			// The return value of this method indicates how many threads were fully terminated.
			// NOTE: Link status is always ignored when stopping threads by state.
			std::size_t stop_threads(const EntityState& state, EntityStateIndex state_index, bool limit_to_static_range=false);

			// NOTE: Link status is always ignored when stopping threads by state.
			std::size_t stop_threads(EntityStateIndex state_index);

			// Attempts to stop all threads.
			// 
			// The return value of this method indicates how many threads were fully terminated.
			std::size_t stop_all();

			// This method erases a thread without formally stopping it first.
			// The return value of this method indicates if the specified thread was erased.
			// 
			// If no thread could be found, false will be returned.
			bool erase_thread(EntityThreadIndex thread_index);

			// This method erases a thread without formally stopping it first.
			bool erase_thread(const EntityDescriptor& descriptor, EntityThreadID thread_id);

			// The return value of this method indicates how many threads were erased.
			std::size_t erase_threads(const EntityThreadRange& thread_range);

			// The return value of this method indicates how many threads were erased.
			std::size_t erase_threads(const EntityDescriptor& descriptor, EntityStateIndex state_index, bool limit_to_static_range=false);

			// The return value of this method indicates how many threads were erased.
			std::size_t erase_threads(const EntityState& state, EntityStateIndex state_index, bool limit_to_static_range=false);

			// Manually erases all threads associated with a state index.
			std::size_t erase_threads(EntityStateIndex state_index);

			// Manually erases all threads marked with the `is_complete` flag.
			std::size_t erase_completed_threads();

			// Pauses a thread with the `thread_id` specified.
			bool pause_thread
			(
				const EntityDescriptor& descriptor,
				EntityThreadID thread_id,
				bool check_linked=true
			);

			// Pauses a thread originating from the `thread_index` specified.
			bool pause_thread
			(
				EntityThreadIndex thread_index,
				bool check_linked=true
			);

			// Pauses threads originating from the `thread_range` specified.
			// The return value of this method indicates how many threads were paused.
			std::size_t pause_threads(const EntityThreadRange& thread_range, bool check_linked=true);

			// Resumes a thread with the `thread_id` specified.
			bool resume_thread
			(
				const EntityDescriptor& descriptor,
				EntityThreadID thread_id,
				bool check_linked=true
			);

			// Resumes a thread originating from the `thread_index` specified.
			bool resume_thread
			(
				EntityThreadIndex thread_index,
				bool check_linked=true
			);

			// Resumes threads originating from the `thread_range` specified.
			// The return value of this method indicates how many threads have resumed execution.
			std::size_t resume_threads(const EntityThreadRange& thread_range, bool check_linked=true);

			// Attaches a thread with the `thread_id` specified.
			bool attach_thread
			(
				const EntityDescriptor& descriptor,
				EntityThreadID thread_id,
				std::optional<EntityStateIndex> state_index=std::nullopt,
				bool check_linked=true
			);

			// Attaches a thread originating from the `thread_index` specified.
			bool attach_thread
			(
				EntityThreadIndex thread_index,
				std::optional<EntityStateIndex> state_index=std::nullopt,
				bool check_linked=true
			);

			// Attaches threads originating from the `thread_range` specified.
			// The return value of this method indicates how many threads have been reattached.
			std::size_t attach_threads
			(
				const EntityThreadRange& thread_range,
				std::optional<EntityStateIndex> state_index=std::nullopt,
				bool check_linked=true
			);

			// Detaches a thread with the `thread_id` specified.
			bool detach_thread
			(
				const EntityDescriptor& descriptor,
				EntityThreadID thread_id,
				bool check_linked=true
			);

			// Detaches a thread originating from the `thread_index` specified.
			bool detach_thread
			(
				EntityThreadIndex thread_index,
				bool check_linked=true
			);

			// Detaches threads originating from the `thread_range` specified.
			// The return value of this method indicates how many threads have been detached.
			std::size_t detach_threads(const EntityThreadRange& thread_range, bool check_linked=true);

			// Unlinks a thread with the `thread_id` specified.
			bool unlink_thread
			(
				const EntityDescriptor& descriptor,
				EntityThreadID thread_id
			);

			// Unlinks a thread originating from the `thread_index` specified.
			bool unlink_thread
			(
				EntityThreadIndex thread_index
			);

			// Unlinks threads originating from the `thread_range` specified.
			// The return value of this method indicates how many threads have been unlinked.
			std::size_t unlink_threads(const EntityThreadRange& thread_range);

			// Attempts to re-link a thread by its local index.
			// 
			// If a thread with the same source index (at a different local index)
			// is already linked, this action will fail (return false).
			bool link_thread(LocalThreadIndex local_thread_index);

			// Skips forward `instruction_count` instructions on a thread with the `thread_id` specified.
			bool skip_thread
			(
				const EntityDescriptor& descriptor,
				EntityThreadID thread_id,
				EntityInstructionCount instruction_count=1,
				bool check_linked=true
			);

			// Skips forward `instruction_count` instructions on a thread originating from the `thread_index` specified.
			bool skip_thread
			(
				EntityThreadIndex thread_index,
				EntityInstructionCount instruction_count=1,
				bool check_linked=true
			);

			// Skips forward `instruction_count` instructions on threads originating from the `thread_range` specified.
			// The return value of this method indicates how many threads have skipped forward the desired amount.
			std::size_t skip_threads(const EntityThreadRange& thread_range, EntityInstructionCount instruction_count=1, bool check_linked=true);

			// Skips backward `instruction_count` instructions on a thread with the `thread_id` specified.
			bool rewind_thread
			(
				const EntityDescriptor& descriptor,
				EntityThreadID thread_id,
				EntityInstructionCount instruction_count=1,
				bool check_linked=true
			);

			// Skips backward `instruction_count` instructions on a thread originating from the `thread_index` specified.
			bool rewind_thread
			(
				EntityThreadIndex thread_index,
				EntityInstructionCount instruction_count=1,
				bool check_linked=true
			);

			// Skips backward `instruction_count` instructions on threads originating from the `thread_range` specified.
			// The return value of this method indicates how many threads have skipped backward the desired amount.
			std::size_t rewind_threads(const EntityThreadRange& thread_range, EntityInstructionCount instruction_count=1, bool check_linked=true);

			// Checks if a (linked) thread spawned from the index specified is running.
			// 
			// See also: `get_thread`
			bool thread_running(EntityThreadIndex thread_index) const;

			// Checks if a (linked) thread spawned with the ID specified is running.
			// 
			// See also: `get_thread`
			bool thread_running(const EntityDescriptor& descriptor, EntityThreadID thread_id) const;

			// Retrieves a pointer to a thread instance with the index specified.
			// 
			// If `check_linked` is true, only threads with a positive link-status will be checked.
			// If `check_linked` is false, the first thread matching the provided index will be retrieved.
			const EntityThread* get_thread(EntityThreadIndex thread_index, bool check_linked=false) const;

			// Retrieves a pointer to a thread instance with the ID specified.
			// 
			// If `check_linked` is true, only threads with a positive link-status will be checked.
			// If `check_linked` is false, the first thread matching the provided ID will be retrieved.
			const EntityThread* get_thread(const EntityDescriptor& descriptor, EntityThreadID thread_id, bool check_linked=false) const;

			template <typename... ThreadLookup>
			EntityThread* get_thread(ThreadLookup&&... thread_info)
			{
				return const_cast<EntityThread*>
				(
					const_cast<const EntityThreadComponent*>(this)->get_thread
					(
						std::forward<ThreadLookup>(thread_info)...
					)
				);
			}

			template <typename Callback>
			std::size_t enumerate_threads(const EntityThreadRange& thread_range, Callback&& callback) const
			{
				return enumerate_threads_impl(*this, thread_range, std::forward<Callback>(callback));
			}

			template <typename Callback>
			std::size_t enumerate_threads(const EntityThreadRange& thread_range, Callback&& callback)
			{
				return enumerate_threads_impl(*this, thread_range, std::forward<Callback>(callback));
			}

			template <typename Callback>
			static std::size_t iterate_thread_range(const EntityThreadRange& thread_range, Callback&& callback)
			{
				std::size_t count = 0;

				for (auto thread_index = thread_range.begin_point(); thread_index < thread_range.end_point(); thread_index++)
				{
					count++;

					if constexpr (std::is_invocable_r_v<bool, Callback, decltype(thread_index)>)
					{
						if (!callback(thread_index))
						{
							break;
						}
					}
					else
					{
						callback(thread_index);
					}
				}

				return count;
			}

			std::optional<LocalThreadIndex> get_local_index(const EntityThread& thread) const;

			EntityThreadContainer threads;

			// Optional pointer to a container of global variables shared between each thread of this entity.
			std::shared_ptr<ThreadGlobalVariables> global_variables;
		protected:
			// NOTE: This method erases a thread without formally stopping it first, use at your own risk.
			EntityThreadIterator erase_thread(EntityThreadIterator thread_it);

			// Retrieves an iterator for a given thread.
			EntityThreadIterator get_thread_iterator(const EntityThread& thread);

			void terminate_all();

			template <typename SelfType, typename Callback>
			static std::size_t enumerate_threads_impl(SelfType& self, const EntityThreadRange& thread_range, Callback&& callback, bool check_linked=true)
			{
				std::size_t resolved_count = 0;

				iterate_thread_range
				(
					thread_range,

					[&self, &callback, check_linked, &resolved_count](EntityThreadIndex thread_index)
					{
						if (auto thread = self.get_thread(thread_index, check_linked))
						{
							resolved_count++;

							if constexpr((std::is_invocable_r_v<bool, Callback, decltype(thread_index), decltype(*thread)>))
							{
								if (!callback(thread_index, *thread))
								{
									return false;
								}
							}
							else if constexpr ((std::is_invocable_v<Callback, decltype(thread_index), decltype(*thread)>))
							{
								callback(thread_index, *thread);
							}
							else if constexpr ((std::is_invocable_r_v<bool, Callback, decltype(*thread)>))
							{
								if (!callback(*thread))
								{
									return false;
								}
							}
							else if constexpr ((std::is_invocable_v<Callback, decltype(*thread)>))
							{
								callback(*thread);
							}
						}

						return true;
					}
				);

				return resolved_count;
			}

			std::optional<EntityThreadIterator> stop_thread_impl(EntityThread& thread);
			std::optional<EntityThreadIterator> stop_thread_impl(EntityThreadIterator thread_it);
	};
}