#pragma once

#include <co.h>
#include <co/clock_0.h>
#include <co/win32/api.h>

#ifdef __cplusplus
extern "C"
{
#endif

/// @defgroup c_win32_clock_0 Win32 clock
/// @brief Clock showing the system time (UTC) of Windows.
///
/// A Win32 clock is a clock (see @ref c_clock_0) created with co_win32_clock_0_create_0(); its handle may be cast to
/// co_clock_0 with CO_CLOCK_0_CAST.
///
/// - The time is the system time, from GetSystemTimePreciseAsFileTime(), in nanoseconds since the creation of the
///   clock. It jumps when the system time is set. It saturates about 292 years before or after the origin, which it
///   reaches only if the system time is set so.
/// - The calendar origin is the system time at the creation of the clock, with 100 ns resolution.
/// - The base time is the interrupt time, from QueryInterruptTimePrecise(), which advances while the system is
///   suspended, or the unbiased interrupt time, from QueryUnbiasedInterruptTimePrecise(), which does not; see
///   co_win32_clock_0_create_info_0::include_suspend_time.
/// - The mapping runs at rate 1 from a reading of both times.
/// - Changes are not detected yet: the change generation stays 0, co_clock_0_wait_change_0() returns
///   #CO_RESULT_0_ERROR_NOT_SUPPORTED, and co_clock_0_cancel_0() has no wait to cancel.
///
/// Requires Windows 10 or later.
/// @{

/// @brief Structure type of co_win32_clock_0_create_info_0.
extern CO_WIN32_API
const co_structure_type_0 CO_STRUCTURE_TYPE_0_WIN32_CLOCK_0_CREATE_INFO_0;

typedef struct co_win32_clock_0_t co_win32_clock_0_t; // private; do not use
/// @brief Handle of a Win32 clock; may be cast to co_clock_0 with CO_CLOCK_0_CAST.
typedef struct co_win32_clock_0_t* co_win32_clock_0;

/// @brief Casts a handle to co_win32_clock_0, unchecked; `NULL` stays `NULL`. A downcast requires the object to be a
/// Win32 clock. See "Handles and casts" in Concepts.
#define CO_WIN32_CLOCK_0_CAST(handle) ((co_win32_clock_0)(handle))

/// @brief Parameters of a Win32 clock.
typedef struct co_win32_clock_0_create_info_0 CO_FINAL
{
	/// @brief Identifies the structure; set to #CO_STRUCTURE_TYPE_0_WIN32_CLOCK_0_CREATE_INFO_0.
	co_structure_type_0 structure_type;

	/// @brief Extension chain; may hold a co_allocation_callbacks_0, which allocate the clock. Without one, the clock is
	/// allocated with the global operator new of the library.
	const co_in_structure_0* next_structure;

	/// @brief Whether the base time advances while the system is suspended (sleep, hibernation).
	bool include_suspend_time;
} co_win32_clock_0_create_info_0;

/// @brief Creates a Win32 clock; *clock is written only on success.
/// @retval CO_RESULT_0_SUCCESS *clock received the clock.
/// @retval CO_RESULT_0_ERROR_NOT_SUPPORTED The next chain holds an unsupported entry, or the system lacks a needed
/// function (before Windows 10).
/// @retval CO_RESULT_0_ERROR_OUT_OF_MEMORY The clock cannot be allocated.
/// @pre create_info is not `NULL`; its next chain holds at most one co_allocation_callbacks_0, with allocate_fn and
/// deallocate_fn not `NULL`.
/// @pre clock is not `NULL`.
CO_WIN32_API
co_result_0 co_win32_clock_0_create_0(const co_win32_clock_0_create_info_0* create_info, co_win32_clock_0* clock) CO_NOEXCEPT;

/// @brief Destroys a Win32 clock; does nothing when clock is `NULL`. Same as co_clock_0_destroy() on the clock, which it
/// calls inline.
/// @pre No wait of the clock is pending, and no other call on it is in progress.
static inline void co_win32_clock_0_destroy(co_win32_clock_0 clock) CO_NOEXCEPT
{
	co_clock_0_destroy(CO_CLOCK_0_CAST(clock));
}

/// @}

#ifdef __cplusplus
}
#endif
