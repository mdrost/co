#pragma once

#include <co/suspend_watcher_0.h>

#include <co.hpp>

namespace co
{

/// @brief Completion callback of suspend_watcher_0::wait_suspend_0(), invoked exactly once per successful wait with
/// `result_0::success` or `result_0::cancelled` and the suspend generation. See
/// co_suspend_watcher_0_wait_suspend_0_completion_fn.
using suspend_watcher_0_wait_suspend_0_completion_fn = void (*)(void* data, result_0 status, std::size_t generation) noexcept;

/// @brief Completion callback of suspend_watcher_0::wait_resume_0(), invoked exactly once per successful wait with
/// `result_0::success` or `result_0::cancelled` and the resume generation. See
/// co_suspend_watcher_0_wait_resume_0_completion_fn.
using suspend_watcher_0_wait_resume_0_completion_fn = void (*)(void* data, result_0 status, std::size_t generation) noexcept;

/// @brief Handle of a suspend watcher; destroyed with destroy(). See suspend_watcher_0.h for its contract.
class suspend_watcher_0 final
{
public:

	/// @brief Completion callback of wait_suspend_0().
	using wait_suspend_0_completion_fn = suspend_watcher_0_wait_suspend_0_completion_fn;

	/// @brief Completion callback of wait_resume_0().
	using wait_resume_0_completion_fn = suspend_watcher_0_wait_resume_0_completion_fn;

	suspend_watcher_0() noexcept = default;

	suspend_watcher_0(std::nullptr_t) noexcept
	{
	}

	explicit suspend_watcher_0(co_suspend_watcher_0 handle) noexcept
		: m_handle(handle)
	{
	}

	explicit operator co_suspend_watcher_0() const noexcept
	{
		return m_handle;
	}

	explicit operator bool() const noexcept
	{
		return m_handle != nullptr;
	}

	friend bool operator==(const suspend_watcher_0&, const suspend_watcher_0&) noexcept = default;

	/// @brief Destroys the suspend watcher; does nothing when the handle is null. See co_suspend_watcher_0_destroy().
	/// @pre No wait of the suspend watcher is pending, and no other call on it is in progress.
	void destroy() const noexcept
	{
		co_suspend_watcher_0_destroy(m_handle);
	}

	/// @brief Gets the suspend generation. See co_suspend_watcher_0_get_suspend_generation_0().
	void get_suspend_generation_0(std::size_t* generation) const noexcept
	{
		co_suspend_watcher_0_get_suspend_generation_0(m_handle, generation);
	}

	/// @brief Gets the resume generation. See co_suspend_watcher_0_get_resume_generation_0().
	void get_resume_generation_0(std::size_t* generation) const noexcept
	{
		co_suspend_watcher_0_get_resume_generation_0(m_handle, generation);
	}

	/// @brief Waits for a suspend of the system after the given suspend generation. See
	/// co_suspend_watcher_0_wait_suspend_0().
	/// @param cancellation Cancellation whose request cancels the wait, or null.
	/// @pre after_generation is a suspend generation the suspend watcher had.
	/// @pre executor and completion_fn are not null.
	result_0 wait_suspend_0(std::size_t after_generation, cancellation_0 cancellation, executor_0 executor, wait_suspend_0_completion_fn completion_fn, void* data) const noexcept
	{
		return static_cast<result_0>(co_suspend_watcher_0_wait_suspend_0(m_handle, after_generation, static_cast<co_cancellation_0>(cancellation), static_cast<co_executor_0>(executor), reinterpret_cast<co_suspend_watcher_0_wait_suspend_0_completion_fn>(completion_fn), data));
	}

	/// @brief Waits for a resume of the system after the given resume generation. See
	/// co_suspend_watcher_0_wait_resume_0().
	/// @param cancellation Cancellation whose request cancels the wait, or null.
	/// @pre after_generation is a resume generation the suspend watcher had.
	/// @pre executor and completion_fn are not null.
	result_0 wait_resume_0(std::size_t after_generation, cancellation_0 cancellation, executor_0 executor, wait_resume_0_completion_fn completion_fn, void* data) const noexcept
	{
		return static_cast<result_0>(co_suspend_watcher_0_wait_resume_0(m_handle, after_generation, static_cast<co_cancellation_0>(cancellation), static_cast<co_executor_0>(executor), reinterpret_cast<co_suspend_watcher_0_wait_resume_0_completion_fn>(completion_fn), data));
	}

	/// @brief Cancels every pending wait of the suspend watcher. See co_suspend_watcher_0_cancel_0().
	result_0 cancel_0() const noexcept
	{
		return static_cast<result_0>(co_suspend_watcher_0_cancel_0(m_handle));
	}

private:
	co_suspend_watcher_0 m_handle = nullptr;
};

static_assert(detail::is_handle_layout_compatible<suspend_watcher_0, co_suspend_watcher_0>);

} // namespace co
