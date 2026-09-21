#pragma once

#include "win32/executor_priority_0.h"

#include <cstdint>

namespace co::win32
{

// A Win32 thread pool class maps a co_executor_priority_info_0 onto the three levels of TP_CALLBACK_PRIORITY with
// three bands of equal width over [CO_EXECUTOR_PRIORITY_0_LOWEST, CO_EXECUTOR_PRIORITY_0_HIGHEST]. Excluding
// INTPTR_MIN leaves 2 * INTPTR_MAX + 1 priorities, a multiple of 3, so the bands are exactly equal; the middle one is
// centered on CO_EXECUTOR_PRIORITY_0_NORMAL, and the public CO_WIN32_EXECUTOR_PRIORITY_0_* constants are the
// centers of the bands.

// Greatest priority in the band of TP_CALLBACK_PRIORITY_LOW.
inline constexpr std::intptr_t tp_executor_priority_0_low_max = -(INTPTR_MAX / 3) - 1;
// Least priority in the band of TP_CALLBACK_PRIORITY_HIGH.
inline constexpr std::intptr_t tp_executor_priority_0_high_min = (INTPTR_MAX / 3) + 1;

static_assert(tp_executor_priority_0_low_max - CO_EXECUTOR_PRIORITY_0_LOWEST + 1 == tp_executor_priority_0_high_min - tp_executor_priority_0_low_max - 1);
static_assert(CO_EXECUTOR_PRIORITY_0_HIGHEST - tp_executor_priority_0_high_min + 1 == tp_executor_priority_0_high_min - tp_executor_priority_0_low_max - 1);
static_assert(tp_executor_priority_0_low_max == -tp_executor_priority_0_high_min);
static_assert(CO_WIN32_EXECUTOR_PRIORITY_0_LOW - CO_EXECUTOR_PRIORITY_0_LOWEST == tp_executor_priority_0_low_max - CO_WIN32_EXECUTOR_PRIORITY_0_LOW);
static_assert(CO_WIN32_EXECUTOR_PRIORITY_0_HIGH - tp_executor_priority_0_high_min == CO_EXECUTOR_PRIORITY_0_HIGHEST - CO_WIN32_EXECUTOR_PRIORITY_0_HIGH);
static_assert(CO_EXECUTOR_PRIORITY_0_LOW <= tp_executor_priority_0_low_max);
static_assert(CO_EXECUTOR_PRIORITY_0_HIGH >= tp_executor_priority_0_high_min);

// Level of TP_CALLBACK_PRIORITY a priority maps to.
// @pre priority is not INTPTR_MIN.
constexpr TP_CALLBACK_PRIORITY to_tp_callback_priority(std::intptr_t priority) noexcept
{
	if (priority <= tp_executor_priority_0_low_max) {
		return TP_CALLBACK_PRIORITY_LOW;
	}
	if (priority >= tp_executor_priority_0_high_min) {
		return TP_CALLBACK_PRIORITY_HIGH;
	}
	return TP_CALLBACK_PRIORITY_NORMAL;
}

static_assert(to_tp_callback_priority(CO_EXECUTOR_PRIORITY_0_LOWEST) == TP_CALLBACK_PRIORITY_LOW);
static_assert(to_tp_callback_priority(CO_WIN32_EXECUTOR_PRIORITY_0_LOW) == TP_CALLBACK_PRIORITY_LOW);
static_assert(to_tp_callback_priority(tp_executor_priority_0_low_max + 1) == TP_CALLBACK_PRIORITY_NORMAL);
static_assert(to_tp_callback_priority(CO_WIN32_EXECUTOR_PRIORITY_0_NORMAL) == TP_CALLBACK_PRIORITY_NORMAL);
static_assert(to_tp_callback_priority(tp_executor_priority_0_high_min - 1) == TP_CALLBACK_PRIORITY_NORMAL);
static_assert(to_tp_callback_priority(CO_WIN32_EXECUTOR_PRIORITY_0_HIGH) == TP_CALLBACK_PRIORITY_HIGH);
static_assert(to_tp_callback_priority(CO_EXECUTOR_PRIORITY_0_HIGHEST) == TP_CALLBACK_PRIORITY_HIGH);

} // namespace co::win32
