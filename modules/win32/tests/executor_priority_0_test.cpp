#include <co/win32/executor_priority_0.hpp>

#include <cstdint>

// The constants are the centers of three bands of equal width; the middle one is centered on normal.
static_assert(CO_WIN32_EXECUTOR_PRIORITY_0_LOW == -CO_WIN32_EXECUTOR_PRIORITY_0_HIGH);
static_assert(static_cast<std::uintptr_t>(CO_WIN32_EXECUTOR_PRIORITY_0_HIGH) == (static_cast<std::uintptr_t>(INTPTR_MAX) * 2 + 1) / 3);
static_assert(CO_WIN32_EXECUTOR_PRIORITY_0_NORMAL == CO_EXECUTOR_PRIORITY_0_NORMAL);
static_assert(co::win32_executor_priority_0_low == CO_WIN32_EXECUTOR_PRIORITY_0_LOW);
static_assert(co::win32_executor_priority_0_normal == CO_WIN32_EXECUTOR_PRIORITY_0_NORMAL);
static_assert(co::win32_executor_priority_0_high == CO_WIN32_EXECUTOR_PRIORITY_0_HIGH);
