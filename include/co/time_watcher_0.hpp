#pragma once

#include <co/time_watcher_0.h>

#include <co.hpp>

#include <cstddef>

namespace co
{

/// @brief Completion callback of time_watcher_0::wait_change_0(), invoked exactly once per successful wait with
/// `result_0::success` or `result_0::cancelled` and the change generation. See
/// co_time_watcher_0_wait_change_0_completion_fn.
using time_watcher_0_wait_change_0_completion_fn = void (*)(void* data, result_0 status, std::size_t generation) noexcept;

/// @brief Handle of a time watcher; destroyed with destroy(). See @ref c_time_watcher_0 for its contract.
class time_watcher_0 final
{
public:

	/// @brief Completion callback of wait_change_0().
	using wait_change_0_completion_fn = time_watcher_0_wait_change_0_completion_fn;

	time_watcher_0() noexcept = default;

	time_watcher_0(std::nullptr_t) noexcept
	{
	}

	explicit time_watcher_0(co_time_watcher_0 handle) noexcept
		: m_handle(handle)
	{
	}

	explicit operator co_time_watcher_0() const noexcept
	{
		return m_handle;
	}

	explicit operator bool() const noexcept
	{
		return m_handle != nullptr;
	}

	friend bool operator==(const time_watcher_0&, const time_watcher_0&) noexcept = default;

	/// @brief Destroys the time watcher; does nothing when the handle is null. See co_time_watcher_0_destroy().
	/// @pre No wait of the time watcher is pending, and no other call on it is in progress.
	void destroy() const noexcept
	{
		co_time_watcher_0_destroy(m_handle);
	}

	/// @brief Gets the change generation. See co_time_watcher_0_get_generation_0().
	void get_generation_0(std::size_t* generation) const noexcept
	{
		co_time_watcher_0_get_generation_0(m_handle, generation);
	}

	/// @brief Waits for a change of the system time after the given change generation. See
	/// co_time_watcher_0_wait_change_0().
	/// @param cancellation Cancellation whose request cancels the wait, or null.
	/// @pre after_generation is a change generation the time watcher had.
	/// @pre executor and completion_fn are not null.
	result_0 wait_change_0(std::size_t after_generation, cancellation_0 cancellation, executor_0 executor, wait_change_0_completion_fn completion_fn, void* data) const noexcept
	{
		return static_cast<result_0>(co_time_watcher_0_wait_change_0(m_handle, after_generation, static_cast<co_cancellation_0>(cancellation), static_cast<co_executor_0>(executor), reinterpret_cast<co_time_watcher_0_wait_change_0_completion_fn>(completion_fn), data));
	}

	/// @brief Cancels every pending wait of the time watcher. See co_time_watcher_0_cancel_0().
	result_0 cancel_0() const noexcept
	{
		return static_cast<result_0>(co_time_watcher_0_cancel_0(m_handle));
	}

private:
	co_time_watcher_0 m_handle = nullptr;
};

static_assert(detail::is_handle_layout_compatible<time_watcher_0, co_time_watcher_0>);

} // namespace co
