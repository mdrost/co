#pragma once

#include <co/win32/executor_priority_0.h>

#include <co/executor_priority_0.hpp>

#include <cstdint>

namespace co
{

/// @defgroup cpp_win32_executor_priority_0 Win32 executor priority
/// @brief Executor priorities matching the levels of TP_CALLBACK_PRIORITY.
/// @{

/// @brief See CO_WIN32_EXECUTOR_PRIORITY_0_LOW.
inline constexpr std::intptr_t win32_executor_priority_0_low = CO_WIN32_EXECUTOR_PRIORITY_0_LOW;
/// @brief See CO_WIN32_EXECUTOR_PRIORITY_0_NORMAL.
inline constexpr std::intptr_t win32_executor_priority_0_normal = CO_WIN32_EXECUTOR_PRIORITY_0_NORMAL;
/// @brief See CO_WIN32_EXECUTOR_PRIORITY_0_HIGH.
inline constexpr std::intptr_t win32_executor_priority_0_high = CO_WIN32_EXECUTOR_PRIORITY_0_HIGH;

/// @}

} // namespace co
