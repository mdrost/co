#pragma once

#include <co/timer_0.h>

#include <co.hpp>

#include <cstdint>

namespace co
{

/// @defgroup cpp_timer_0 Timer
/// @brief One-shot timers whose completions are posted to an executor.
///
/// Timers are created by a timer service with timer_0_service_0::create_timer_0(); the timer service is found in an
/// event domain that provides service_type_0_timer_0_service_0. A timer is reusable: it runs one start at a time.
///
/// - timer_0::start_0() starts the timer. After a successful start, its completion callback is posted to the given
///   executor exactly once, with `result_0::success` when the timer elapsed or `result_0::cancelled` when it was
///   cancelled first. It is never invoked from within start_0().
/// - A start is pending from its successful start_0() until its completion callback is invoked. The timer is idle when
///   no start is pending. It may only be started when idle, for example again from within the completion callback of
///   its previous start.
/// - timer_0::cancel_0() requests cancellation of the pending start. It does nothing when no start is pending or the
///   timer has already elapsed; the outcome is reported only by the status passed to the completion callback.
/// - The timer may be destroyed with timer_0::destroy() when idle; in particular from within the completion callback of
///   its last start.
/// - If the executor rejects a completion callback, the process is terminated.
/// - Starting and cancelling may be done from any thread, but the caller must ensure that the preconditions above hold,
///   for example by calling them only from handlers of one strand.
///
/// To stop using a timer while a start may be pending, cancel the start and destroy the timer from its completion
/// callback, which is invoked (with `result_0::success` or `result_0::cancelled`) once the timer no longer uses it. The
/// owner must not start the timer again from that callback, for example by setting a flag that the callback checks.
///
/// See @ref c_timer_0 for the C API.
/// @{

/// @brief Completion callback of timer_0::start_0(), invoked exactly once per successful start.
/// @param data User data passed to timer_0::start_0().
/// @param status `result_0::success` when the timer elapsed, or `result_0::cancelled` when timer_0::cancel_0() took
/// effect first.
using timer_0_start_0_completion_fn = void (*)(void* data, result_0 status) noexcept;

/// @brief Handle of a timer; destroyed with destroy(). See @ref cpp_timer_0 for its contract.
class timer_0 final
{
public:

	/// @brief Completion callback of start_0().
	using start_0_completion_fn = timer_0_start_0_completion_fn;

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
	/// @pre The timer is idle: it was never started, or the completion callback of its last start has been invoked or is
	/// being invoked. Destroying it from within that callback is allowed.
	/// @pre No other call on the timer is in progress.
	void destroy() const noexcept
	{
		co_timer_0_destroy(m_handle);
	}

	/// @brief Starts the timer. See co_timer_0_start_0().
	///
	/// On success, completion_fn is posted to executor exactly once, after the timer elapsed or was cancelled. On failure,
	/// completion_fn is never invoked and the timer stays idle.
	///
	/// @param due_ns Delay in nanoseconds after which the timer elapses, rounded up to the resolution of the timer class.
	/// Zero makes the timer elapse as soon as possible.
	/// @param executor Executor to which completion_fn is posted. Must stay valid until completion_fn has been invoked.
	/// @param completion_fn Callback invoked when the timer elapsed or was cancelled.
	/// @param data User data passed to completion_fn.
	/// @retval result_0::success The start is pending.
	/// @retval result_0::error_out_of_memory The timer cannot allocate its bookkeeping.
	/// @pre The timer is idle. Starting it again from within the completion callback of its previous start is allowed.
	/// @pre executor and completion_fn are not null.
	result_0 start_0(uint64_t due_ns, executor_0 executor, start_0_completion_fn completion_fn, void* data) const noexcept
	{
		return static_cast<result_0>(co_timer_0_start_0(m_handle, due_ns, static_cast<co_executor_0>(executor), reinterpret_cast<co_timer_0_start_0_completion_fn>(completion_fn), data));
	}

	/// @brief Requests cancellation of the pending start. See co_timer_0_cancel_0().
	///
	/// If the request takes effect before the timer elapses, the completion callback of the start is invoked with
	/// `result_0::cancelled`. Does nothing and returns `result_0::success` when no start is pending or the timer has
	/// already elapsed, for example while its completion callback is queued on the executor.
	///
	/// @retval result_0::success The request was made, or there was nothing to cancel.
	/// @return An error when the request could not be made; the start then stays pending and the request may be retried.
	result_0 cancel_0() const noexcept
	{
		return static_cast<result_0>(co_timer_0_cancel_0(m_handle));
	}

private:
	co_timer_0 m_handle = nullptr;
};

static_assert(detail::is_handle_layout_compatible<timer_0, co_timer_0>);

/// @}

} // namespace co
