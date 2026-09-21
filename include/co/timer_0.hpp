#pragma once

#include <co/timer_0.h>

#include <co.hpp>
#include <co/clock_0.hpp>

#include <cassert>
#include <coroutine>
#include <cstdint>

namespace co
{

/// @defgroup cpp_timer_0 Timer
/// @brief Timers with a settable expiry whose waits complete on an executor.
///
/// Timers are created by a timer service with timer_0_service_0::create_timer_0(); the timer service is found in an
/// event domain that provides service_type_0_timer_0_service_0.
///
/// - A timer is bound to the clock given at its creation (timer_0_create_info_0::clock): expiries at a point in time
///   are in the time of the clock, expiries after a duration in its base time.
/// - timer_0::expiry_at_0()
///   timer has no expiry. The timer has expired once its expiry has been reached, until the expiry is set again.
/// - timer_0::wait_0() waits until the timer has expired, either with a completion callback or, without one, as an
///   awaitable: `co_await timer.wait_0()`. Any number of waits may be pending; setting the expiry applies to them too.
/// - After a successful wait, its completion callback is posted to the given executor exactly once, with
///   `result_0::success` when the timer expired or `result_0::cancelled` when the wait was cancelled first, through its
///   cancellation or timer_0::cancel_0(). If the executor rejects a completion callback, the process is terminated.
/// - The timer may be destroyed with timer_0::destroy() when no wait is pending; in particular from within the
///   completion callback of its last wait.
/// - The methods other than timer_0::destroy() may be called from any thread, concurrently.
///
/// See @ref c_timer_0 for the full contract and the C API.
/// @{

/// @brief Completion callback of timer_0::wait_0(), invoked exactly once per successful wait.
/// @param data User data passed to timer_0::wait_0().
/// @param status `result_0::success` when the timer expired, or `result_0::cancelled` when the wait was cancelled
/// first.
using timer_0_wait_0_completion_fn = void (*)(void* data, result_0 status) noexcept;

/// @brief Structure type of timer_0_create_info_0.
extern CO_API
const structure_type_0 structure_type_0_timer_0_create_info_0;

/// @brief Create info of a timer, passed to timer_0_service_0::create_timer_0(). See co_timer_0_create_info_0.
struct timer_0_create_info_0 final
{
	structure_type_0 structure_type;
	const in_structure_0* next_structure;

	/// @brief Clock the timer is bound to. The timer does not own the clock.
	/// @pre Not null.
	/// @pre The clock outlives the timer.
	clock_0 clock;
};

/// @brief Handle of a timer; destroyed with destroy(). See @ref cpp_timer_0 for its contract.
class timer_0 final
{
public:

	/// @brief Completion callback of wait_0().
	using wait_0_completion_fn = timer_0_wait_0_completion_fn;

	class wait_0_awaiter;

	timer_0() noexcept = default;

	timer_0(std::nullptr_t) noexcept
	{
	}

	explicit timer_0(co_timer_0 handle) noexcept
		: m_handle(handle)
	{
	}

	explicit operator co_timer_0() const noexcept
	{
		return m_handle;
	}

	explicit operator bool() const noexcept
	{
		return m_handle != nullptr;
	}

	friend bool operator==(const timer_0&, const timer_0&) noexcept = default;

	/// @brief Destroys the timer; does nothing when the handle is null. See co_timer_0_destroy().
	/// @pre No wait of the timer is pending. Destroying it from within the completion callback of its last wait is
	/// allowed.
	/// @pre No other call on the timer is in progress.
	void destroy() const noexcept
	{
		co_timer_0_destroy(m_handle);
	}

	/// @brief Gets the clock the timer is bound to. See co_timer_0_get_clock_0().
	void get_clock_0(clock_0* clock) const noexcept
	{
		co_timer_0_get_clock_0(m_handle, reinterpret_cast<co_clock_0*>(clock));
	}

	/// @brief Sets the expiry to a point in time, as a time of the clock of the timer in signed nanoseconds. See
	/// co_timer_0_expiry_at_0().
	result_0 expiry_at_0(std::int64_t time_ns) const noexcept
	{
		return static_cast<result_0>(co_timer_0_expiry_at_0(m_handle, time_ns));
	}

	/// @brief Sets the expiry to a duration from now, in nanoseconds of the base time of the clock of the timer. See
	/// co_timer_0_expiry_after_0().
	result_0 expiry_after_0(std::uint64_t duration_ns) const noexcept
	{
		return static_cast<result_0>(co_timer_0_expiry_after_0(m_handle, duration_ns));
	}

	/// @brief Waits until the timer has expired. See co_timer_0_wait_0().
	/// @param cancellation Cancellation whose request cancels the wait, or null.
	/// @pre executor and completion_fn are not null.
	result_0 wait_0(cancellation_0 cancellation, executor_0 executor, wait_0_completion_fn completion_fn, void* data) const noexcept
	{
		return static_cast<result_0>(co_timer_0_wait_0(m_handle, static_cast<co_cancellation_0>(cancellation), static_cast<co_executor_0>(executor), reinterpret_cast<co_timer_0_wait_0_completion_fn>(completion_fn), data));
	}

	/// @brief Waits until the timer has expired, as an awaitable: `co_await timer.wait_0()`.
	///
	/// The awaiting coroutine is resumed in the execution context of executor; without one, of the executor of the
	/// awaiting coroutine (see executor_bound_promise_0), so a task_0 resumes on its own executor. The result of
	/// `co_await` is `result_0::success` when the timer expired, `result_0::cancelled` when the wait was cancelled, or the
	/// error of the wait; in the latter case the coroutine is not suspended.
	/// @param cancellation Cancellation whose request cancels the wait, or null.
	/// @param executor Executor on which the coroutine is resumed, or null for the executor of the awaiting coroutine.
	/// @pre executor is not null, or the awaiting coroutine has an executor_bound_promise_0 with a non-null executor.
	[[nodiscard]] wait_0_awaiter wait_0(cancellation_0 cancellation = nullptr, executor_0 executor = nullptr) const noexcept;

	/// @brief Cancels every pending wait of the timer. See co_timer_0_cancel_0().
	result_0 cancel_0() const noexcept
	{
		return static_cast<result_0>(co_timer_0_cancel_0(m_handle));
	}

	/// @brief Sets the expiry to a calendar time, converted on the clock of the timer. See
	/// co_timer_0_expiry_at_calendar_0().
	result_0 expiry_at_calendar_0(const calendar_time_0& calendar_time) const noexcept
	{
		return static_cast<result_0>(co_timer_0_expiry_at_calendar_0(m_handle, reinterpret_cast<const co_calendar_time_0*>(&calendar_time)));
	}

private:
	co_timer_0 m_handle = nullptr;
};

static_assert(detail::is_handle_layout_compatible<timer_0, co_timer_0>);

/// @brief Awaiter returned by timer_0::wait_0() without a completion callback.
class timer_0::wait_0_awaiter final
{
public:

	wait_0_awaiter(timer_0 timer, cancellation_0 cancellation, executor_0 executor) noexcept
		: m_timer(timer)
		, m_cancellation(cancellation)
		, m_executor(executor)
	{
	}

	wait_0_awaiter(const wait_0_awaiter&) = delete;
	wait_0_awaiter& operator=(const wait_0_awaiter&) = delete;

	[[nodiscard]] constexpr bool await_ready() const noexcept
	{
		return false;
	}

	bool await_suspend(std::coroutine_handle<> continuation) noexcept
	{
		assert(m_executor && "wait_0() without an executor may only be awaited by an executor-bound coroutine");
		return wait(continuation);
	}

	template <executor_bound_promise_0 Promise>
	bool await_suspend(std::coroutine_handle<Promise> continuation) noexcept
	{
		if (!m_executor) {
			m_executor = continuation.promise().get_executor();
			assert(m_executor && "the awaiting coroutine is not bound to an executor");
		}
		return wait(continuation);
	}

	[[nodiscard]] result_0 await_resume() const noexcept
	{
		return m_result;
	}

private:

	bool wait(std::coroutine_handle<> continuation) noexcept
	{
		m_continuation = continuation;
		result_0 result = m_timer.wait_0(m_cancellation, m_executor, &wait_0_awaiter::complete, this);
		if (result != result_0::success) {
			m_result = result;
			return false;
		}
		// The completion may already be resuming the coroutine on another thread: do not touch this anymore.
		return true;
	}

	static void complete(void* data, result_0 status) noexcept
	{
		wait_0_awaiter* self = static_cast<wait_0_awaiter*>(data);
		self->m_result = status;
		// Invoked in the execution context of the executor: resume the coroutine right here.
		self->m_continuation.resume();
	}

private:
	timer_0 m_timer;
	cancellation_0 m_cancellation;
	executor_0 m_executor;
	std::coroutine_handle<> m_continuation;
	result_0 m_result = result_0::success;
};

inline timer_0::wait_0_awaiter timer_0::wait_0(cancellation_0 cancellation, executor_0 executor) const noexcept
{
	return wait_0_awaiter(*this, cancellation, executor);
}

/// @}

} // namespace co
