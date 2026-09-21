#pragma once

#include <co/timer_0.hpp>

#include <atomic>
#include <cassert>
#include <concepts>
#include <coroutine>
#include <cstdint>
#include <exception>
#include <mutex>
#include <utility>

namespace co
{

/// @brief Promise of a coroutine bound to an executor, such as the promise of task_0.
template <class Promise>
concept executor_bound_promise_0 = requires(const Promise& promise) {
	{ promise.get_executor() } -> std::convertible_to<executor_0>;
};

/// @defgroup cpp_awaitable_timer_0 Awaitable timer
/// @brief Coroutine-friendly wrapper of a timer_0 that can wait until the timer is idle.
///
/// awaitable_timer_0 always installs its own completion callback when it starts the timer, so it knows whether a start
/// is pending. This lets a coroutine cancel the timer and wait until the cancellation took effect, without the code that
/// started the timer having to know in advance whether it will be cancelled:
///
/// - start_0() starts the timer with a plain completion callback (fire and forget).
/// - async_start_0() starts the timer and suspends the awaiting coroutine until the timer elapsed or was cancelled.
///   Without an executor argument, the timer is started with the executor of the awaiting coroutine (see
///   executor_bound_promise_0), so a task_0 resumes on its own executor.
/// - `co_await timer` suspends the awaiting coroutine until the pending start, if any, has completed, and returns the
///   status of that start. It does not suspend when the timer is idle.
/// - async_cancel_0() requests cancellation of the pending start, if any, and then waits like `co_await timer`. It is
///   not terminal: afterwards the timer may be started again or destroyed.
///
/// Any number of coroutines may wait with `co_await timer` or async_cancel_0() at the same time. They are resumed
/// after the completion callback of the start has returned. A coroutine bound to an executor is resumed on that
/// executor; any other coroutine is resumed from within the completion, or continues without suspending when the start
/// completes while it is starting to wait.
///
/// Thread safety: `co_await timer`, async_cancel_0(), cancel_0() and pending() may be used from any threads, concurrently
/// with each other and with the completion of the pending start. A start (start_0() or async_start_0()) requires the
/// timer to be idle and must not be concurrent with another start; starting again from the completion callback of the
/// previous start, or from a coroutine resumed by it, is allowed. Ordering is the responsibility of the caller:
/// - A `co_await timer` or async_cancel_0() waits only for a start made before it; one that runs concurrently with a
///   start may find the timer idle, or request cancellation before the wrapped timer is started and so not cancel it.
/// - An async_cancel_0() that runs concurrently with the completion of the start it waits for may request cancellation
///   of the next start, if that completion starts the timer again.
///
/// The wrapper does not own the timer. It must not be moved. It may only be destroyed when no start is pending, every
/// coroutine awaiting it has been resumed or has continued, and no other call on it is in progress.
/// @{

/// @brief Wrapper of a timer_0 whose starts and cancellations can be awaited. See @ref cpp_awaitable_timer_0.
class awaitable_timer_0 final
{
public:

	/// @brief Completion callback of start_0().
	using start_0_completion_fn = timer_0::start_0_completion_fn;

	class start_0_awaiter;
	class idle_awaiter;

	explicit awaitable_timer_0(timer_0 timer) noexcept
		: m_timer(timer)
	{
	}

	awaitable_timer_0(const awaitable_timer_0&) = delete;
	awaitable_timer_0& operator=(const awaitable_timer_0&) = delete;

	~awaitable_timer_0()
	{
		assert(!m_pending && "an awaitable timer may not be destroyed while a start is pending");
		assert(!m_first_waiter && "an awaitable timer may not be destroyed while coroutines wait for it");
	}

	/// @brief The wrapped timer.
	[[nodiscard]] timer_0 get_timer() const noexcept
	{
		return m_timer;
	}

	/// @brief Whether a start is pending: its completion has not been invoked yet.
	///
	/// Under concurrency, the value may be outdated as soon as it is returned.
	[[nodiscard]] bool pending() const noexcept
	{
		std::lock_guard lock(m_mutex);
		return m_pending;
	}

	/// @brief Starts the timer. See timer_0::start_0().
	/// @pre The timer is idle and no other start is in progress.
	result_0 start_0(uint64_t due_ns, executor_0 executor, start_0_completion_fn completion_fn, void* data) noexcept
	{
		{
			std::lock_guard lock(m_mutex);
			assert(!m_pending && "the timer may only be started when idle");
			m_completion_fn = completion_fn;
			m_completion_data = data;
			m_pending = true;
		}
		// On success, the completion may run, and the wrapper be destroyed, before the call returns: this is not touched.
		result_0 result = m_timer.start_0(due_ns, executor, &awaitable_timer_0::complete, this);
		if (result != result_0::success) {
			waiter* first_waiter;
			{
				std::lock_guard lock(m_mutex);
				m_pending = false;
				m_completion_fn = nullptr;
				m_completion_data = nullptr;
				first_waiter = take_waiters();
			}
			// Coroutines that started waiting concurrently with the failed start find the timer idle.
			resume_waiters(first_waiter, result_0::success);
		}
		return result;
	}

	/// @brief Requests cancellation of the pending start without waiting. See timer_0::cancel_0().
	result_0 cancel_0() const noexcept
	{
		return m_timer.cancel_0();
	}

	/// @brief Starts the timer and suspends until it elapsed or was cancelled.
	///
	/// The awaiting coroutine is resumed in the completion callback, in the execution context of executor. The result of
	/// `co_await` is `result_0::success` when the timer elapsed, `result_0::cancelled` when it was cancelled, or the error
	/// of timer_0::start_0(); in the latter case the coroutine is not suspended.
	/// @pre The timer is idle and no other start is in progress.
	[[nodiscard]] start_0_awaiter async_start_0(uint64_t due_ns, executor_0 executor) noexcept;

	/// @brief Starts the timer with the executor of the awaiting coroutine and suspends until it elapsed or was
	/// cancelled. See async_start_0(uint64_t, executor_0).
	/// @pre The awaiting coroutine has an executor_bound_promise_0 with a non-null executor.
	/// @pre The timer is idle and no other start is in progress.
	[[nodiscard]] start_0_awaiter async_start_0(uint64_t due_ns) noexcept;

	/// @brief Requests cancellation of the pending start, if any, and suspends until the timer is idle.
	///
	/// Same as `co_await timer` after requesting cancellation with timer_0::cancel_0(). The result of `co_await` is
	/// `result_0::success` once the timer is idle, or the error of timer_0::cancel_0(); in the latter case the coroutine
	/// is not suspended and the start stays pending.
	[[nodiscard]] idle_awaiter async_cancel_0() noexcept;

	/// @brief Suspends until the pending start, if any, has completed: `co_await timer`.
	///
	/// Does not suspend when no start is pending. Otherwise the awaiting coroutine is resumed after the completion
	/// callback of the pending start has returned: posted to its own executor if it has an executor_bound_promise_0,
	/// otherwise directly from within the completion, in the execution context of the executor of that start. When the
	/// start completes while the coroutine is still starting to wait, the coroutine continues without suspending. The
	/// result of `co_await` is the status of that start, or `result_0::success` when no start was pending. If the
	/// executor of the awaiting coroutine rejects its resumption, the process is terminated.
	///
	/// Only the coroutines waiting when the start completes are resumed; if the completion callback starts the timer
	/// again, coroutines that start waiting afterwards wait for the new start.
	[[nodiscard]] idle_awaiter operator co_await() noexcept;

private:

	// Coroutine waiting for the completion of the pending start. Lives in the awaiter, in the coroutine frame.
	//
	// The completion may take the waiter while await_suspend() is still running on another thread. The state decides who
	// continues the coroutine: the completion resumes it only if await_suspend() has finished (suspended); otherwise
	// await_suspend() sees completed and does not suspend.
	struct waiter
	{
		enum class state_type : unsigned char
		{
			suspending,
			suspended,
			completed,
		};

		waiter* next = nullptr;
		std::coroutine_handle<> coroutine;
		executor_0 executor;
		result_0 result = result_0::success;
		std::atomic<state_type> state = state_type::suspending;
	};

	// Requires m_mutex.
	void add_waiter(waiter& added) noexcept
	{
		added.next = nullptr;
		if (m_last_waiter) {
			m_last_waiter->next = &added;
		}
		else {
			m_first_waiter = &added;
		}
		m_last_waiter = &added;
	}

	// Requires m_mutex. Returns false when the waiter is not listed, because a completion has taken it.
	bool remove_waiter(waiter& removed) noexcept
	{
		waiter* previous = nullptr;
		waiter** link = &m_first_waiter;
		while (*link != &removed) {
			if (!*link) {
				return false;
			}
			previous = *link;
			link = &previous->next;
		}
		*link = removed.next;
		if (m_last_waiter == &removed) {
			m_last_waiter = previous;
		}
		return true;
	}

	// Requires m_mutex.
	waiter* take_waiters() noexcept
	{
		m_last_waiter = nullptr;
		return std::exchange(m_first_waiter, nullptr);
	}

	static void resume_waiters(waiter* first_waiter, result_0 status) noexcept
	{
		while (first_waiter) {
			waiter* current = first_waiter;
			// Read everything before publishing the status: afterwards the coroutine may run and destroy the waiter.
			first_waiter = current->next;
			std::coroutine_handle<> coroutine = current->coroutine;
			executor_0 executor = current->executor;
			current->result = status;
			if (current->state.exchange(waiter::state_type::completed, std::memory_order_acq_rel) == waiter::state_type::suspended) {
				resume(coroutine, executor);
			}
		}
	}

	static void complete(void* data, result_0 status) noexcept
	{
		awaitable_timer_0* self = static_cast<awaitable_timer_0*>(data);
		start_0_completion_fn completion_fn;
		void* completion_data;
		waiter* first_waiter;
		{
			std::lock_guard lock(self->m_mutex);
			completion_fn = std::exchange(self->m_completion_fn, nullptr);
			completion_data = std::exchange(self->m_completion_data, nullptr);
			// Take the waiters of this start: coroutines that wait afterwards wait for the next start, if any.
			first_waiter = self->take_waiters();
			self->m_pending = false;
		}
		// The wrapper is idle: it may now be started again or destroyed, on any thread, so self is not touched anymore.
		completion_fn(completion_data, status);
		resume_waiters(first_waiter, status);
	}

	// Resumes the coroutine on executor, or directly when executor is null.
	static void resume(std::coroutine_handle<> coroutine, executor_0 executor) noexcept
	{
		if (!executor) {
			coroutine.resume();
			return;
		}
		result_0 result = executor.post_0(executor_0::handler_0{
			.invoke_fn = [](void* address) noexcept { std::coroutine_handle<>::from_address(address).resume(); },
			.data = coroutine.address(),
		});
		if (result != result_0::success) {
			std::terminate();
		}
	}

private:
	timer_0 m_timer;
	mutable std::mutex m_mutex;
	bool m_pending = false;
	start_0_completion_fn m_completion_fn = nullptr;
	void* m_completion_data = nullptr;
	waiter* m_first_waiter = nullptr;
	waiter* m_last_waiter = nullptr;
};

/// @brief Awaiter returned by awaitable_timer_0::async_start_0().
class awaitable_timer_0::start_0_awaiter final
{
public:

	start_0_awaiter(awaitable_timer_0& timer, uint64_t due_ns, executor_0 executor) noexcept
		: m_timer(timer)
		, m_due_ns(due_ns)
		, m_executor(executor)
	{
	}

	[[nodiscard]] constexpr bool await_ready() const noexcept
	{
		return false;
	}

	bool await_suspend(std::coroutine_handle<> continuation) noexcept
	{
		assert(m_executor && "async_start_0() without an executor may only be awaited by an executor-bound coroutine");
		return start(continuation);
	}

	template <executor_bound_promise_0 Promise>
	bool await_suspend(std::coroutine_handle<Promise> continuation) noexcept
	{
		if (!m_executor) {
			m_executor = continuation.promise().get_executor();
			assert(m_executor && "the awaiting coroutine is not bound to an executor");
		}
		return start(continuation);
	}

	[[nodiscard]] result_0 await_resume() const noexcept
	{
		return m_result;
	}

private:

	bool start(std::coroutine_handle<> continuation) noexcept
	{
		m_continuation = continuation;
		result_0 result = m_timer.start_0(m_due_ns, m_executor, &start_0_awaiter::complete, this);
		if (result != result_0::success) {
			m_result = result;
			return false;
		}
		// The completion may already be resuming the coroutine on another thread: do not touch this anymore.
		return true;
	}

	static void complete(void* data, result_0 status) noexcept
	{
		start_0_awaiter* self = static_cast<start_0_awaiter*>(data);
		self->m_result = status;
		self->m_continuation.resume();
	}

private:
	awaitable_timer_0& m_timer;
	uint64_t m_due_ns;
	executor_0 m_executor;
	std::coroutine_handle<> m_continuation;
	result_0 m_result = result_0::success;
};

/// @brief Awaiter of `co_await timer` and awaitable_timer_0::async_cancel_0().
class awaitable_timer_0::idle_awaiter final
{
public:

	idle_awaiter(awaitable_timer_0& timer, bool cancel) noexcept
		: m_timer(timer)
		, m_cancel(cancel)
	{
	}

	idle_awaiter(const idle_awaiter&) = delete;
	idle_awaiter& operator=(const idle_awaiter&) = delete;

	// Whether the timer is idle is checked in await_suspend(), together with registering the waiter.
	[[nodiscard]] constexpr bool await_ready() const noexcept
	{
		return false;
	}

	bool await_suspend(std::coroutine_handle<> continuation) noexcept
	{
		return wait(continuation, nullptr);
	}

	template <executor_bound_promise_0 Promise>
	bool await_suspend(std::coroutine_handle<Promise> continuation) noexcept
	{
		return wait(continuation, continuation.promise().get_executor());
	}

	[[nodiscard]] result_0 await_resume() const noexcept
	{
		return m_cancel ? m_cancel_result : m_waiter.result;
	}

private:

	bool wait(std::coroutine_handle<> continuation, executor_0 executor) noexcept
	{
		m_waiter.coroutine = continuation;
		m_waiter.executor = executor;
		{
			std::lock_guard lock(m_timer.m_mutex);
			if (!m_timer.m_pending) {
				return false;
			}
			// Register before cancelling, so that the completion, which cancel_0() may post, finds the waiter.
			m_timer.add_waiter(m_waiter);
		}
		// The completion may take the waiter from now on, but leaves the coroutine to us until it is suspended. Not
		// being resumed, the coroutine keeps the wrapper alive (see the destruction precondition).
		if (m_cancel) {
			if (result_0 result = m_timer.m_timer.cancel_0(); result != result_0::success) {
				std::lock_guard lock(m_timer.m_mutex);
				if (m_timer.remove_waiter(m_waiter)) {
					m_cancel_result = result;
					return false;
				}
				// The start has completed meanwhile: the timer is idle, finish waiting as usual.
			}
		}
		// Suspend unless the completion has already taken the waiter. Once suspended, the completion may resume the
		// coroutine on another thread at any time, so this is not touched anymore.
		return m_waiter.state.exchange(waiter::state_type::suspended, std::memory_order_acq_rel) == waiter::state_type::suspending;
	}

private:
	awaitable_timer_0& m_timer;
	bool m_cancel;
	result_0 m_cancel_result = result_0::success;
	waiter m_waiter;
};

inline awaitable_timer_0::start_0_awaiter awaitable_timer_0::async_start_0(uint64_t due_ns, executor_0 executor) noexcept
{
	return start_0_awaiter(*this, due_ns, executor);
}

inline awaitable_timer_0::start_0_awaiter awaitable_timer_0::async_start_0(uint64_t due_ns) noexcept
{
	return start_0_awaiter(*this, due_ns, nullptr);
}

inline awaitable_timer_0::idle_awaiter awaitable_timer_0::async_cancel_0() noexcept
{
	return idle_awaiter(*this, true);
}

inline awaitable_timer_0::idle_awaiter awaitable_timer_0::operator co_await() noexcept
{
	return idle_awaiter(*this, false);
}

/// @}

} // namespace co
