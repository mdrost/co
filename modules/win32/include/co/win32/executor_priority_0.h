#pragma once

#include <co/executor_priority_0.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/// @defgroup c_win32_executor_priority_0 Win32 executor priority
/// @brief Executor priorities matching the levels of TP_CALLBACK_PRIORITY.
///
/// A Win32 thread pool class maps a co_executor_priority_info_0 onto the three levels of TP_CALLBACK_PRIORITY with three
/// bands of equal width, the middle one centered on CO_EXECUTOR_PRIORITY_0_NORMAL. These constants are the centers of
/// the bands.
/// @{

/// @brief Executor priority mapping to TP_CALLBACK_PRIORITY_LOW.
#define CO_WIN32_EXECUTOR_PRIORITY_0_LOW (-(INTPTR_MAX / 3 * 2) - 1)
/// @brief Executor priority mapping to TP_CALLBACK_PRIORITY_NORMAL.
#define CO_WIN32_EXECUTOR_PRIORITY_0_NORMAL ((intptr_t)0)
/// @brief Executor priority mapping to TP_CALLBACK_PRIORITY_HIGH.
#define CO_WIN32_EXECUTOR_PRIORITY_0_HIGH ((INTPTR_MAX / 3 * 2) + 1)

/// @}

#ifdef __cplusplus
}
#endif
