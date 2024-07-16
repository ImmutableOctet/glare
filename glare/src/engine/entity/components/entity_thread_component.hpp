#pragma once

#include <engine/entity/entity_thread.hpp>
#include <engine/entity/entity_thread_range.hpp>
#include <engine/entity/entity_variables.hpp>

#include <engine/script/script_handle.hpp>

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

			inline const EntityThreadContainer& get_threads() const
			{
				return threads;
			}

			inline EntityThreadContainer& get_threads()
			{
				return threads;
			}

			// Attempts to allocate a `ThreadGlobalVariables` object, managed internally.
			// If a `ThreadGlobalVariables` object has already been allocated, this will return a pointer to the existing instance.
			// The value returned is a non-owning pointer to the remote object. (see `global_variables`)
			ThreadGlobalVariables* get_global_variables(); // const

			EntityThread* start_thread
			(
				ScriptFiber&& fiber,
				
				std::optional<EntityStateIndex> state_index=std::nullopt,

				EntityThreadID thread_id={},
				EntityThreadID parent_thread_id={},

				const EntityThreadFlags& flags={},

				ScriptHandle script_handle={}
			);

			EntityThread* start_thread
			(
				EntityThreadIndex thread_index,

				std::optional<EntityStateIndex> state_index=std::nullopt,
				
				bool check_existing=true,
				bool check_linked=true,
				bool restart_existing=false,
				
				EntityThreadID thread_id={},
				EntityThreadID parent_thread_id={},
				
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
				
				EntityThreadID parent_thread_id,

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
				bool restart_existing=false,

				EntityThreadID parent_thread_id={}
			);

			std::size_t start_threads
			(
				const EntityDescriptor& descriptor,
				const EntityThreadRange& thread_range,
				std::optional<EntityStateIndex> state_index=std::nullopt,
				EntityThreadID parent_thread_id={},
				bool restart_existing=false
			);

			std::size_t start_threads
			(
				const EntityDescriptor& descriptor,
				EntityStateIndex state_index,
				EntityThreadID parent_thread_id={},
				bool restart_existing=false
			);

			std::size_t start_threads
			(
				const EntityDescriptor& descriptor,
				const EntityState& state,
				EntityStateIndex state_index,
				EntityThreadID parent_thread_id={},
				bool restart_existing=false
			);

			// The return value of this method indicates if the specified
			// thread was terminated (true) or detached (false).
			// 
			// If no thread could be found, true will be returned.
			bool stop_thread(EntityThreadIndex thread_index, bool check_linked=true);

			inline bool stop_thread(const EntityDescriptor& descriptor, EntityThreadID thread_id, bool check_linked=true)
			{
				return stop_thread_by_id(descriptor, thread_id, check_linked);
			}

			bool stop_thread_by_id(EntityThreadID thread_id, bool check_linked=true);
			bool stop_thread_by_id(const EntityDescriptor& descriptor, EntityThreadID thread_id, bool check_linked=true);

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
			bool erase_thread_by_id(const EntityDescriptor& descriptor, EntityThreadID thread_id);

			// This method erases a thread without formally stopping it first.
			bool erase_thread_by_id(EntityThreadID thread_id);

			inline bool erase_thread(const EntityDescriptor& descriptor, EntityThreadID thread_id)
			{
				return erase_thread_by_id(descriptor, thread_id);
			}

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

			// Pauses a thread originating from the `thread_index` specified.
			bool pause_thread(EntityThreadIndex thread_index, bool check_linked=true);

			// Pauses a thread with the `thread_id` specified.
			bool pause_thread_by_id(EntityThreadID thread_id, bool check_linked=true);

			// Pauses a thread with the `thread_id` specified.
			bool pause_thread_by_id(const EntityDescriptor& descriptor, EntityThreadID thread_id, bool check_linked=true);

			inline bool pause_thread(const EntityDescriptor& descriptor, EntityThreadID thread_id, bool check_linked=true)
			{
				return pause_thread_by_id(descriptor, thread_id, check_linked);
			}

			// Pauses threads originating from the `thread_range` specified.
			// The return value of this method indicates how many threads were paused.
			std::size_t pause_threads(const EntityThreadRange& thread_range, bool check_linked=true);

			// Resumes a thread originating from the `thread_index` specified.
			bool resume_thread(EntityThreadIndex thread_index, bool check_linked=true);

			// Resumes a thread with the `thread_id` specified.
			bool resume_thread_by_id(EntityThreadID thread_id, bool check_linked=true);

			// Resumes a thread with the `thread_id` specified.
			bool resume_thread_by_id(const EntityDescriptor& descriptor, EntityThreadID thread_id, bool check_linked=true);

			// Resumes a thread with the `thread_id` specified.
			inline bool resume_thread(const EntityDescriptor& descriptor, EntityThreadID thread_id, bool check_linked=true)
			{
				return resume_thread_by_id(descriptor, thread_id, check_linked);
			}

			// Resumes threads originating from the `thread_range` specified.
			// The return value of this method indicates how many threads have resumed execution.
			std::size_t resume_threads(const EntityThreadRange& thread_range, bool check_linked=true);

			// Attaches a thread originating from the `thread_index` specified.
			bool attach_thread
			(
				EntityThreadIndex thread_index,
				std::optional<EntityStateIndex> state_index=std::nullopt,
				bool check_linked=true
			);

			// Attaches a thread with the `thread_id` specified.
			bool attach_thread_by_id
			(
				EntityThreadID thread_id,
				std::optional<EntityStateIndex> state_index=std::nullopt,
				bool check_linked=true
			);

			// Attaches a thread with the `thread_id` specified.
			bool attach_thread_by_id
			(
				const EntityDescriptor& descriptor,
				EntityThreadID thread_id,
				std::optional<EntityStateIndex> state_index=std::nullopt,
				bool check_linked=true
			);

			// Attaches a thread with the `thread_id` specified.
			inline bool attach_thread
			(
				const EntityDescriptor& descriptor,
				EntityThreadID thread_id,
				std::optional<EntityStateIndex> state_index=std::nullopt,
				bool check_linked=true
			)
			{
				return attach_thread_by_id
				(
					descriptor,
					thread_id,
					state_index,
					check_linked
				);
			}

			// Attaches threads originating from the `thread_range` specified.
			// The return value of this method indicates how many threads have been reattached.
			std::size_t attach_threads
			(
				const EntityThreadRange& thread_range,
				std::optional<EntityStateIndex> state_index=std::nullopt,
				bool check_linked=true
			);

			// Detaches a thread originating from the `thread_index` specified.
			bool detach_thread(EntityThreadIndex thread_index, bool check_linked=true);

			// Detaches a thread with the `thread_id` specified.
			bool detach_thread_by_id(EntityThreadID thread_id, bool check_linked=true);

			// Detaches a thread with the `thread_id` specified.
			bool detach_thread_by_id(const EntityDescriptor& descriptor, EntityThreadID thread_id, bool check_linked=true);

			// Detaches a thread with the `thread_id` specified.
			inline bool detach_thread(const EntityDescriptor& descriptor, EntityThreadID thread_id, bool check_linked=true)
			{
				return detach_thread_by_id(descriptor, thread_id, check_linked);
			}

			// Detaches threads originating from the `thread_range` specified.
			// The return value of this method indicates how many threads have been detached.
			std::size_t detach_threads(const EntityThreadRange& thread_range, bool check_linked=true);

			// Unlinks a thread originating from the `thread_index` specified.
			bool unlink_thread(EntityThreadIndex thread_index);

			// Unlinks a thread with the `thread_id` specified.
			bool unlink_thread_by_id(EntityThreadID thread_id);

			// Unlinks a thread with the `thread_id` specified.
			bool unlink_thread_by_id(const EntityDescriptor& descriptor, EntityThreadID thread_id);

			// Unlinks a thread with the `thread_id` specified.
			inline bool unlink_thread(const EntityDescriptor& descriptor, EntityThreadID thread_id)
			{
				return unlink_thread_by_id(descriptor, thread_id);
			}

			// Unlinks threads originating from the `thread_range` specified.
			// The return value of this method indicates how many threads have been unlinked.
			std::size_t unlink_threads(const EntityThreadRange& thread_range);

			// Attempts to re-link a thread by its local index.
			// 
			// If a thread with the same source index (at a different local index)
			// is already linked, this action will fail (return false).
			bool link_local_thread(LocalThreadIndex local_thread_index);

			// Skips forward `instruction_count` instructions on a thread originating from the `thread_index` specified.
			bool skip_thread(EntityThreadIndex thread_index, EntityInstructionCount instruction_count=1, bool check_linked=true);

			// Skips forward `instruction_count` instructions on a thread with the `thread_id` specified.
			bool skip_thread_by_id(EntityThreadID thread_id, EntityInstructionCount instruction_count=1, bool check_linked=true);

			// Skips forward `instruction_count` instructions on a thread with the `thread_id` specified.
			bool skip_thread_by_id(const EntityDescriptor& descriptor, EntityThreadID thread_id, EntityInstructionCount instruction_count=1, bool check_linked=true);

			// Skips forward `instruction_count` instructions on a thread with the `thread_id` specified.
			inline bool skip_thread(const EntityDescriptor& descriptor, EntityThreadID thread_id, EntityInstructionCount instruction_count=1, bool check_linked=true)
			{
				return skip_thread_by_id(descriptor, thread_id, instruction_count, check_linked);
			}

			// Skips forward `instruction_count` instructions on threads originating from the `thread_range` specified.
			// The return value of this method indicates how many threads have skipped forward the desired amount.
			std::size_t skip_threads(const EntityThreadRange& thread_range, EntityInstructionCount instruction_count=1, bool check_linked=true);

			// Skips backward `instruction_count` instructions on a thread originating from the `thread_index` specified.
			bool rewind_thread(EntityThreadIndex thread_index, EntityInstructionCount instruction_count=1, bool check_linked=true);

			// Skips backward `instruction_count` instructions on a thread with the `thread_id` specified.
			bool rewind_thread_by_id(EntityThreadID thread_id, EntityInstructionCount instruction_count=1, bool check_linked=true);

			// Skips backward `instruction_count` instructions on a thread with the `thread_id` specified.
			bool rewind_thread_by_id(const EntityDescriptor& descriptor, EntityThreadID thread_id, EntityInstructionCount instruction_count=1, bool check_linked=true);

			// Skips backward `instruction_count` instructions on a thread with the `thread_id` specified.
			inline bool rewind_thread(const EntityDescriptor& descriptor, EntityThreadID thread_id, EntityInstructionCount instruction_count=1, bool check_linked=true)
			{
				return rewind_thread_by_id(descriptor, thread_id, instruction_count, check_linked);
			}

			// Skips backward `instruction_count` instructions on threads originating from the `thread_range` specified.
			// The return value of this method indicates how many threads have skipped backward the desired amount.
			std::size_t rewind_threads(const EntityThreadRange& thread_range, EntityInstructionCount instruction_count=1, bool check_linked=true);

			// Checks if a (linked) thread spawned from the index specified is running.
			// 
			// See also: `get_thread`
			bool is_thread_running(EntityThreadIndex thread_index) const;

			// Checks if a (linked) thread spawned with the ID specified is running.
			// 
			// See also: `is_thread_running`, `get_thread`
			bool is_thread_with_id_running(EntityThreadID thread_id) const;

			// Checks if a (linked) thread spawned with the ID specified is running.
			// 
			// See also: `is_thread_running`, `get_thread`
			bool is_thread_with_id_running(const EntityDescriptor& descriptor, EntityThreadID thread_id) const;

			// Checks if a (linked) thread spawned with the ID specified is running.
			// 
			// See also: `get_thread`
			inline bool is_thread_running(const EntityDescriptor& descriptor, EntityThreadID thread_id) const
			{
				return is_thread_with_id_running(descriptor, thread_id);
			}

			// Retrieves a pointer to a thread instance with the index specified.
			// 
			// If `check_linked` is true, only threads with a positive link-status will be checked.
			// If `check_linked` is false, the first thread matching the provided index will be retrieved.
			const EntityThread* get_thread(EntityThreadIndex thread_index, bool check_linked=false) const;

			// Retrieves a pointer to a thread instance with the ID specified.
			// 
			// If `check_linked` is true, only threads with a positive link-status will be checked.
			// If `check_linked` is false, the first thread matching the provided ID will be retrieved.
			const EntityThread* get_thread_by_id(EntityThreadID thread_id, bool check_linked=false) const;

			// Retrieves a pointer to a thread instance with the ID specified.
			// This function attempts to resolve thread indices using `descriptor` first, before checking internal records for `thread_id`.
			// 
			// If `check_linked` is true, only threads with a positive link-status will be checked.
			// If `check_linked` is false, the first thread matching the provided ID will be retrieved.
			const EntityThread* get_thread_by_id(const EntityDescriptor& descriptor, EntityThreadID thread_id, bool check_linked=false) const;

			inline const EntityThread* get_thread(const EntityDescriptor& descriptor, EntityThreadID thread_id, bool check_linked=false) const
			{
				return get_thread_by_id(descriptor, thread_id, check_linked);
			}

			template <typename... ThreadLookup>
			EntityThread* get_thread(ThreadLookup&&... thread_info)
			{
				return const_cast<EntityThread*>
				(
					std::as_const(*this).get_thread
					(
						std::forward<ThreadLookup>(thread_info)...
					)
				);
			}

			template <typename... ThreadLookup>
			EntityThread* get_thread_by_id(ThreadLookup&&... thread_info)
			{
				return const_cast<EntityThread*>
				(
					std::as_const(*this).get_thread_by_id
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